#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <asm-generic/uaccess.h>
#include "ring_buffer.h"

MODULE_AUTHOR ("Frank Drews");
MODULE_DESCRIPTION ("Virtual Event Recorder");
MODULE_LICENSE ("GPL");

#define INPUT_DEVICE_NAME_MOUSE "vv_mouse_dev"
#define INPUT_DEVICE_NAME_MOUSE_REL "vv_mouse_dev_rel"
#define INPUT_DEVICE_NAME_KB "vr_kb_dev"
#define INPUT_CHAR_DEVICE_NAME "vrecorder"
#define INPUT_FILTER_HANDLER_NAME "vr_filter"

#define ABS_MAX_X 32768u
#define ABS_MAX_Y 32768u
//#define ABS_MAX_Y 65535u

unsigned int vr_chrdev_major;
static struct class* vr_chrdev_class = NULL;
static struct device* vr_chrdev_device = NULL;

#define CLASS_NAME  "virtual"

#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

static struct input_dev *vr_mouse_dev;
static struct input_dev *vr_mouse_dev_rel;
static struct input_dev *vr_kb_dev;

/* you will likely need a waitqueue to put a caller
to sleep in 'read()' */

static DECLARE_WAIT_QUEUE_HEAD(vr_waitqueue);

//holds identiy of the troubled data structure
extern unsigned int myevent_id;

/* you will likely need a mutex to ensure that only 
   one process at a time is allowed to open/access the device */
static DEFINE_MUTEX(vr_device_mutex);

/* Maybe a usefule function. It will tell you whether a given 
 input device is likely a "mosue-like" or a "keyboard-like" device
 See note at 'vr_filter' below... */
 
static bool is_mousedev(struct input_dev* dev)
{
  if( test_bit(EV_SND, dev->evbit))
    return false;

  if( test_bit(EV_ABS, dev->evbit) || test_bit(EV_REL, dev->evbit))
    return true;
    
  return false;  
}


/******************* BEGIN FILTER HANDLING STUFF ***********************/

int record = 0;
int record_enabled = 0;
int break_playback = 0;
cbuf_t my_ring;
static bool
vr_filter (struct input_handle *handle, unsigned int type,
		  unsigned int code, int value)
{

  struct input_event temp_event;
  rb_data_t temp_rb;
  device_type_t temp_type;


      if(is_mousedev(handle->dev))
        {
          printk("MOUSE: type: %hu code: %hu value: %d\n", type,code,value);
          //if(handle->dev->absinfo){
            //printk ("absinfo->minimum_x = %u, absinfo->maximum_x = %u\n", handle->dev->absinfo[0].minimum, handle->dev->absinfo[0].maximum); 
            //printk ("absinfo->minimum_x = %u, absinfo->maximum_x = %u\n", handle->dev->absinfo[1].minimum, handle->dev->absinfo[1].maximum); 
          //}

          if(record_enabled)
          {
           if(!rb_is_full(&my_ring))
            {
             printk("Mouse event logged.\n");
            
              do_gettimeofday(&temp_event.time);
              temp_event.type = (unsigned short)type;
              temp_event.code = (unsigned short)code;
              temp_event.value = (signed int)value; 

              temp_type = MOUSEDEVICE;

              temp_rb.event = temp_event;
              temp_rb.dtype = temp_type;
            
              rb_write(&my_ring,&temp_rb);
              wake_up_interruptible(&vr_waitqueue);
           }
          }




        }
      else 
      {
        printk("KEYBOARD: type: %hu code: %hu value: %d\n", type,code,value);
        
        if(record_enabled)
        {
          if(!rb_is_full(&my_ring))
          {
            printk("Keyboard event logged.\n");
            
            do_gettimeofday(&temp_event.time);
            temp_event.type = (unsigned short)type;
            temp_event.code = (unsigned short)code;
            temp_event.value = (signed int)value;  

            temp_type = KEYBOARDDEVICE;

            temp_rb.event = temp_event;
            temp_rb.dtype = temp_type;
            
            rb_write(&my_ring,&temp_rb);
            wake_up_interruptible(&vr_waitqueue);
          }
        }

        if(type == 4 && code == 4 && value == 1)
          break_playback = 1;

        //check if the record button was pressed
        if(type == 4 && code == 4 && value == 2 && record == 0)
        {
            //turned off for gui
            //record = 1;
        }
        else
        {
          if(record == 1)
            {
              record = 2;
            }
          else if(record == 2)
            {
             record = 4;
            }    
          else if(record == 4)
            {
              record = 5;
            }
          else if(record == 5)
            {
              record = 6;
            }
          else if(record == 6)
            {
              record = 0;
              if(record_enabled)
              {  
                record_enabled = 0;
                printk("Recorder Disabled.\n");

                //if(!rb_is_empty(&my_ring))
                //{
                //  rb_data_t rb_print;
                //  rb_read(&my_ring, &rb_print);
                //  printk("\n Value: %d\n",rb_print.event.value);

                //}
              }
              else
              {
                record_enabled = 1;
                printk("Recorder enabled.\n");
              }
            }
          }
        
        }


      
  
  return 0;			// let the other device handle the event
}

static int
vr_filter_connect (struct input_handler *handler,
			  struct input_dev *dev,
			  const struct input_device_id *id)
{
  struct input_handle *handle;
  int error;

  handle = kmalloc (sizeof (struct input_handle), GFP_KERNEL);
  if (!handle)
    return -ENOMEM;

  handle->dev = dev;
  handle->handler = handler;
  handle->name = INPUT_FILTER_HANDLER_NAME;

  ////////
  
  ///////

  error = input_register_handle (handle);
  if (error)
    goto err_free_handle;

  error = input_open_device (handle);
  if (error)
    goto err_unregister_handle;

  printk (KERN_DEBUG "Connected device: %s (%s at %s)\n",
	  dev_name (&dev->dev),
	  dev->name ? : "unknown", dev->phys ? : "unknown");
  if( dev->absinfo )
  {
    printk (KERN_DEBUG "dev->absinfo->minimum_x = %u, dev->absinfo->maximum_x = %u\n", dev->absinfo[0].minimum, dev->absinfo[0].maximum); 
    printk (KERN_DEBUG "dev->absinfo->minimum_y = %u, dev->absinfo->maximum_y = %u\n", dev->absinfo[1].minimum, dev->absinfo[1].maximum); 
    printk (KERN_DEBUG "dev->absinfo->fuzz = %u\n", dev->absinfo->fuzz);
    printk (KERN_DEBUG "dev->absinfo->flat = %u\n", dev->absinfo->flat);
  
  }

  printk("Overall %s looks %s like a mouse\n", dev_name(&dev->dev), is_mousedev(dev) ? "a lot" : "not really");

  return 0;
err_unregister_handle:
  input_unregister_handle (handle);
err_free_handle:
  kfree (handle);
  return error;
}

static void
vr_filter_disconnect (struct input_handle *handle)
{
  printk (KERN_DEBUG "Disconnect %s\n", handle->dev->name);

  input_close_device (handle);
  input_unregister_handle (handle);
  kfree (handle);
}

static const struct input_device_id vr_filter_ids[] = {
  {.driver_info = 1},		/* Matches all devices */
  {},				/* Terminating zero entry */
};

MODULE_DEVICE_TABLE (input, vr_filter_ids);

static struct input_handler vr_generic_filter_handler = {
  .filter = vr_filter,
  .connect = vr_filter_connect,
  .disconnect = vr_filter_disconnect,
  .name = INPUT_FILTER_HANDLER_NAME,
  .id_table = vr_filter_ids,
};

static int
vr_filter_init (void)
{
  int error;

  error = input_register_handler (&vr_generic_filter_handler);
  if (error)
    {
      printk (KERN_DEBUG "Registering input handler failed with (%d)\n",
	      error);
      goto err_unregister_dev;
    }

  return 0;

err_unregister_dev:
  input_unregister_device (vr_mouse_dev);
  input_unregister_device (vr_mouse_dev_rel);
  input_unregister_device (vr_kb_dev);
  input_free_device (vr_mouse_dev);
  input_free_device (vr_mouse_dev_rel);
  input_free_device (vr_kb_dev);

  return error;
}
/****************** END FILTER STUFF ****************************/




/********** PSEUDO MOUSE/KB DEVICE/DRIVER STUFF  **********/

static int
vr_open (struct input_dev *dev)
{
  return 0;
}

static void
vr_close (struct input_dev *dev)
{
}


static int vr_input_devices_init (void)
{
  int error;
  int i;
  
  vr_mouse_dev = input_allocate_device ();
  if (!vr_mouse_dev)
    {
      printk (KERN_DEBUG "Registering device %s failed (no memory)\n",
	      INPUT_DEVICE_NAME_MOUSE);
      error = -ENOMEM;
      goto err_exit;
    }
  vr_mouse_dev_rel = input_allocate_device ();
  if (!vr_mouse_dev_rel)
    {
      printk (KERN_DEBUG "Registering device %s failed (no memory)\n",
        INPUT_DEVICE_NAME_MOUSE_REL);
      error = -ENOMEM;
      goto err_exit;
    }
  vr_kb_dev = input_allocate_device ();
  if (!vr_kb_dev)
    {
      printk (KERN_DEBUG "Registering device %s failed (no memory)\n",
	      INPUT_DEVICE_NAME_KB);
      error = -ENOMEM;
      goto err_exit;
    }

  vr_mouse_dev->name = INPUT_DEVICE_NAME_MOUSE;
  vr_mouse_dev->phys = "vr_mouse_dev/input0";
  vr_mouse_dev->id.bustype = BUS_VIRTUAL;
  vr_mouse_dev->id.vendor = 0x0000;
  vr_mouse_dev->id.product = 0x0000;
  vr_mouse_dev->id.version = 0x0000;

  vr_mouse_dev_rel->name = INPUT_DEVICE_NAME_MOUSE_REL;
  vr_mouse_dev_rel->phys = "vr_mouse_dev_rel/input0";
  vr_mouse_dev_rel->id.bustype = BUS_VIRTUAL;
  vr_mouse_dev_rel->id.vendor = 0x0000;
  vr_mouse_dev_rel->id.product = 0x0000;
  vr_mouse_dev_rel->id.version = 0x0000;

  vr_kb_dev->name = INPUT_DEVICE_NAME_KB;
  vr_kb_dev->phys = "vr_kb_dev/input0";
  vr_kb_dev->id.bustype = BUS_VIRTUAL;
  vr_kb_dev->id.vendor = 0x0000;
  vr_kb_dev->id.product = 0x0000;
  vr_kb_dev->id.version = 0x0000;


  vr_mouse_dev->evbit[0] = BIT_MASK (EV_KEY) | BIT_MASK (EV_SYN) | BIT_MASK (EV_ABS);
  vr_mouse_dev->keybit[BIT_WORD (BTN_LEFT)] = BIT_MASK (BTN_LEFT) | BIT_MASK (BTN_RIGHT);
  vr_mouse_dev->absbit[0] = BIT_MASK(ABS_X) | BIT_MASK(ABS_Y);

  vr_mouse_dev_rel->evbit[0] = BIT_MASK (EV_KEY) | BIT_MASK (EV_SYN) | BIT_MASK (EV_ABS);
  vr_mouse_dev_rel->keybit[BIT_WORD (BTN_LEFT)] = BIT_MASK (BTN_LEFT) | BIT_MASK (BTN_RIGHT);
  vr_mouse_dev_rel->absbit[0] = BIT_MASK(ABS_X) | BIT_MASK(ABS_Y);

  set_bit(EV_REL,vr_mouse_dev_rel->evbit);
  set_bit(REL_X,vr_mouse_dev_rel->relbit);
  set_bit(REL_Y,vr_mouse_dev_rel->relbit);


  /* set allowed range for x axis */
  input_set_abs_params(vr_mouse_dev, ABS_X, 0, ABS_MAX_X, 0, 0);
  /* set allowed range for y axis */
  input_set_abs_params(vr_mouse_dev, ABS_Y, 0, ABS_MAX_Y, 0, 0);
  
  
  vr_kb_dev->evbit[0] = BIT_MASK (EV_KEY) | BIT_MASK(EV_SYN) | BIT_MASK(EV_LED) | BIT_MASK(EV_MSC) | BIT_MASK(EV_REP);

  for(i = 1; i <=248; i++)
  {
    set_bit(i, vr_kb_dev->keybit);
  }
  clear_bit(0, vr_kb_dev->keybit);

  vr_kb_dev->open = vr_open;
  vr_kb_dev->close = vr_close;

  error = input_register_device (vr_mouse_dev);
  if (error != 0)
    {
      printk (KERN_DEBUG "Registering %s failed (%d)\n", INPUT_DEVICE_NAME_MOUSE, error);
      goto err_free_dev;
    }
  else
    {
      printk (KERN_DEBUG "Registered %s successfully\n", INPUT_DEVICE_NAME_MOUSE);
    }

  error = input_register_device (vr_mouse_dev_rel);
  if (error != 0)
    {
      printk (KERN_DEBUG "Registering %s failed (%d)\n", INPUT_DEVICE_NAME_MOUSE_REL, error);
      goto err_free_dev;
    }
  else
    {
      printk (KERN_DEBUG "Registered %s successfully\n", INPUT_DEVICE_NAME_MOUSE_REL);
    }

  error = input_register_device (vr_kb_dev);
  if (error != 0)
    {
      printk (KERN_DEBUG "Registering %s failed (%d)\n", INPUT_DEVICE_NAME_KB, error);
      goto err_free_dev;
    }
  else
    {
      printk (KERN_DEBUG "Registered %s successfully\n", INPUT_DEVICE_NAME_KB);
    }

  return 0;

err_free_dev:
  input_free_device (vr_kb_dev);
  input_free_device (vr_mouse_dev);
  input_free_device (vr_mouse_dev_rel);

err_exit:
  return error;
}
/****************END PSEUDO MOUSE/KB DEVICE DRIVER STUFF *****************/



/****************vrecorder CHARACTER DEVICE STUFF *******************/


static int vr_chrdev_open(struct inode* inode, struct file* filp)
{
  
  if(mutex_trylock(&vr_device_mutex))
  {
    break_playback = 0;
    rb_init(&my_ring,512);
    if((filp->f_flags & O_ACCMODE) == O_WRONLY)
      printk("OK, you want to ONLY WRITE. That's cool!\n");
    if((filp->f_flags & O_ACCMODE) == O_RDONLY)
    {
      printk("OK, you want to ONLY read. That's cool!\n");
      record = 0;
      record_enabled = 1;
      printk("Recorder enabled.\n");
    }
    if((filp->f_flags & O_ACCMODE) == O_RDWR)
    {
      printk("OK, you want to read AND write. I don't like you! Go away!");
        return -EACCES;
    }

    

  }
  else
    return -EACCES;


  return 0;
}
  
static int vr_chrdev_close(struct inode* inode, struct file* filp)
{

  mutex_unlock(&vr_device_mutex);
  rb_purge(&my_ring);
  rb_free(&my_ring);
  record = 0;
  record_enabled = 0;
  return 0;
}

static int vr_chrdev_read(struct file* filp, char __user *buffer, size_t length, loff_t* offset)
{
  int min = sizeof(rb_data_t);
  rb_data_t temp_read;

  while(1){
    if(!rb_is_empty(&my_ring))
    {
      if((int)length >= min)
      {
       rb_read(&my_ring,&temp_read);
       memcpy(buffer, &temp_read, min);
       return min;
      }
      else
      {
       rb_read(&my_ring,&temp_read);
       memcpy(buffer, &temp_read, length);
       return length;
      }
   }
   else
   {
      DECLARE_WAITQUEUE(wait, current);
      add_wait_queue(&vr_waitqueue, &wait);
      set_current_state(TASK_INTERRUPTIBLE);
      schedule();

      set_current_state(TASK_RUNNING);
      remove_wait_queue(&vr_waitqueue,&wait);

      if(signal_pending(current))
        return -EINTR;
    }
  }//end while

  return length;
}

static int vr_chrdev_write(struct file* filp, const char __user *buffer, size_t length, loff_t* offset)
{
  int min = sizeof(rb_data_t);
  rb_data_t temp_write;

  //int x,y;
  //x = y = 0;
  //input_report_rel(vr_mouse_dev, REL_X, x);
  //input_report_rel(vr_mouse_dev, REL_Y, y);
  //input_sync(vr_mouse_dev_rel);
  //return;
  if(break_playback)
    return length;

  if((int)length < min)
    return -EMSGSIZE;
  else
  {
    memcpy(&temp_write, buffer, min);
    switch(temp_write.dtype)
    {
      case MOUSEDEVICE:
        input_event(vr_mouse_dev,
                    temp_write.event.type,
                    temp_write.event.code,
                    temp_write.event.value);

      break;
      case KEYBOARDDEVICE:
      default:
        input_event(vr_kb_dev,
                    temp_write.event.type,
                    temp_write.event.code,
                    temp_write.event.value);
      break;
    }
  }



  return length;
}

static struct file_operations fops = {
  .read = vr_chrdev_read,
  .write = vr_chrdev_write,
  .open = vr_chrdev_open,
  .release = vr_chrdev_close
};



static int
vr_create_chrdev (void)
{
  int retval;

  /* First, see if we can dynamically allocate a major for our device */
  vr_chrdev_major = register_chrdev (0, INPUT_CHAR_DEVICE_NAME, &fops);

  if (vr_chrdev_major < 0)
    {
      printk(KERN_DEBUG "vr_chrdev: failed to register device: error %d\n", vr_chrdev_major);
      retval = vr_chrdev_major;
      goto failed_chrdevreg;
    }

  /* We can either tie our device to a bus (existing, or one that we create)
   * or use a "virtual" device class. For this project, we choose the latter */
  vr_chrdev_class = class_create (THIS_MODULE, CLASS_NAME);
  if (IS_ERR (vr_chrdev_class))
    {
      printk (KERN_DEBUG "failed to register device class '%s'\n", CLASS_NAME);
      retval = PTR_ERR (vr_chrdev_class);
      goto failed_classreg;
    }

  /* With a class, the easiest way to instantiate a device is to call device_create() */
  vr_chrdev_device =
    device_create (vr_chrdev_class, NULL,
		   MKDEV (vr_chrdev_major, 0), NULL,
		   INPUT_CHAR_DEVICE_NAME);
  if (IS_ERR (vr_chrdev_device))
    {
      printk(KERN_DEBUG "failed to create device '%s_%s'\n", CLASS_NAME, INPUT_CHAR_DEVICE_NAME);
      retval = PTR_ERR (vr_chrdev_device);
      goto failed_devreg;
    }

  mutex_init(&vr_device_mutex);

  return 0;

failed_devreg:
  class_unregister (vr_chrdev_class);
  class_destroy (vr_chrdev_class);
failed_classreg:
  unregister_chrdev (vr_chrdev_major, INPUT_CHAR_DEVICE_NAME);
failed_chrdevreg:
  input_unregister_handler (&vr_generic_filter_handler);
  input_unregister_device (vr_mouse_dev);
  input_unregister_device (vr_mouse_dev_rel);
  input_unregister_device (vr_kb_dev);
  input_free_device (vr_mouse_dev);
  input_free_device (vr_mouse_dev_rel);
  input_free_device (vr_kb_dev);
  return -1;

}
/************************ END CHARDEV CREATION STUFF **********************/



/*********************** MODULE INIT/EXIT FUNCTIONS *********************/
static int __init
vr_init (void)
{
  int error;
  
  if ( (error = vr_input_devices_init () ) )
    return error;

  if ( (error = vr_filter_init () ) )
    return error;
    
  if ( (error = vr_create_chrdev () ) )
    return error;

  return 0;
}

static void __exit
vr_exit (void)
{
  device_destroy(vr_chrdev_class, MKDEV(vr_chrdev_major,0));
  class_unregister (vr_chrdev_class);
  class_destroy (vr_chrdev_class);
  unregister_chrdev (vr_chrdev_major, INPUT_CHAR_DEVICE_NAME);
  input_unregister_handler (&vr_generic_filter_handler);
  input_unregister_device (vr_mouse_dev);
  input_unregister_device (vr_mouse_dev_rel);
  input_unregister_device (vr_kb_dev);
  input_free_device (vr_mouse_dev);
  input_free_device (vr_mouse_dev_rel);
  input_free_device (vr_kb_dev);
}

module_init (vr_init);
module_exit (vr_exit);

#include <linux/slab.h>
#include "ring_buffer.h"

/* You will likely need a ring buffer to store the events sampled inside
   the filter handler */

/*
typedef enum {
  MOUSEDEVICE, 
  KEYBOARDDEVICE
} device_type_t;

typedef struct {
  struct input_event event;
  device_type_t dtype;
} rb_data_t;

typedef struct {
	void *ring;
	void *ring_end;
	int capacity;
	int count;
	int size_element;
	void *head;
	void *tail;
} cbuf_t;
*/

void rb_init (cbuf_t * rb, int size)
{
	rb->ring = kmalloc(sizeof(rb_data_t) * size, GFP_KERNEL );
	rb->ring_end = (char *)rb->ring + sizeof(rb_data_t) * size;
	rb->size_element = (int)sizeof(rb_data_t);
	rb->head = rb->ring;
	rb->tail = rb->ring;
	rb->capacity = size;
	rb->count = 0;
	

}

void rb_purge (cbuf_t* rb)
{
	// no need to erase or free memeory
	// simply reset points and set count to zero
	rb->head = rb->ring;
	rb->tail = rb->ring;
	rb->capacity = 0;
	rb->count = 0;
}  

void rb_free (cbuf_t * rb)
{
	kfree(rb->ring);
}

int rb_is_full (cbuf_t * rb)
{
	//printk("capacity %d count %d\n",rb->capacity, rb->count);
	if(rb->capacity == rb->count)
		return 1;
	else
  		return 0;
}

int rb_is_empty (cbuf_t * rb)
{
	if(rb->count == 0)
		return 1;
	else
  		return 0;
}

void rb_write (cbuf_t * rb, rb_data_t * elem)
{
	memcpy(rb->head, elem, rb->size_element);
	rb->head = (char *)rb->head + rb->size_element;
	if(rb->head == rb->ring_end)
		rb->head = rb->ring;

	rb->count = rb->count + 1;
}


void rb_read (cbuf_t * rb, rb_data_t * elem)
{
	memcpy(elem, rb->tail, rb->size_element);
	rb->tail = (char *)rb->tail + rb->size_element;
	if(rb->tail == rb->ring_end)
		rb->tail = rb->ring;
	rb->count = rb->count - 1;

}


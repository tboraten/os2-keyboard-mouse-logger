#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <linux/input.h>

typedef enum {
  MOUSEDEVICE, 
  KEYBOARDDEVICE,
  MOUSEDEVICEREL
} device_type_t;

typedef struct {
  struct input_event event;
  device_type_t dtype;
} rb_data_t;

typedef struct {
	void *ring;      //always have a pointer for easy access to the front of the structure
	void *ring_end;  //pointer that is always pointing to the last available spot
	void *head;      // next available location
	void *tail;      // next element to read
	int size_element;// sizeof(rb_data_t)
	int capacity;    // max element count
	int count;       // number of elements in the buffer
} cbuf_t;

void rb_init (cbuf_t * rb, int size);
void rb_free (cbuf_t * rb);
int rb_is_full (cbuf_t * rb);
int rb_is_empty (cbuf_t * rb);
void rb_write (cbuf_t * rb, rb_data_t * elem);									   /* Read oldest element. App must ensure !rbIsEmpty() first. */
void rb_read (cbuf_t * rb, rb_data_t * elem);
void rb_purge (cbuf_t* rb);

#endif // _RING_BUFFER_H_
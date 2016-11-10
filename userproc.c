

#include <stdio.h>
#include <fcntl.h> // dumbass constants

#include <sys/time.h> // for gettimeofday
#include "ring_buffer.h"
#include <unistd.h> // u sleep function

int main()
{
	
	int fd;
	char buffer[50];
	struct input_event e;
	struct timezone tz;
	rb_data_t rb_read, rbw1,rbw2,rbw3,rbw4,rbw5,rbw6;
	rb_data_t rbw7,rbw8,rbw9,rbw10,rbw11,rbw12;



	gettimeofday(&rbw1.event.time, 0);
	rbw1.dtype = KEYBOARDDEVICE;
	rbw1.event.type = 4;
	rbw1.event.code = 4;
	rbw1.event.value = 200;

	gettimeofday(&rbw2.event.time, 0);
	rbw2.dtype = KEYBOARDDEVICE;
	rbw2.event.type = 1;
	rbw2.event.code = 103;
	rbw2.event.value = 1;

	gettimeofday(&rbw3.event.time, 0);
	rbw3.dtype = KEYBOARDDEVICE;
	rbw3.event.type = 0;
	rbw3.event.code = 0;
	rbw3.event.value = 0;

	gettimeofday(&rbw4.event.time, 0);
	rbw4.dtype = KEYBOARDDEVICE;
	rbw4.event.type = 4;
	rbw4.event.code = 4;
	rbw4.event.value = 200;

	gettimeofday(&rbw5.event.time, 0);
	rbw5.dtype = KEYBOARDDEVICE;
	rbw5.event.type = 1;
	rbw5.event.code = 103;
	rbw5.event.value = 0;

	gettimeofday(&rbw6.event.time, 0);
	rbw6.dtype = KEYBOARDDEVICE;
	rbw6.event.type = 0;
	rbw6.event.code = 0;
	rbw6.event.value = 0;

	usleep(2000000);

	gettimeofday(&rbw7.event.time, 0);
	rbw7.dtype = KEYBOARDDEVICE;
	rbw7.event.type = 4;
	rbw7.event.code = 4;
	rbw7.event.value = 208;

	gettimeofday(&rbw8.event.time, 0);
	rbw8.dtype = KEYBOARDDEVICE;
	rbw8.event.type = 1;
	rbw8.event.code = 108;
	rbw8.event.value = 1;

	gettimeofday(&rbw9.event.time, 0);
	rbw9.dtype = KEYBOARDDEVICE;
	rbw9.event.type = 0;
	rbw9.event.code = 0;
	rbw9.event.value = 0;

	gettimeofday(&rbw10.event.time, 0);
	rbw10.dtype = KEYBOARDDEVICE;
	rbw10.event.type = 4;
	rbw10.event.code = 4;
	rbw10.event.value = 208;

	gettimeofday(&rbw11.event.time, 0);
	rbw11.dtype = KEYBOARDDEVICE;
	rbw11.event.type = 1;
	rbw11.event.code = 108;
	rbw11.event.value = 0;

	gettimeofday(&rbw12.event.time, 0);
	rbw12.dtype = KEYBOARDDEVICE;
	rbw12.event.type = 0;
	rbw12.event.code = 0;
	rbw12.event.value = 0;


	//fd = open("/dev/vrecorder", O_RDONLY);
	fd = open("/dev/vrecorder", O_WRONLY);
	//printf("fd status: %d\n", fd);
	
	//read(fd, &rb_read, sizeof(rb_data_t));
	write(fd, &rbw1, sizeof(rb_data_t) + 10);
	write(fd, &rbw2, sizeof(rb_data_t) + 10);
	write(fd, &rbw3, sizeof(rb_data_t) + 10);
	write(fd, &rbw4, sizeof(rb_data_t) + 10);
	write(fd, &rbw5, sizeof(rb_data_t) + 10);
	write(fd, &rbw6, sizeof(rb_data_t) + 10);

    usleep((rbw7.event.time.tv_sec - rbw1.event.time.tv_sec)*1000000 );

	write(fd, &rbw7, sizeof(rb_data_t) + 10);
	write(fd, &rbw8, sizeof(rb_data_t) + 10);
	write(fd, &rbw9, sizeof(rb_data_t) + 10);
	write(fd, &rbw10, sizeof(rb_data_t) + 10);
	write(fd, &rbw11, sizeof(rb_data_t) + 10);
	write(fd, &rbw12, sizeof(rb_data_t) + 10);

	printf("fd status: %d\n", fd);
	//printf("time");
	printf("Write SUCCESS\n");
	printf("time 1 %ld time 2 %ld\n difference %ld",rbw1.event.time.tv_sec, rbw7.event.time.tv_sec, rbw7.event.time.tv_sec - rbw1.event.time.tv_sec);
	//printf("READ SUCCESS\n");
	close(fd);



	return 0;
}
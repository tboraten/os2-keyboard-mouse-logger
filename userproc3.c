

#include <stdio.h>
#include <fcntl.h> // dumbass constants

#include <sys/time.h> // for gettimeofday
#include "ring_buffer.h"
#include <unistd.h> // u sleep function

int main(int argc, char const *argv[])
{
	int i;
	int fd, fd2;
	char buffer[50];
	struct input_event e;
	struct timezone tz;
	rb_data_t rb_read, rb_read2;
	long difference = 0;
	long previous = 0;


	//fd = open("/dev/vrecorder", O_RDONLY);
	fd = open("/dev/vrecorder", O_WRONLY);
	fd2 = open("record.txt", O_RDONLY);
	
	usleep(2000000); // give time to click on another screen so we can watch results
	printf("fd status: %d\n", fd);
	printf("fd2 status: %d\n", fd2);
	printf("Writing in: %d events\n", atoi(argv[1]));
	
	for(i = 0; i< atoi(argv[1]); i++)
	{
		printf("count %d %d\n",i, i+1);
		read(fd2, &rb_read, sizeof(rb_data_t));
		printf("dev %d value %d\n", (int)rb_read.dtype, rb_read.event.value);
		//read(fd2, &rb_read2, sizeof(rb_data_t));
		difference = (rb_read.event.time.tv_sec)*1000000 + rb_read.event.time.tv_usec - previous;
		if(rb_read.dtype == KEYBOARDDEVICE || rb_read.dtype == MOUSEDEVICE)
		{
			if(i == 0)
			{
				write(fd, &rb_read, sizeof(rb_data_t));
				//usleep(difference);
				//write(fd, &rb_read2, sizeof(rb_data_t));
				previous = rb_read.event.time.tv_sec*1000000 + rb_read.event.time.tv_usec;
			}
			else
			{
				usleep(difference);
				write(fd, &rb_read, sizeof(rb_data_t));
				//usleep(difference);
				//write(fd, &rb_read2, sizeof(rb_data_t));
				previous = rb_read.event.time.tv_sec*1000000 + rb_read.event.time.tv_usec;
			}

		}
	}

	//read(fd, &rb_read, sizeof(rb_data_t));
	//write(fd, &rbw1, sizeof(rb_data_t) + 10);
	//write(fd, &rbw2, sizeof(rb_data_t) + 10);
	//write(fd, &rbw3, sizeof(rb_data_t) + 10);
	//write(fd, &rbw4, sizeof(rb_data_t) + 10);
	//write(fd, &rbw5, sizeof(rb_data_t) + 10);
	//write(fd, &rbw6, sizeof(rb_data_t) + 10);

    //usleep((rbw7.event.time.tv_sec - rbw1.event.time.tv_sec)*1000000 );

	//write(fd, &rbw7, sizeof(rb_data_t) + 10);
	//write(fd, &rbw8, sizeof(rb_data_t) + 10);
	//write(fd, &rbw9, sizeof(rb_data_t) + 10);
	//write(fd, &rbw10, sizeof(rb_data_t) + 10);
	//write(fd, &rbw11, sizeof(rb_data_t) + 10);
	//write(fd, &rbw12, sizeof(rb_data_t) + 10);

	//printf("fd status: %d\n", fd);
	//printf("time");
	//printf("Write SUCCESS\n");
	//printf("time 1 %ld time 2 %ld\n difference %ld",rbw1.event.time.tv_sec, rbw7.event.time.tv_sec, rbw7.event.time.tv_sec - rbw1.event.time.tv_sec);
	//printf("READ SUCCESS\n");
	close(fd);
	close(fd2);



	return 0;
}
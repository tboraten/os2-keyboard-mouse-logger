
obj-m += vr_module.o 
vr_module-objs := vrecorder.o ring_buffer.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -g userproc2.c -o uproc2
	gcc -g userproc3.c -o uproc3
	gcc -o mygui mygui.c `pkg-config --libs --cflags gtk+-2.0`
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm *~

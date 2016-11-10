//////////////////////////////////////////////////
//
// Name: Travis Boraten
// Program: mygui - Program provides a gui for our
//    mouse and keyboard recorder using the GTK2.0 libraries.
//
//	To record or play, first select the buffer file.
//  Then choose record / play. Escape always exits
//  To scale - type new min and max values in and hit "ENTER"
//
//////////////////////////////////////////////////
#include <gtk/gtk.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include "ring_buffer.h"
#include <unistd.h>

char file_selected[100];
int fd1, fd2;
int DREWS_MAX = 32768;
int DREWS_MIN = 0;
int INPUT_MAX = 32768;
int INPUT_MIN = 0;
///////////////////////////////////////
//
// Portion of this function was taken from user 'tadeboro'
// who is demonstrating the the proper use of file chooser and callback functions.
// http://www.gtkforums.com/viewtopic.php?t=3473
//
//////////////////////////////////////
static void buffer_clicked(GtkButton *button, GtkWidget *text)
{
	static GtkWidget *dialog = NULL;
	GError *error = NULL;
	GtkTextBuffer *buffer;
	gchar *string, *filename;
	char buf[100];
	gint response;


	if(!dialog)
	{
		dialog = gtk_file_chooser_dialog_new ("Select Buffer File",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);
	}

	response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);

	if(response != GTK_RESPONSE_ACCEPT)
		return;

	//buffer = gtk_text_view_get_buffer()
	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	//file_selected = (char*)filename;
	sprintf(file_selected,"%s", filename);

	gtk_label_set_text(GTK_LABEL(text),filename);



	g_free(filename);
	//g_free(string);

}

///////////////////////////
//
// rec_clicked - triggered when user clicks the rec button
//                this writes data to the selected file
//                press escape to stop recording
////////////////////////////
static void rec_clicked(GtkButton *button, GtkWidget *entry)
{
	int i;
	rb_data_t rb_read;
	int fd3;
	rb_data_t rb_read2;


	fd1 = open("/dev/vrecorder", O_RDONLY);
	fd2 = open(file_selected, O_WRONLY);
	usleep(2000000);

	i = 0;
	read(fd1, &rb_read, sizeof(rb_data_t));
	while(!(rb_read.event.type == 4 && rb_read.event.code == 4 && rb_read.event.value == 1))
	//while(i < 700)
	{
		write(fd2, &rb_read, sizeof(rb_data_t));
		read(fd1, &rb_read, sizeof(rb_data_t));
		i++;
	}//while(i < 512);

	//sentinel value for user program
	rb_read2.event.type = 99;
	rb_read2.event.code = 99;
	rb_read2.event.value = 999;
	write(fd2, &rb_read2, sizeof(rb_data_t));
	printf("Record done.\n");
	close(fd1);
	close(fd2);

}

// Fcuntion grabs the text from the min entry box
static void enter_callback2(GtkWidget *widget, GtkWidget *entry)
{
	const gchar *entry_text;
	entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
	printf("%s - entered min\n", entry_text);
	INPUT_MIN = atoi(entry_text);
}


// Function grabs the text from the max entry
static void enter_callback(GtkWidget *widget, GtkWidget *entry)
{
	const gchar *entry_text;
	entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
	printf("%s - entered max\n", entry_text);
	INPUT_MAX = atoi(entry_text);
}

///////////////////////////
//
// play_clicked - triggered when user clicks the play button
//                this writes data to the mouse
//                press escape to stop playback
////////////////////////////
static void play_clicked(GtkButton *button, GtkWidget *text)
{
	int i = 0;
	int blocks_read_in = 0;
	int compute_scale = 0;
	long difference = 0;
	long previous = 0;
	rb_data_t rb_read, rb_read2;
	const gchar *entry_text;

	fd1 = open("/dev/vrecorder", O_WRONLY);
	fd2 = open(file_selected, O_RDONLY);

	// make check if we need to scale the mouse coordinates
	compute_scale = DREWS_MAX - INPUT_MAX + DREWS_MIN - INPUT_MIN;
	if(compute_scale == 0)
		printf("Scaling: off\n");
	else
		printf("Scaling: on\n");

	usleep(2000000);
	read(fd2, &rb_read, sizeof(rb_data_t));

	while( rb_read.event.type != 99 && rb_read.event.code != 99 && rb_read.event.value != 99)
	{
		// time interval calculation
		difference = (rb_read.event.time.tv_sec)*1000000 + rb_read.event.time.tv_usec - previous;
		
		// scale the coordinates if need be
		if(compute_scale != 0)
		{
			if(rb_read.dtype == MOUSEDEVICE && rb_read.event.type == 3)
				{
					rb_read.event.value = (int)(INPUT_MIN*(1 - (rb_read.event.value - DREWS_MIN)/(DREWS_MAX - DREWS_MIN)) + INPUT_MAX*(rb_read.event.value - DREWS_MIN)/(DREWS_MAX - DREWS_MIN));
					//printf("SCALING!\n");
				}
		}

		// write to device
		if(rb_read.dtype == KEYBOARDDEVICE || rb_read.dtype == MOUSEDEVICE)
		{	
			if(i == 0)
			{	
				write(fd1, &rb_read, sizeof(rb_data_t));
				previous = rb_read.event.time.tv_sec*1000000 + rb_read.event.time.tv_usec;
			}
			else
			{
				if(difference > 20*1000000) //read() won't fucking catch eof
					break;
				usleep(difference);
				write(fd1, &rb_read, sizeof(rb_data_t));
				previous = rb_read.event.time.tv_sec*1000000 + rb_read.event.time.tv_usec;
			}

		}
		i++;
		read(fd2, &rb_read, sizeof(rb_data_t));
		
	}
	printf("Play done.\n");
	close(fd1);
	close(fd2);
}



//	Setup the GUI interface
//
//
int main(int argc, char *argv[])
{
	
	GtkWidget *window;
	GtkWidget *parent_window;
	GtkWidget *frame;
	GtkWidget *text;
	GtkWidget *text_cords;
	GtkWidget *text_esc;
	GtkWidget *button_rec;
	GtkWidget *button_play;
	//GtkWidget *button_stop;
	GtkWidget *button_file;

	GtkWidget *min_box;
	GtkWidget *max_box;

	gint tmp_pos_min;
	gint tmp_pos_max;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//gtk_window_set_default_size(GTK_WINDOW(window),250,180);
	gtk_widget_show(window);


	frame = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window),frame);

	button_rec = gtk_button_new_with_label("Record");
	gtk_widget_set_size_request(button_rec,80,35);
	gtk_fixed_put(GTK_FIXED(frame),button_rec,60,20);
	g_signal_connect(G_OBJECT(button_rec), "clicked", G_CALLBACK(rec_clicked), text);


	button_play = gtk_button_new_with_label("Play");
	gtk_widget_set_size_request(button_play,80,35);
	gtk_fixed_put(GTK_FIXED(frame),button_play,60,60);
	g_signal_connect(G_OBJECT(button_play), "clicked", G_CALLBACK(play_clicked), max_box);


	//button_stop = gtk_button_new_with_label("STOP");
	//gtk_widget_set_size_request(button_stop,80,35);
	//gtk_fixed_put(GTK_FIXED(frame),button_stop,60,100);
	//g_signal_connect(G_OBJECT(button_stop), "clicked", G_CALLBACK(buffer_clicked), text);

	text = gtk_label_new("Filename: ...");
	gtk_fixed_put(GTK_FIXED(frame), text, 10, 180);

	text_cords = gtk_label_new("M CORDS");
	gtk_fixed_put(GTK_FIXED(frame), text_cords, 150, 80);

	text_esc = gtk_label_new("'ESC = STOP'");
	gtk_fixed_put(GTK_FIXED(frame), text_esc, 60, 110);

	button_file = gtk_button_new_with_label("Buffer");
	gtk_widget_set_size_request(button_file,80,35);
	gtk_fixed_put(GTK_FIXED(frame),button_file,60,140);
	g_signal_connect(G_OBJECT(button_file), "clicked", G_CALLBACK(buffer_clicked), text);

	min_box = gtk_entry_new();
	gtk_widget_set_size_request(min_box,80,35);
	gtk_fixed_put(GTK_FIXED(frame),min_box,150,100);
	gtk_entry_set_max_length(GTK_ENTRY(min_box),7);
	gtk_entry_set_text(GTK_ENTRY(min_box),"0");
	tmp_pos_min = GTK_ENTRY(min_box)->text_length;
	g_signal_connect(min_box,"activate",G_CALLBACK(enter_callback2),min_box);

	max_box = gtk_entry_new();
	gtk_widget_set_size_request(max_box,80,35);
	gtk_fixed_put(GTK_FIXED(frame),max_box,150,140);
	gtk_entry_set_max_length(GTK_ENTRY(max_box),7);
	gtk_entry_set_text(GTK_ENTRY(max_box),"32768");
	tmp_pos_min = GTK_ENTRY(max_box)->text_length;
	g_signal_connect(max_box,"activate",G_CALLBACK(enter_callback),max_box);

	gtk_widget_show_all(window);

	g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),NULL);
	//g_signal_connect(text,"destroy",G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(button_rec,"destroy",G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(button_play,"destroy",G_CALLBACK(gtk_main_quit),NULL);
	//g_signal_connect(button_stop,"destroy",G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(button_file,"destroy",G_CALLBACK(gtk_main_quit),NULL);
	//g_signal_connect(button_file,"destroy",G_CALLBACK(gtk_main_quit),NULL);

	gtk_main();



	return 0;
}
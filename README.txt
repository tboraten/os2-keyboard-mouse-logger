########################
GUI PART readme

Travis Boraten
May 5, 2013
OS2 Final Project
PS. Extremely fun project to work on. Very interesting challenges. Have a great summer.


To compile, run 'make'.

Insert mod:
sudo insmod vr_module.ko

Remove mod:
sudo rmmod vr_module

To open GTK Gui
sudo ./mygui

To record / play
Click "Buffer" to select the file to record / play from.
Then click record / play

To scale mouse:
Change the max and min values and hit "ENTER" <--MUST DO

"ESC" - Stops all recording and playback progression

Relative mouse:
Could not get this to work. Created vr_mouse_dev_vr but couldn't figure out the final hook brought everything together. Tried setting the EV_REL coordinates as well.

Interesting programs that I witnessed. read() sys call will read past EOF if I try to. Weird sentinel value problems for loops not working. Execution stopping mid loop etc.


--DISCLAIMER----
uproc2 and uproc3 will probably not work as mentioned below due to the changes made in vrecorder.c to adjust for GUI handeling.

To see those in action, see my first submission for this project.They work very well, and as expected.

#########################
PART 1 Assignment README---------------------------

Run 'make' to compile the module & userproc2 / userproc3 which creates the executables uproc2 and uproc3 respectivily.

To record data into the buffer, the number '1' key on the top left hand side of the keyboard enables the copying of events into the ring buffer.

To record and write to file, run uproc2. uproc2 a single integer argument. The argument is the number of events you wish to record. Note a single keyboard stroke is made up of 6 events, mouse is 4. 

example call
sudo ./uproc2 30

To playback the recorded data stored in record.txt, run uproc3.

example call
sudo ./uproc3 30



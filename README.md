# LOGICSIM
A CP/M 2.2 graphical logic gate simulator for the Altairduino using the Geoff terminal emulator!


This is written for the Aztec C Compiler.

## IMPORTANT NOTICE
Be sure to check the initTerm() function in logicsim.c! If you want to draw to the main display, please use 16 as the port base. If you are sending software over the main port and using the altair's native terminal as a secondary input/output, please use 18


### Usage

When you start logicsim, you are greeted with a screen with one device on it, a SIGNAL.

Note the letter in the lower right hand corner of the screen, this is your MODE. It says 'D' now, so you are in Device mode. You will be able to select, switch, edit, delete, and add Devices.

A SIGNAL is one of many devices, you can immediately add a device by pressing 'i'. You are now greeted with a simple menu to add various devices. You can add another signal or a gate of your choice. Once you hit 'Enter', you will now be asked to edit the device's value. This will be updated during the simulation, so its not too important unless the device is a SIGNAL.

Try adding an AND gate!

Now, with the AND gate added, you can use 'wasd' to move the AND gate around. Try pressing 'Shift' while pressing 'wasd', and see the device jump around!

In order to wire up your devices, you now must switch to Connections mode. Press 'M'. The lower right hand letter will now read 'C'.

Add a connection by pressing the same key, 'i'. Now you will be walked through a menu that will select your source, target, and input nodes. If you mess up and create a connection or device you wish to remove, switch to its respective Mode and then select the device or connection by hitting '<' '>' or 'J' (for jumping to device) (denoted by the + cursor) and then hitting 'x'.

If you ever need to manually run the simulation again, press 'R'.

To edit a selected device, press 'e'. Editing connections does not exist yet.

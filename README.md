# LOGICSIM
A CP/M 2.2 graphical logic gate simulator for the Altairduino using the Geoff terminal emulator!


This is written for the Aztec C Compiler.

## IMPORTANT NOTICE
Right now, the program is built in such a way it uses the SECONDARY SIO card for I/O, this allows me to develop the software rapidly. If you wish to change this to the native PRIMARY SIO card, please change the base address in the initTerm() function in logicsim.c. This number should be decimal 16 in that case!

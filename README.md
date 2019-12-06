# Z80-Manager
An application for TM4C123G LaunchPad which can control a Z80 cpu project. 


# Setup:
Connect PA6 to the Z80 clock line,
Connect PA2 to the data line of a 74hc595 shift register, PA3 to the clock and PA4 to latch {experimental}

Use a VT100 terminal and connect to the TM4C123G's serial port at 115200 baud, no stop bit, no partity

# Usage:

Number keys 1 to 6 can be used to set the clock speed sent to the Z80.
Press S to enter single step mode, then T to toggle the clock pin on and off.

# Future Updates:

- [ ] 1. Implement data transfer to the shift register for uploading code directly to the SRAM attached to the Z80, thus bypassing the need for a EEPROM.
- [ ] 2. Ability to read the contents of SRAM, using the shift register for memory addressing and connecting the Z80 cpu's data lines to a free 8bit Port on the TM4C123G
- [ ] 3. Copy out the terminal controller and setup code into a seperate project and develop a full VT100 terminal class.

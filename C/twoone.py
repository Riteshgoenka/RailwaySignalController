#!/usr/bin/python

import serial, time
#initialization and open the port

#possible timeout values:
#    1. None: wait forever, block call
#    2. 0: non-blocking mode, return immediately
#    3. x, x is bigger than 0, float allowed, timeout block call

ser1 = serial.Serial()
ser1.port = "/dev/ttyXRUSB0"
ser1.baudrate = 115200
ser1.bytesize = serial.EIGHTBITS #number of bits per bytes
ser1.parity = serial.PARITY_NONE #set parity check: no parity
ser1.stopbits = serial.STOPBITS_ONE #number of stop bits
ser1.timeout = 100000           #non-block read
ser1.xonxoff = False     #disable software flow control
ser1.rtscts = False     #disable hardware (RTS/CTS) flow control
ser1.dsrdtr = False       #disable hardware (DSR/DTR) flow control
ser1.writeTimeout = 100000     #timeout for write

ser2 = serial.Serial()
ser2.port = "/dev/ttyXRUSB1"
ser2.baudrate = 115200
ser2.bytesize = serial.EIGHTBITS #number of bits per bytes
ser2.parity = serial.PARITY_NONE #set parity check: no parity
ser2.stopbits = serial.STOPBITS_ONE #number of stop bits
ser2.timeout = 100000           #non-block read
ser2.xonxoff = False     #disable software flow control
ser2.rtscts = False     #disable hardware (RTS/CTS) flow control
ser2.dsrdtr = False       #disable hardware (DSR/DTR) flow control
ser2.writeTimeout = 100000     #timeout for write

ser1.open()
ser2.open()

while True:
	b2_to_b1 = ser2.read()
	print(b2_to_b1)
	ser1.write(b2_to_b1)
	
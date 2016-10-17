from itertools import chain
import os
import serial
import struct
import sys
import time

import font
#thefont = font.led5x8_font_fixed
thefont = font.led5x8_font_var

port = serial.Serial(sys.argv[1],baudrate=115200)
color=0

def byteify(a):
	flat = []
	for c in a:
		for r in c:
			flat.append(r)
	r1 = []
	for i in flat:
		r1.append((i&0xFF00) >> 8)
		r1.append(i&0x00FF)
	print(list(map(hex,r2)))
	return r2

def send_helper(command, message):
	d2s = []
	i=0
	while (i < len(message)):
		b0 = message[i]
		b1 = message[i+1]
		b2 = message[i+2]
		#print(hex(b0),hex(b1),hex(b2))
		d2s.append(0x40 | ((b0 & 0xfc) >> 2))
		#print(d2s[-1])
		d2s.append(0x40 | ((b0 & 0x03) << 4) | ((b1 & 0xf0) >> 4))
		#print(d2s[-1])
		d2s.append(0x40 | ((b1 & 0x0f) << 2) | ((b2 & 0xc0) >> 6))
		#print(d2s[-1])
		d2s.append(0x40 | ((b2 & 0x3f)))
		#print(d2s[-1])
		i+=3
	waiting = True
	while(waiting):
		rr = port.read(port.inWaiting())
		r = []
		for i in rr:
			r.append(i)
		i=0
		while(i < len(r)):
			if (r[i] == 0x01):
				#print(r)
				waiting = False
			elif (r[i] == 0x02):
				try:
					print("Command: {0}".format(r[i+1]))
				except (IndexError):
					print("Command: (corrupted data)")
				i+=1
			elif (r[i] == 0x04):
				try:
					print("Echo: {0} {1} {2}".format(bin(r[i+1]), bin(r[i+2]), bin(r[i+3])))
				except (IndexError):
					print("Echo: (corrupted data)")
				i+=3
			else:
				try:
					print("Alert: {0} {1}".format(r[i], r[i+1]))
					i+=1
				except (IndexError):
					print("Alert: {0}".format(r[i]))
			i+=1
	#print(command,list(map(bin,message)))
	#print(command,list(map(bin,d2s)))
	port.write(command)
	port.write(d2s)

def clearScreen():
	port.write(b'\x00')

def redraw():
	port.write(b'\x01')

def sendCol(col):
	send_helper(b'\x02',col)

def sendColNR(col):
	send_helper(b'\x03',col)

def sendScreen(s):
	send_helper(b'\x04',s)

def alphabetScroller(t):
	global color
	for ck in sorted(thefont.keys()):
		cv = thefont[ck];
		sendColNR([0,0,0])
		for col in range(len(cv)):
			#b = os.urandom(3)
			c = [0,0,0]
			c[color] = cv[col]
			#print(c)
			sendColNR(c)
		#sendCol([0,0,0])
		redraw()
		color = (color+1)%3
		#if port.inWaiting() > 0:
		#	print("S:",port.read(port.inWaiting()).decode("UTF-8"))
		time.sleep(t)

def fullscreenTest(t):
	for c in range(3):
		for r in range(8):
			onecol = [0x00]*8
			s = [onecol,onecol.copy(),onecol.copy()]
			#print(c,r,s)
			s[c][r] = 0xffff
			#print(c,r,s)
			scr = byteify(s)
			sendScreen(scr)
			#print()
			#if port.inWaiting() > 0:
			#		print("S:",port.read(port.inWaiting()).decode("UTF-8"))
			time.sleep(t)

while(True):
	alphabetScroller(0.5)
	#fullscreenTest(0.05)


port.close()
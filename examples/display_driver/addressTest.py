import sys
data1 = [0b11101111, 0b10100010, 0b11100100, 0b10101111]
data2 = [0b10000001, 0b01000010, 0b00100100, 0b00011000]

def printOut(out):
	for i in out:
		for b in range(7,-1, -1):
			if i&(1<<b):
				sys.stdout.write("X")
			else:
				sys.stdout.write(" ")
		sys.stdout.write('\n')

printOut(data1)
printOut(data2)
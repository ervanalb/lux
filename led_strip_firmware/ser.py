import serial
import time

s=serial.Serial("/dev/ttyUSB0",115200)
s.setRTS(True)

while True:
    raw_input()
    s.setRTS(False)
    s.write("u")
    s.flush()
    time.sleep(0.01)
    s.setRTS(True)

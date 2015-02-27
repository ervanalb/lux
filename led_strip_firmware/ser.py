import serial
import time
import struct
import binascii

s=serial.Serial("/dev/ttyUSB0",3000000)
s.setRTS(True)

def cobs_encode(data):
    output=''
    data+='\0'
    ptr=0
    while ptr < len(data):
        next_zero=data.index('\0',ptr)
        if next_zero-ptr >= 254:
            output+='\xFF'+data[ptr:ptr+254]
            ptr+=254
        else:
            output+=chr(next_zero-ptr+1)+data[ptr:next_zero]
            ptr=next_zero+1
    return output

def cobs_decode(data):
    output=''
    ptr=0
    while ptr < len(data):
        ctr=ord(data[ptr])
        if ptr+ctr > len(data):
            raise ValueError("COBS decoding failed")
        output+=data[ptr+1:ptr+ctr]+'\0'
        ptr+=ctr
    if output[-1]!='\0':
        raise ValueError("COBS decoding failed")
    return output[0:-1]

def frame(destination,data):
    packet=struct.pack('<I',destination)+data
    cs=binascii.crc32(packet) & ((1<<32)-1)
    packet += struct.pack('<I',cs)
    return '\0'+cobs_encode(packet)+'\0'

def unframe(packet):
    cs=binascii.crc32(packet) & ((1<<32)-1)
    if cs != 0x2144DF1C:
        pass
        #raise ValueError("BAD CRC: "+hex(cs))
    return (struct.unpack('<I',packet[0:4])[0],packet[4:-4])

def send(pkt):
    s.write(pkt)
    s.flush()

f1=(chr(255)+chr(0)+chr(0))*50
f2=(chr(0)+chr(255)+chr(0))*50
f3=(chr(0)+chr(0)+chr(255))*50

b1=chr(0b01010101)*15
b2=chr(0b10101010)*15

s.setRTS(False)

n=0
try:
    st=time.time()
#    while True:
    for i in [1]:
        for buf in [f1,f2,f3]:
        #for buf in [b1,b2]:
            f=frame(0xFFFFFFFF,'\0'+buf)
            send(f)
            time.sleep(1)
            n+=1
        t=time.time()-st
        print n/t

finally:
    s.setRTS(True)

"""
print "waiting for rx..."

b=''
while True:
    a=s.read()
    if a=='\0':
        break
    b+=a

decoded=unframe(cobs_decode(b))
#print ','.join([hex(ord(c)) for c in b])
#print ','.join([hex(ord(c)) for c in decoded[0]])
#print ','.join([hex(ord(c)) for c in decoded[1]])

print decoded[1]
"""
#while True:
    #raw_input()
#    s.setRTS(False)
#    s.write("u")
#    s.flush()
#    time.sleep(0.01)
#    s.setRTS(True)
#    print "u"

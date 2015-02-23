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
    packet=destination+data
    cs=binascii.crc32(packet) & ((1<<32)-1)
    print hex(cs)
    packet += struct.pack('<I',cs)
    print ','.join([hex(ord(c)) for c in packet])
    print hex(binascii.crc32(packet) & ((1<<32)-1))
    return cobs_encode(packet)+'\0'

def unframe(packet):
    cs=binascii.crc32(packet) & ((1<<32)-1)
    if cs != 0x2144DF1C:
        raise ValueError("BAD CRC: "+hex(cs))
    return (packet[0:4],packet[4:-4])

def send(pkt):
    print len(pkt)
    s.setRTS(False)
    n=100
    for i in range(0,len(pkt),n):
        chunk=pkt[i:min(i+n,len(pkt))]
        s.write(chunk)
    s.flush()
    time.sleep(0.1)
    s.setRTS(True)

f=frame('\0\0\0\0','\x01\x02\x03\x04\xFF\xFE\xFD\xFC'*(1024/8))
print ','.join([hex(ord(c)) for c in f])

send(f)

#b=''
#while True:
#    a=s.read()
#    if a=='\0':
#        break
#    b+=a

#decoded=unframe(cobs_decode(b))
#print ','.join([hex(ord(c)) for c in b])
#print ','.join([hex(ord(c)) for c in decoded[0]])
#print ','.join([hex(ord(c)) for c in decoded[1]])

#while True:
    #raw_input()
#    s.setRTS(False)
#    s.write("u")
#    s.flush()
#    time.sleep(0.01)
#    s.setRTS(True)
#    print "u"

import serial
import time
import struct
import binascii

class LuxBus:
    BAUD = 3000000

    @staticmethod
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

    @staticmethod
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

    @classmethod
    def frame(cls, destination, data):
        packet=struct.pack('<I',destination)+data
        cs=binascii.crc32(packet) & ((1<<32)-1)
        packet += struct.pack('<I',cs)
        return '\0'+cls.cobs_encode(packet)+'\0'

    @classmethod
    def unframe(cls, packet):
        packet = cls.cobs_decode(packet)
        cs=binascii.crc32(packet) & ((1<<32)-1)
        if cs != 0x2144DF1C:
            raise ValueError("BAD CRC: "+hex(cs))
        return (struct.unpack('<I',packet[0:4])[0],packet[4:-4])

    def __init__(self,serial_port):
        self.s=None
        self.s=serial.Serial(serial_port,self.BAUD)
        self.s.setRTS(False)
        # TODO implement data reception

    def __del__(self):
        if self.s:
            self.s.setRTS(True)

    def send_packet(self,destination,data):
        self.s.write(self.frame(destination, data))
        self.s.flush()

    def ping(self,destination):
        raise NotImplementedError

class LEDStrip:
    def __init__(self,bus,address,length):
        self.bus = bus
        self.address = address
        self.length = length

    def send_frame(self,pixels):
        assert len(pixels) == self.length
        data = '\0' + ''.join([chr(r) + chr(g) + chr(b) for (r,g,b) in pixels])
        self.bus.send_packet(self.address, data)

if __name__ == '__main__':
    l = 194
    tail = 40

    bus = LuxBus('/dev/ttyUSB0')
    strip = LEDStrip(bus,0xFFFFFFFF,l)

    pos = 0
    while True:
        frame = [(0,0,0)] * l
        for i in range(tail+1):
            v = 255 * i / tail
            frame[(pos + i) % l] = (v, v, v)

        strip.send_frame(frame)
        time.sleep(0.01)

        pos = (pos + 1) % l

import serial
import time
import struct
import binascii

class DeviceTypeError(Exception):
    pass

class LuxDecodeError(Exception):
    pass

class LuxBus(object):
    BAUD = 3000000
    RX_PAUSE = 0

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
        #while ptr < len(data):
        while data[ptr] != '\0':
            ctr=ord(data[ptr])
            if ptr+ctr >= len(data):
                raise LuxDecodeError("COBS decoding failed", repr(data))
            output+=data[ptr+1:ptr+ctr]+'\0'
            ptr+=ctr
        if ptr != len(data) - 1:
            raise LuxDecodeError("COBS decoding failed", repr(data))
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
            raise LuxDecodeError("BAD CRC: "+hex(cs), repr(packet))
        return (struct.unpack('<I',packet[0:4])[0],packet[4:-4])

    def __init__(self,serial_port):
        self.s=None
        self.s=serial.Serial(serial_port,self.BAUD)
        self.s.timeout = 1 # 1 second timeout on reads
        self.s.setRTS(True)
        self.rx = ""

    def __del__(self):
        if self.s:
            self.s.setRTS(True)

    def read(self):
        self.rx += self.s.read(size=self.s.inWaiting())
        while '\0' not in self.rx:
            r = self.s.read()
            self.rx += r
            if r == '': # Timeout
                return None
        frame, _null, self.rx = self.rx.partition('\0')
        return self.unframe(frame + '\0')

    @classmethod
    def rx_pause(cls):
        time.sleep(cls.RX_PAUSE)

    def clear_rx(self):
        self.rx = ""
        self.s.flushInput()

    def send_packet(self,destination,data):
        self.s.setRTS(False)
        self.s.write(self.frame(destination, data))
        self.s.flush()
        time.sleep(0.001)
        self.s.setRTS(True)

    def ping(self,destination):
        raise NotImplementedError

    def send_command(self, destination, cmd=None, data=None, raw_data=None):
        frame = chr(cmd) if cmd is not None else ''
        if raw_data is not None:
            frame += raw_data
        elif data is not None:
            frame += ''.join(map(chr, data))
        self.send_packet(destination, frame)

class LuxDevice(object):
    # Main commands
    CMD_FRAME = 0
    CMD_READID = 1
    CMD_LED = 2
    CMD_BUTTON = 3
    CMD_SETADDR = 4
    CMD_BOOTLOADER = 5
    CMD_SETUSERDATA = 6
    CMD_GETUSERDATA = 7

    # Bootloader commands
    BL_CMD_RESET = 0x80
    BL_CMD_READID = 0x81
    BL_CMD_INVALIDATEAPP = 0x82
    BL_CMD_ERASE = 0x83
    BL_CMD_WRITE = 0x84
    BL_CMD_READ = 0x85

    def __init__(self,bus,address):
        self.bus = bus
        self.address = address

    def send_command(self, *args, **kwargs):
        return self.bus.send_command(self.address, *args, **kwargs)

    def read(self, *args, **kwargs):
        self.bus.rx_pause()
        return self.bus.read()

    def get_id(self):
        self.bus.clear_rx()
        self.send_command()
        return self.read()

    def bootloader(self):
        """ Cause device to jump to bootloader"""
        self.send_command(self.CMD_BOOTLOADER)

class LEDStrip(LuxDevice):
    def __init__(self,bus,address,length):
        super(LEDStrip, self).__init__(bus, address)
        self.length = length

    def send_frame(self,pixels):
        assert len(pixels) == self.length
        data = ''.join([chr(r) + chr(g) + chr(b) for (r,g,b) in pixels])
        self.send_command(0, raw_data=data)
        #self.bus.send_packet(self.address, data)

    def set_led(self, state):
        self.bus.send_command(self.address, 2, [1 if state else 0])

    def get_button(self):
        self.bus.send_command(self.address, 3)
        time.sleep(0.3)
        return self.bus.read()[1]



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

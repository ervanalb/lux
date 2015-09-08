import serial
import time
import struct
import binascii

class DeviceTypeError(Exception):
    pass

class LuxDecodeError(Exception):
    pass

class LuxCommandError(Exception):
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
        time.sleep(0.01)
        self.s.setRTS(False)
        raw_data = self.frame(destination, data)
        self.s.write(raw_data)
        self.s.flush()
        time.sleep(0.001)
        self.s.setRTS(True)

    def ping(self,destination):
        raise NotImplementedError

    def send_command(self, destination, cmd=None, data=''):
        frame = chr(cmd) if cmd is not None else ''
        frame += data
        self.send_packet(destination, frame)

class LuxDevice(object):
    # General Lux Commands
    CMD_GET_ID = 0x00
    CMD_RESET = 0x01
    CMD_BOOTLOADER = 0x02

    # Configuration commands
    CMD_WRITE_CONFIG = 0x10
    CMD_WRITE_CONFIG_ACK = 0x11

    CMD_GET_ADDR = 0x12
    CMD_SET_ADDR = 0x13
    CMD_SET_ADDR_ACK = 0x14

    CMD_GET_USERDATA = 0x15
    CMD_SET_USERDATA = 0x16
    CMD_SET_USERDATA_ACK = 0x17

    CMD_GET_PKTCNT = 0x18
    CMD_RESET_PKTCNT = 0x19
    CMD_RESET_PKTCNT_ACK = 0x1a

    def __init__(self,bus,address):
        self.bus = bus
        self.address = address

    def send_command(self, *args, **kwargs):
        return self.bus.send_command(self.address, *args, **kwargs)

    def read(self, *args, **kwargs):
        self.bus.rx_pause()
        return self.bus.read()

    def read_result(self, retry=4):
        for i in range(retry):
            try:
                r = self.read()
            except LuxDecodeError:
                continue
            if r is None:
                continue
            if r[0] is not 0:
                continue
            return r[1]
        raise LuxDecodeError

    def read_ack(self, retry=3, read_retry=4):
        errors = []
        for i in range(retry):
            try:
                r = self.read_result()
            except LuxDecodeError:
                continue
            if r == '\x00':
                return 0
            errors.append(r)
        raise LuxCommandError(errors)

    def command_response(self, *args, **kwargs):
        self.bus.clear_rx()
        self.send_command(*args, **kwargs)
        return self.read_result()

    def command_ack(self, *args, **kwargs):
        self.bus.clear_rx()
        self.send_command(*args, **kwargs)
        return self.read_ack()

    def get_id(self):
        return self.command_response()

    def reset(self):
        self.send_command(self.CMD_RESET)

    def bootloader(self):
        """ Cause device to jump to bootloader"""
        self.send_command(self.CMD_BOOTLOADER)

    def set_address(self, multi_addr, multi_mask, uni_addrs):
        while len(uni_addrs) < 16:
            uni_addrs.append(0xFFFFFFFF)
        raw_data = struct.pack("18I", multi_addr, multi_mask, *uni_addrs)
        self.command_ack(self.CMD_SET_ADDR_ACK, data=raw_data)

    def get_address(self):
        r = self.command_response(self.CMD_GET_ADDR)
        a = struct.unpack("18I", r)
        return a[0], a[1], a[2:]

    def set_userdata(self, data):
        self.command_ack(self.CMD_SET_USERDATA_ACK, data=data[:32])

    def get_userdata(self):
        return self.command_response(self.CMD_GET_USERDATA)

    def write_config(self):
        self.command_ack(self.CMD_WRITE_CONFIG_ACK)

    def get_packet_stats(self):
        r = self.command_response(self.CMD_GET_PKTCNT)
        metrics = struct.unpack("IIIII", r)
        desc = (
                "good",
                "malformed",
                "overrun",
                "checksum",
                "interrupted"
               )
        return dict(zip(desc, metrics))

    def reset_packet_stats(self):
        self.command_ack(self.CMD_RESET_PKTCNT)


class LEDStrip(LuxDevice):
    # Strip-specific configuration
    CMD_SET_LENGTH = 0x20
    CMD_GET_LENGTH = 0x21
    CMD_SET_LENGTH_ACK = 0x22

    # Strip-specific commands
    CMD_FRAME = 0x90
    CMD_FRAME_ACK = 0x91

    CMD_SET_LED = 0x92
    CMD_SET_LED_ACK = 0x93
    CMD_GET_BUTTON = 0x94

    def __init__(self,bus,address):
        super(LEDStrip, self).__init__(bus, address)
        self.type_id = self.get_id()
        if self.type_id not in {"WS2811 LED Strip", "LPD6803 LED Strip"}:
            raise DeviceTypeError
        self.get_length()

    def get_length(self):
        r = self.command_response(self.CMD_GET_LENGTH)
        self.length = struct.unpack("H", r)[0]
        return self.length

    def set_length(self, l):
        self.command_ack(self.CMD_SET_LENGTH_ACK, data=struct.pack("H", l))

    def send_solid_frame(self, pixel):
        self.send_frame([pixel] * self.length)

    def send_frame(self,pixels):
        assert len(pixels) == self.length
        data = ''.join([chr(r) + chr(g) + chr(b) for (r,g,b) in pixels])
        #self.command_ack(self.CMD_FRAME_ACK, data=data)
        self.send_command(self.CMD_FRAME, data=data)

    def set_led(self, state):
        self.command_ack(self.CMD_SET_LED_ACK, '\x01' if state else '\x00')

    def get_button(self):
        return self.command_response(self.CMD_GET_BUTTON)



if __name__ == '__main__':
    l = 194
    tail = 40

    bus = LuxBus('/dev/ttyUSB0')
    strip = LEDStrip(bus,0xFFFFFFFF)
    l = strip.length

    pos = 0
    while True:
        frame = [(0,0,0)] * l
        for i in range(tail+1):
            v = 255 * i / tail
            frame[(pos + i) % l] = (v, v, v)

        #strip.send_frame(frame)
        strip.send_frame([(3,2,1)] * l)
        time.sleep(0.01)

        pos = (pos + 1) % l

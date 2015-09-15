import struct
import binascii
import select
import os
import functools

class TimeoutException(Exception):
    pass

class DeviceTypeError(Exception):
    pass

class LuxDecodeError(Exception):
    pass

class LuxCommandError(Exception):
    pass

class LuxBus(object):
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
        output = ''
        ptr = 0
        while ptr < len(data):
            ctr=ord(data[ptr])
            if ptr + ctr > len(data):
                raise LuxDecodeError("COBS decoding failed", repr(data))
            output += data[ptr + 1:ptr + ctr]
            if ctr < 255:
                output += '\0'
            ptr += ctr
        if ptr != len(data):
            raise LuxDecodeError("COBS decoding failed", repr(data))
        return output[0:-1]

    @classmethod
    def frame(cls, destination, data):
        packet=struct.pack('<I',destination)+data
        cs=binascii.crc32(packet) & ((1<<32)-1)
        packet += struct.pack('<I',cs)
        return cls.cobs_encode(packet)

    @classmethod
    def unframe(cls, packet):
        packet = cls.cobs_decode(packet)
        cs=binascii.crc32(packet) & ((1<<32)-1)
        if cs != 0x2144DF1C:
            raise LuxDecodeError("BAD CRC: "+hex(cs), repr(packet))
        return (struct.unpack('<I',packet[0:4])[0],packet[4:-4])

    def __init__(self, device):
        self.s = None
        self.dev = device
        self.rx = ''
        self.timeout = 0.1

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, type, value, traceback):
        self.close()

    def open(self):
        self.s = os.open(self.dev, os.O_RDWR | os.O_NONBLOCK)

    def close(self):
        os.close(self.s)
        self.s = None

    def lowlevel_write(self, data):
        data += '\0'
        while data:
            (r, w, e) = select.select([], [self.s], [], self.timeout)
            if self.s in w:
                len_written = os.write(self.s, data)
                data = data[len_written:]
            else:
                raise TimeoutException("lux write timed out")

    def lowlevel_read(self):
        while True:
            while '\0' not in self.rx:
                (r, w, e) = select.select([self.s], [], [], self.timeout)
                if self.s in r:
                    self.rx += os.read(self.s, 4096)
                else:
                    raise TimeoutException("lux read timed out")
            while '\0' in self.rx:
                frame, _null, self.rx = self.rx.partition('\0')
                return frame

    def read(self):
        while True:
            try:
                return self.unframe(self.lowlevel_read())
            except LuxDecodeError:
                pass

    def clear_rx(self):
        self.rx = ''
        while os.read(self.s, 4096):
            pass

    def write(self, destination, data):
        self.clear_rx()
        self.lowlevel_write(self.frame(destination, data))

    def command(self, destination, data, retry = 3):
        for r in range(retry):
            try:
                self.write(destination, data)
                rx_destination, rx_data = self.read()
                if rx_destination != 0:
                    raise LuxDecodeError
                return rx_data
            except LuxTimeoutError, LuxDecodeError:
                pass
        raise

    def ping(self, destination, *args, **kwargs):
        return self.command(destination, '', *args, **kwargs)

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

    def __init__(self, bus, address):
        self.bus = bus
        self.address = address

    def send_command(self, *args, **kwargs):
        return self.bus.send_command(self.address, *args, **kwargs)

    def read(self, *args, **kwargs):
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
        if len(pixels) != self.length:
            raise RuntimeError("Expected {0} pixels, got {1}".format(self.length, len(pixels)))
        data = ''.join([chr(r) + chr(g) + chr(b) for (r,g,b) in pixels])
        #self.command_ack(self.CMD_FRAME_ACK, data=data)
        self.send_command(self.CMD_FRAME, data=data)

    def set_led(self, state):
        self.command_ack(self.CMD_SET_LED_ACK, '\x01' if state else '\x00')

    def get_button(self):
        return self.command_response(self.CMD_GET_BUTTON)



if __name__ == '__main__':
    import time
    tail = 10

    with LuxBus('/dev/ttyACM0') as bus:
        print bus.ping(0xFFFFFFFF)

        strip = LEDStrip(bus,0xFFFFFFFF)
        strip.set_length(150)
        l = strip.get_length()

        pos = 0
        while True:
            frame = [(0,0,0)] * l
            for i in range(tail+1):
                v = 255 * i / tail
                frame[(pos + i) % l] = (v, v, v)

            #bus.send_packet(0xffffffff,'')
            #bus.read()
            strip.send_frame(frame)
            #strip.send_frame([(3,2,1)] * l)
            time.sleep(0.5)

            pos = (pos + 1) % l

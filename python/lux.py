import struct
import binascii
import serial

class TimeoutError(Exception):
    pass

class DeviceTypeError(Exception):
    pass

class DecodeError(Exception):
    pass

class CommandError(Exception):
    pass

class Bus(object):
    MULTICAST = 0xFFFFFFFF

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
                raise DecodeError("COBS decoding failed", repr(data))
            output += data[ptr + 1:ptr + ctr]
            if ctr < 255:
                output += '\0'
            ptr += ctr
        if ptr != len(data):
            raise DecodeError("COBS decoding failed", repr(data))
        return output[0:-1]

    @classmethod
    def frame(cls, destination, data):
        packet = struct.pack('<I', destination) + data
        cs = binascii.crc32(packet) & ((1 << 32) - 1)
        packet += struct.pack('<I', cs)
        return cls.cobs_encode(packet)

    @classmethod
    def unframe(cls, packet):
        packet = cls.cobs_decode(packet)
        cs=binascii.crc32(packet) & ((1<<32)-1)
        if cs != 0x2144DF1C:
            raise DecodeError("BAD CRC: "+hex(cs), repr(packet))
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
        self.s = serial.Serial(self.dev, 3000000, timeout = self.timeout, writeTimeout = self.timeout, xonxoff = False)

    def close(self):
        self.s.close()
        self.s = None

    def lowlevel_write(self, data):
        data += '\0'
        try:
            self.s.write(data)
        except serial.SerialTimeoutException as e:
            raise TimeoutError("lux write timed out") # from e

    def lowlevel_read(self):
        while True:
            while '\0' not in self.rx:
                r = self.s.read()
                if len(r) == 0:
                    raise TimeoutError("lux read timed out")
                self.rx += r 
            while '\0' in self.rx:
                frame, _null, self.rx = self.rx.partition('\0')
                return frame

    def read(self):
        while True:
            try:
                return self.unframe(self.lowlevel_read())
            except DecodeError:
                pass

    def clear_rx(self):
        self.rx = ''
        t = self.s.timeout
        try:
            self.s.timeout = 0
            self.s.read()
        finally:
            self.s.timeout = t
   
    def write(self, destination, data):
        self.clear_rx()
        self.lowlevel_write(self.frame(destination, data))

    def command(self, destination, data, retry = 3):
        for r in range(retry):
            try:
                self.write(destination, data)
                rx_destination, rx_data = self.read()
                if rx_destination != 0:
                    raise DecodeError
                return rx_data
            except TimeoutError, DecodeError:
                pass
        raise

    def ping(self, destination, *args, **kwargs):
        return self.command(destination, '', *args, **kwargs)

class Device(object):
    # General Lux Commands
    CMD_GET_ID = chr(0x00)
    CMD_RESET = chr(0x01)
    CMD_BOOTLOADER = chr(0x02)

    # Configuration commands
    CMD_WRITE_CONFIG = chr(0x10)

    CMD_GET_ADDR = chr(0x11)
    CMD_SET_ADDR = chr(0x12)

    CMD_GET_USERDATA = chr(0x13)
    CMD_SET_USERDATA = chr(0x14)

    CMD_GET_PKTCNT = chr(0x15)
    CMD_RESET_PKTCNT = chr(0x16)

    def __init__(self, bus, address):
        self.bus = bus
        self.address = address

    def command(self, *args, **kwargs):
        return self.bus.command(self.address, *args, **kwargs)

    def write(self, *args, **kwargs):
        return self.bus.write(self.address, *args, **kwargs)

    def ack_command(self, *args, **kwargs):
        result = self.command(*args, **kwargs)
        if result != '':
            raise DecodeError

    def get_id(self, *args, **kwargs):
        return self.command(self.CMD_GET_ID, *args, **kwargs)

    def reset(self, *args, **kwargs):
        self.write(self.CMD_RESET, *args, **kwargs)

    def bootloader(self, *args, **kwargs):
        """ Cause device to jump to bootloader """
        self.write(self.CMD_BOOTLOADER, *args, **kwargs)

    def set_address(self, multi_addr, multi_mask, uni_addrs, *args, **kwargs):
        uni_addrs += [Bus.MULTICAST] * (16 - len(uni_addrs))
        self.ack_command(self.CMD_SET_ADDR + struct.pack("18I", multi_addr, multi_mask, *uni_addrs), *args, **kwargs)

    def get_address(self, *args, **kwargs):
        r = self.command(self.CMD_GET_ADDR, *args, **kwargs)
        a = struct.unpack("18I", r)
        return a[0], a[1], a[2:]

    def set_userdata(self, data, *args, **kwargs):
        self.ack_command(self.CMD_SET_USERDATA + data[:32], *args, **kwargs)

    def get_userdata(self, *args, **kwargs):
        return self.command(self.CMD_GET_USERDATA, *args, **kwargs)

    def write_config(self, *args, **kwargs):
        self.ack_command(self.CMD_WRITE_CONFIG, *args, **kwargs)

    def get_packet_stats(self, *args, **kwargs):
        r = self.command(self.CMD_GET_PKTCNT, *args, **kwargs)
        metrics = struct.unpack("IIIII", r)
        desc = (
                "good",
                "malformed",
                "overrun",
                "checksum",
                "interrupted"
               )
        return dict(zip(desc, metrics))

    def reset_packet_stats(self, *args, **kwargs):
        self.ack_command(self.CMD_RESET_PKTCNT, *args, **kwargs)

class LEDStrip(Device):
    # Strip-specific configuration
    CMD_SET_LENGTH = chr(0x20)
    CMD_GET_LENGTH = chr(0x21)

    # Strip-specific commands
    CMD_FRAME = chr(0x90)
    CMD_FRAME_ACK = chr(0x91)

    CMD_SET_LED = chr(0x92)
    CMD_GET_BUTTON = chr(0x93)

    def __init__(self, *args, **kwargs):
        super(LEDStrip, self).__init__(*args, **kwargs)
        self.type_id = self.get_id()
        if self.type_id not in {"WS2811 LED Strip", "LPD6803 LED Strip"}:
            raise DeviceTypeError(self.type_id)
        self.get_length()

    def get_length(self, *args, **kwargs):
        r = self.command(self.CMD_GET_LENGTH, *args, **kwargs)
        self.length = struct.unpack("H", r)[0]
        return self.length

    def set_length(self, length, *args, **kwargs):
        self.ack_command(self.CMD_SET_LENGTH + struct.pack("H", length))

    def send_solid_frame(self, pixel, *args, **kwargs):
        self.send_frame([pixel] * self.length, *args, **kwargs)

    def send_frame(self, pixels, ack = False, *args, **kwargs):
        if len(pixels) != self.length:
            raise RuntimeError("Expected {0} pixels, got {1}".format(self.length, len(pixels)))
        data = ''.join([chr(r) + chr(g) + chr(b) for (r,g,b) in pixels])
        if ack:
            self.command(self.CMD_FRAME_ACK + data)
        else:
            self.write(self.CMD_FRAME + data)

    def set_led(self, state, *args, **kwargs):
        self.ack_command(self.CMD_SET_LED + struct.pack('?', state), *args, **kwargs)

    def get_button(self, *args, **kwargs):
        r = self.command(self.CMD_GET_BUTTON, *args, **kwargs)
        return struct.unpack('?', r)[0]

if __name__ == '__main__':
    import time
    tail = 30

    with Bus('/dev/ttyACM0') as bus:
        print bus.ping(0xFFFFFFFF)
        strip = LEDStrip(bus, 0xFFFFFFFF)
        strip.set_length(150)
        l = strip.get_length()

        pos = 0
        while True:
            frame = [(0,0,0)] * l
            for i in range(tail+1):
                v = 255 * i / tail
                frame[(pos + i) % l] = (v, v, v)

            strip.send_frame(frame)
            time.sleep(0.01)

            pos = (pos + 1) % l

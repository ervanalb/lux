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
        output = b''
        data += b'\0'
        ptr = 0
        while ptr < len(data):
            next_zero = data.index(b'\0', ptr)
            if next_zero - ptr >= 254:
                output += b'\xFF' + data[ptr:ptr+254]
                ptr += 254
            else:
                output += bytearray((next_zero - ptr + 1,)) + data[ptr:next_zero]
                ptr = next_zero + 1
        return output

    @staticmethod
    def cobs_decode(data):
        output = b''
        ptr = 0
        while ptr < len(data):
            ctr = data[ptr]
            if ptr + ctr > len(data):
                raise DecodeError("COBS decoding failed", repr(data))
            output += data[ptr + 1:ptr + ctr]
            if ctr < 255:
                output += b'\0'
            ptr += ctr
        if ptr != len(data):
            raise DecodeError("COBS decoding failed", repr(data))
        return output[0:-1]

    @classmethod
    def frame(cls, destination, command, data, index=0):
        packet = struct.pack('<IBB', destination, command, index) + data
        cs = binascii.crc32(packet) & ((1 << 32) - 1)
        cs_bytes = struct.pack('<I', cs)
        return cls.cobs_encode(packet + cs_bytes), cs_bytes

    @classmethod
    def unframe(cls, packet):
        packet = cls.cobs_decode(packet)
        cs=binascii.crc32(packet) & ((1<<32)-1)
        if cs != 0x2144DF1C:
            raise DecodeError("BAD CRC: "+hex(cs), repr(packet))
        return struct.unpack('<IBB',packet[0:6]) + (packet[6:-4], packet[-4:])

    def __init__(self, device):
        self.s = None
        self.dev = device
        self.rx = b''
        self.timeout = 1.0

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
        data += b'\0'
        try:
            self.s.write(data)
        except serial.SerialTimeoutException as e:
            raise TimeoutError("lux write timed out") # from e

    def lowlevel_read(self):
        while True:
            while b'\0' not in self.rx:
                r = self.s.read()
                if len(r) == 0:
                    print("timeout")
                    raise TimeoutError("lux read timed out")
                self.rx += r 
            while b'\0' in self.rx:
                frame, _null, self.rx = self.rx.partition(b'\0')
                return bytearray(frame)

    def read(self):
        while True:
            try:
                return self.unframe(self.lowlevel_read())
            except DecodeError:
                pass

    def clear_rx(self):
        self.rx = b''
        t = self.s.timeout
        try:
            self.s.timeout = 0
            self.s.read()
        finally:
            self.s.timeout = t
   
    def write(self, destination, command, data, index=0):
        self.clear_rx()
        packet, crc = self.frame(destination, command, data, index=index)
        self.lowlevel_write(packet)
        return crc

    def ping(self, destination):
        self.write(destination, 0, b'', 0)
        return self.read()

class Device(object):
    # General Lux Commands
    CMD_GET_ID = 0x00
    CMD_GET_DESCRIPTOR = 0x01
    CMD_RESET = 0x02
    #CMD_BOOTLOADER = 0x02

    # Configuration commands
    CMD_COMMIT_CONFIG = 0x10

    CMD_GET_ADDR = 0x11
    CMD_SET_ADDR = 0x12

    CMD_GET_PKTCNT = 0x13
    CMD_RESET_PKTCNT = 0x14

    def __init__(self, bus, address):
        self.bus = bus
        self.address = address

    def command(self, command, data, index=0, retry=5, get_crc=False):
        for r in range(retry):
            try:
                crc = self.write(command=command, data=data, index=index)
                rx_destination, rx_command, rx_index, rx_data, rx_crc = self.bus.read()
                if rx_destination != 0:
                    raise DecodeError
                if rx_command != command or rx_index != index:
                    raise DecodeError
                return rx_data if not get_crc else (rx_data, crc)
            except (TimeoutError, DecodeError):
                if r == retry - 1:
                    raise

    def ack_command(self, *args, **kwargs):
        kwargs["get_crc"] = True
        response, crc = self.command(*args, **kwargs)
        if response[1:5] == crc:
            rc = response[0]
            if rc == 0:
                return # Success
            else:
                raise CommandError(rc)
        else:
            raise DecodeError

    def write(self, *args, **kwargs):
        return self.bus.write(self.address, *args, **kwargs)

    def get_id(self, *args, **kwargs):
        return self.command(self.CMD_GET_ID, b'', *args, **kwargs)

    def reset(self, flags=0):
        self.write(self.CMD_RESET, bytearray((flags,)))

    def bootloader(self):
        """ Cause device to jump to bootloader """
        self.reset(flags=0x01)

    def set_address(self, multicast_base = Bus.MULTICAST, multicast_mask = 0, unicast = None, *args, **kwargs):
        if unicast is None:
            unicast = []
        unicast += [Bus.MULTICAST] * (16 - len(unicast))
        self.ack_command(self.CMD_SET_ADDR, struct.pack("18I", multicast_base, multicast_mask, *unicast), *args, **kwargs)

    def get_address(self, *args, **kwargs):
        r = self.command(self.CMD_GET_ADDR, b'', *args, **kwargs)
        a = struct.unpack("18I", r)
        return a[0], a[1], a[2:]

    def write_config(self, *args, **kwargs):
        self.ack_command(self.CMD_COMMIT_CONFIG, b'', *args, **kwargs)

    def get_packet_stats(self, *args, **kwargs):
        r = self.command(self.CMD_GET_PKTCNT, b'', *args, **kwargs)
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
        self.ack_command(self.CMD_RESET_PKTCNT, b'', *args, **kwargs)

class LEDStrip(Device):
    # Strip-specific configuration
    CMD_SET_LENGTH = 0x9C
    CMD_GET_LENGTH = 0x9D

    # Strip-specific commands
    CMD_FRAME = 0x92
    CMD_FRAME_ACK = 0x93

    CMD_SET_LED = 0x96
    CMD_GET_BUTTON = 0x97

    def __init__(self, *args, **kwargs):
        super(LEDStrip, self).__init__(*args, **kwargs)
        self.type_id = str(self.get_id())
        if self.type_id not in {'WS2811 LED Strip', 'LPD6803 LED Strip'}:
            raise DeviceTypeError(self.type_id)
        self.get_length()

    def get_length(self):
        r = self.command(self.CMD_GET_LENGTH, b'')
        self.length = struct.unpack("H", r)[0]
        return self.length

    def set_length(self, length):
        self.ack_command(self.CMD_SET_LENGTH, struct.pack('H', length))

    def send_solid_frame(self, pixel, *args, **kwargs):
        self.send_frame([pixel] * self.length, *args, **kwargs)

    def send_frame(self, pixels, ack = False):
        if len(pixels) != self.length:
            raise RuntimeError("Expected {0} pixels, got {1}".format(self.length, len(pixels)))
        data = bytearray()
        for r,g,b in pixels:
            data += bytearray((int(r), int(g), int(b)))

        if ack:
            self.command(self.CMD_FRAME_ACK, data)
        else:
            self.write(self.CMD_FRAME, data)

    def set_led(self, state):
        self.ack_command(self.CMD_SET_LED, struct.pack('?', state))

    def get_button(self):
        r = self.command(self.CMD_GET_BUTTON)
        return struct.unpack('?', r)[0]

if __name__ == '__main__':
    import time
    tail = 30

    with Bus('/dev/ttyACM0') as bus:
        print(bus.ping(0xFFFFFFFF))
        strip = LEDStrip(bus, 0xFFFFFFFF)
        #strip.set_length(140)
        #strip.set_address(unicast=[2])
        #print(strip.get_address())
        #strip.write_config()
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

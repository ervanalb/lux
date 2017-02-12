from cffi import FFI

__all__ = ("LuxBus", )

ffi = FFI()
ffi.cdef("""
    struct lux_packet {
        uint32_t destination;
        enum lux_command command;
        uint8_t index;
        uint8_t payload[1024];
        uint16_t payload_length;
        uint32_t crc;
    };

    enum lux_flags {
        LUX_ACK   = 1,
        LUX_RETRY = 2,
    };

    int lux_uri_open(const char * uri);
    int lux_serial_open(const char * path);
    int lux_network_open(const char * address_spec, uint16_t port);
    void lux_close(int fd);
    int lux_write(int fd, struct lux_packet * packet, enum lux_flags flags);
    int lux_read(int fd, struct lux_packet * packet, enum lux_flags flags);
    int lux_command(int fd, struct lux_packet * packet, struct lux_packet * response, enum lux_flags flags);
    int lux_sync(int fd, int tries);
    extern int lux_timeout_ms;
""")
lib = ffi.dlopen("liblux.so")
lib.lux_timeout_ms = 1000000

class LuxBus(object):
    CMD_GET_ID = 0x00
    CMD_GET_DESCRIPTOR = 0x01
    CMD_RESET = 0x02
    CMD_COMMIT_CONFIG = 0x10
    CMD_GET_ADDR = 0x11
    CMD_SET_ADDR = 0x12
    CMD_GET_PKTCNT = 0x13
    CMD_RESET_PKTCNT = 0x14
    CMD_INVALIDATEAPP = 0x80
    CMD_FLASH_BASEADDR = 0x81
    CMD_FLASH_ERASE = 0x82
    CMD_FLASH_WRITE = 0x83
    CMD_FLASH_READ = 0x84
    CMD_FRAME_FLIP = 0x90      #TODO
    CMD_FRAME_FLIP_ACK = 0x91  #TODO
    CMD_FRAME = 0x92
    CMD_FRAME_ACK = 0x93
    CMD_FRAME_HOLD = 0x94      #TODO
    CMD_FRAME_HOLD_ACK = 0x95  #TODO
    CMD_SET_LED = 0x96
    CMD_GET_BUTTON_COUNT = 0x97 #TODO (currently "is button pressed?"
    CMD_SET_LENGTH = 0x9C
    CMD_GET_LENGTH = 0x9D

    def __init__(self, uri):
        self.fd = lib.lux_uri_open(uri)
        self.packet = ffi.new("struct lux_packet *")

    def close(self):
        lib.lux_close(self.fd)

    def write(self, packet, flags=0):
        return lib.lux_write(self.fd, self.packet, flags)

    def read(self, flags=0):
        return lib.lux_read(self.fd, self.packet, flags)

    def command(self, packet, flags=0):
        return lib.lux_command(self.fd, self.packet, flags)

if __name__ == "__main__":
    lb = LuxBus("udp://127.0.0.1:1365")
    print lb.fd
    print lb.read()



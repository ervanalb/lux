#define _DEFAULT_SOURCE // for usleep

#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "liblux/linux/lux.h"
#include "log.h"

enum loglevel loglevel = LOGLEVEL_SHOW;

static int send_test_messages(int fd, uint32_t addr, uint32_t len, size_t count, long usec) {
    struct timeval t1, t2;

    // ---- 
    struct lux_packet packet = {
        .destination = addr,
        .command = LUX_CMD_FRAME,
        .index = 0,
        .payload_length = len * 3,
    };

    gettimeofday(&t1, NULL);
    count |= 0xFF;
    for (size_t i = 0; i < count; i++) {
        //packet.destination = i % 10;
        //memset(&packet.payload, i & 0xFF, packet.payload_length);
        for (size_t k = 0; k < len; k++) {
            packet.payload[k * 3 + 0] = i & 0x3F;
            packet.payload[k * 3 + 1] = i & 0x3F;
            packet.payload[k * 3 + 2] = i & 0x3F;
        }
        int rc = lux_write(fd, &packet, 0);
        if (rc < 0) {
            PERROR("Unable to write");
            return -1;
        }
        usleep(usec);
    }
    // ---- 
    gettimeofday(&t2, NULL);

    long delta = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
    double ratio = 0.001 * ((double) delta) / ((double) count);
    SHOW("Time delta: %ld micros for %ld writes", delta, count);
    SHOW("Avg: %0.3lf ms/write; %0.3lf ms delay/write", ratio, 0.001 * (double) usec);

    return 0;
}

static int send_test_commands(int fd, uint32_t addr, int count) {
    struct timeval t1, t2;

    gettimeofday(&t1, NULL);
    // ---- 
    struct lux_packet packet = {
        .destination = addr, 
        .command = LUX_CMD_GET_ID,
        .index = 0,
        .payload_length = 0,
    };

    struct lux_packet response;

    int rc = 0;
    for (int i = 0; i < count; i++)
        rc |= lux_command(fd, &packet, &response, LUX_RETRY);
    // ---- 
    gettimeofday(&t2, NULL);


    long delta = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
    double ratio = 0.001 * ((double) delta) / ((double) count);
    SHOW("Time delta: %ld micros for %d commands", delta, count);
    SHOW("Avg: %0.3lf ms/write", ratio);

    if (rc < 0) {
        PERROR("Error in a command");
        return -1;
    }

    SHOW("Received packet for %#010x; cmd=%#02x; idx=%d; plen=%d; data=",
            response.destination, response.command, response.index, response.payload_length);
    SHOW("'%.*s'", response.payload_length, response.payload);
    return 0;
}

static int reset_packet_count(int fd, int addr) {
    struct lux_packet response;
    struct lux_packet packet2 = {
        .destination = addr, 
        .command = LUX_CMD_RESET_PKTCNT,
        .index = 0,
        .payload_length = 0,
    };
    int rc = lux_command(fd, &packet2, &response, LUX_RETRY | LUX_ACK);
    if (rc < 0) {
        PERROR("Unable to send reset command to %#010x", addr);
        return -1;
    }

    SHOW("Reset packet counts on %#010x", addr);
    return 0;
}

static int read_config(int fd, int addr) {
    struct lux_packet response;
    struct lux_packet packet = {
        .command = LUX_CMD_GET_ID,
        .destination = addr, 
        .index = 0,
        .payload_length = 0,
    };
    int rc = lux_command(fd, &packet, &response, LUX_RETRY);
    if (rc < 0) {
        PERROR("Unable to send GET_ID command to %#010x", addr);
        return -1;
    }
    SHOW("Found device on %#010x (%d): %.*s", addr, addr, response.payload_length, response.payload);

    packet.command = LUX_CMD_GET_LENGTH;
    rc = lux_command(fd, &packet, &response, LUX_RETRY);
    if (rc < 0) {
        PERROR("Unable to send GET_LENGTH command to %#010x", addr);
        return -1;
    }
    uint16_t length;
    if (response.payload_length != sizeof(length)) {
        ERROR("Invalid response to GET_LENGTH, got %u bytes expected %zu",
                response.payload_length, sizeof(length));
        return -1;
    }
    memcpy(&length, response.payload, sizeof(length));
    SHOW("Strip length: %u", length);

    packet.command = LUX_CMD_GET_ADDR;
    rc = lux_command(fd, &packet, &response, LUX_RETRY);
    if (rc < 0) {
        PERROR("Unable to send GET_ADDR command to %#010x", addr);
        return -1;
    }
    const struct lux_addresses * response_addresses = (const void *) response.payload;
    if (response.payload_length < sizeof(*response_addresses)) {
        ERROR("Short response to GET_ADDR: got %u bytes, expected %zu",
                response.payload_length, sizeof(*response_addresses));
        return -1;
    }
    SHOW("Primary address: %#010x; Multicast address, mask: %#010x, %#010x",
         response_addresses->unicasts[0], response_addresses->multicast, response_addresses->multicast_mask);
    for (size_t i = 1; i < LUX_UNICAST_ADDRESS_COUNT; i++) {
        if (response_addresses->unicasts[i] != response_addresses->unicasts[i-1])
            SHOW("    address[%zu]: %#010x", i, response_addresses->unicasts[i]);
    }

    return 0;
}

static int commit_config(int fd, int addr) {
    // This is not needed for some firmware
    struct lux_packet response;
    struct lux_packet packet = {
        .command = LUX_CMD_COMMIT_CONFIG,
        .destination = addr, 
        .index = 0,
        .payload_length = 0,
    };
    int rc = lux_command(fd, &packet, &response, LUX_RETRY | LUX_ACK);
    if (rc < 0) {
        PERROR("Unable to send commit config command to %#010x", addr);
        return -1;
    }

    SHOW("Committed config on %#010x", addr);
    return 0;
}

static int blink_led(int fd, uint32_t addr, int count) {
    struct lux_packet packet = {
        .destination = addr, 
        .command = LUX_CMD_SET_LED,
        .index = 0,
        .payload_length = 1,
    };
    struct lux_packet response;

    while (1) {
        if (count-- <= 0) break;
        packet.payload[0] = 1;
        int rc = lux_command(fd, &packet, &response, LUX_ACK);
        if (rc < 0) PERROR("Unable to turn on  LED for %#010x", addr);
        usleep(100000);

        if (count-- <= 0) break;
        packet.payload[0] = 0;
        rc = lux_command(fd, &packet, &response, LUX_ACK);
        if (rc < 0) PERROR("Unable to turn off LED for %#010x", addr);
        usleep(100000);
    }

    return 0;
}

static int check_packet_count(int fd, int addr) {
    struct lux_packet response;
    struct lux_packet packet2 = {
        .destination = addr, 
        .command = LUX_CMD_GET_PKTCNT,
        .index = 0,
        .payload_length = 0,
    };
    int rc = lux_command(fd, &packet2, &response, LUX_RETRY);
    if (rc < 0) {
        PERROR("Packet check count failed");
        return -1;
    }

    SHOW("Received packet for %#010x; cmd=%#02x; idx=%d; plen=%d;",
            response.destination, response.command, response.index, response.payload_length);
    uint32_t *x = (uint32_t *) response.payload;
    /* Response payload:
    uint32_t good_packet;
    uint32_t malformed_packet;
    uint32_t packet_overrun;
    uint32_t bad_checksum;
    uint32_t rx_interrupted;
    uint32_t wrong_address;
    */
    SHOW("Counters: good=%d malfm=%d ovrun=%d badcrc=%d rxint=%d xaddr=%d", x[0], x[1], x[2], x[3], x[4], x[5]);

    return 0;
}

static int assign_address(int fd, uint32_t old_addr, uint32_t new_addr) {
    struct lux_packet response;
    struct lux_packet command = {
        .destination = old_addr,
        .command = LUX_CMD_GET_ADDR
    };
    int rc = lux_command(fd, &command, &response, LUX_RETRY);
    if (rc < 0) {
        PERROR("Unable to get addresses");
        return -1;
    }

    command.command = LUX_CMD_SET_ADDR;
    command.payload_length = response.payload_length;
    memcpy(command.payload, response.payload, response.payload_length);
    memcpy(&command.payload[8], &new_addr, sizeof new_addr);

    rc = lux_command(fd, &command, &response, LUX_ACK | LUX_RETRY);
    if (rc < 0) {
        PERROR("Unable to set addresses");
        return -1;
    }
    SHOW("Reassigned address %#010x to %#010x", old_addr, new_addr);
    return 0;
}

static int detect_addresses(int fd, uint32_t addr_start, uint32_t addr_end) {
    enum loglevel old_loglevel = loglevel;
    int old_lux_timeout_ms = lux_timeout_ms;
    loglevel = LOGLEVEL_WARN;
    lux_timeout_ms = 5;

    int found_addresses = 0;
    for (uint32_t addr = addr_start; addr <= addr_end; addr++) {
        struct lux_packet response;
        struct lux_packet packet = {
            .destination = addr,
            .command = LUX_CMD_GET_ID,
            .index = 0,
            .payload_length = 0,
        };
        int rc = lux_command(fd, &packet, &response, 0);
        if (rc == 0) {
            SHOW("Found address %#010x (%d): %.*s", addr, addr, response.payload_length, response.payload);
            found_addresses++;
        }
    };
    SHOW("Found %d addresses between %#010x and %#010x (inclusive)", found_addresses, addr_start, addr_end);

    loglevel = old_loglevel;
    lux_timeout_ms = old_lux_timeout_ms;
    return found_addresses;
}

static int set_length(int fd, int addr, uint16_t len) {
    struct lux_packet response;
    struct lux_packet command = {
        .destination = addr,
        .command = LUX_CMD_SET_LENGTH,
        .payload_length = 2,
    };
    memcpy(command.payload, &len, sizeof len);
    int rc = lux_command(fd, &command, &response, LUX_ACK | LUX_RETRY);
    if (rc < 0) {
        PERROR("Unable to set len");
        return -1;
    }
    SHOW("Set length of address %#010x to %hu", addr, len);
    return 0;
}

static int reset(int fd, int addr, uint8_t flags) {
    struct lux_packet command = {
        .destination = addr,
        .command = LUX_CMD_RESET,
        .payload_length = 1,
        .payload = { flags },
    };
    int rc = lux_write(fd, &command, 0);
    if (rc < 0) {
        PERROR("Unable to send reset");
        return -1;
    }
    SHOW("Reset address %#010x (flags %#x)", addr, flags);
    return 0;
}

static int usage() {
    fprintf(stderr, "\n\
  Usage: luxctl <lux_uri> <commands...>\n\
    Commands are executed serially, in order.\n\
    Flags specify commands, and can be used multiple times\n\
    Default address is 0x80000000\n\
 \n\
  Lux URIs:\n\
    serial:///dev/ttyACM0\n\
    udp://127.0.0.1:1365\n\
 \n\
  Commands:\n\
    -h                  This help\n\
    -a <address>        Use address for subsequent commands\n\
    -A <address>        Change the address of the device and\n\
                        use the new address for subsequent commands\n\
    -d <n>              Detect devices with addresses up to n\n\
    -f <n>              Flood n FRAME packets \n\
    -i <n>              Flood n GET_ID commands \n\
    -b <n>              Blink the LED n times\n\
    -s                  Read packet statistics\n\
    -S                  Reset packet statistics\n\
    -L <len>            Set strip length\n\
    -c                  Read config (ID, addresses, length)\n\
    -C                  Commit config\n\
    -r                  Reset device\n\
    -R                  Reset device to bootloader\n\
  \n\
  Examples:\n\
    Blink LED once, set address of 0x80000000 to 0x90, set the length to 100, and blink the LED\n\
    $ luxctl udp://127.0.0.1:1365 -b 1 -A 0x90 -L 100 -b 10\n\
    \n\
    Get packet statistics, blink LED, and then reset statistics for 0x90\n\
    $ luxctl udp://127.0.0.1:1365 -a 0x90 -s -b 3 -S\n\
    \n\
    Stress-test device 0x90 with FRAME packets. Reset stats, send 1k packets, and read stats back\n\
    $ luxctl udp://127.0.0.1:1365 -a 0x90 -b 3 -S -f 1000 -s\n\
");
    return 1;
}

static uint32_t parse_address(const char * arg) {
    if (arg == NULL) {
        return LUX_ADDRESS_NONE;
    } else if (strcasecmp(arg, "none") == 0) {
        return LUX_ADDRESS_NONE;
    } else if (strcasecmp(arg, "reply") == 0) {
        return LUX_ADDRESS_REPLY;
    } else if (strcasecmp(arg, "button") == 0) {
        return LUX_ADDRESS_BUTTON;
    } else if (strcasecmp(arg, "bootloader") == 0) {
        return LUX_ADDRESS_BL;
    } else if (strcasecmp(arg, "bl") == 0) {
        return LUX_ADDRESS_BL;
    } else if (strcasecmp(arg, "new") == 0) {
        return LUX_ADDRESS_NEW;
    } else if (strcasecmp(arg, "all") == 0) {
        return LUX_ADDRESS_ALL;
    } else {
        return strtoul(arg, NULL, 0);
    }
}

int main(int argc, char ** argv) {
    if (argc < 2)
        return usage();
    if (strcmp(argv[1], "-h") == 0)
        return usage();

    int fd = lux_uri_open(argv[1]);
    if (fd < 0) {
        PERROR("Unable to open Lux URI '%s'", argv[1]);
        return 1;
    }

    uint32_t address = 0x80000000;
    uint32_t length = 1;
    optind = 2;
    int opt = -1;
    int rc = 0;
    while ((opt = getopt(argc, argv, "a:A:d:f:i:b:sSL:cCrRh")) != -1) {
        uint32_t optarg_addr = parse_address(optarg);

        switch (opt) {
        case 'a': // use address
            address = optarg_addr;
            SHOW("Using address %#010x", address);
            break;
        case 'A': // set address
            rc = assign_address(fd, address, optarg_addr);
            address = optarg_addr;
            break;
        case 'd': // detect addresses
            rc = detect_addresses(fd, 1, optarg_addr);
            break;
        case 'f': // flood frame packets
            rc = send_test_messages(fd, address, length, optarg_addr, 0);
            break;
        case 'i': // flood id commands
            rc = send_test_commands(fd, address, optarg_addr);
            break;
        case 'b': // blink LED
            rc = blink_led(fd, address, optarg_addr);
            break;
        case 's': // read stats
            rc = check_packet_count(fd, address);
            break;
        case 'S': // clear stats
            rc = reset_packet_count(fd, address);
            break;
        case 'L': // set length
            length = optarg_addr;
            rc = set_length(fd, address, length);
            break;
        case 'c':; // read config
            rc = read_config(fd, address);
            break;
        case 'C':; // commit config
            rc = commit_config(fd, address);
            break;
        case 'r':; // reset
            rc = reset(fd, address, 0x00);
            break;
        case 'R':; // reset to bootloader
            rc = reset(fd, address, 0x01);
            break;
        case 'h':; // usage / help
        default:
            return usage();
        }
        if (rc < 0) {
            ERROR("Command failed; quitting");
            break;
        }
    }

    lux_close(fd);
    return 0;
}


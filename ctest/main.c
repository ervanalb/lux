#include "linux/lux.h"

#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

/*
uint8_t lux_fn_match_destination(uint8_t * dest) {
    return 1;
}

void lux_fn_rx() {
    printf("Recieved packet for %#08X\n; cmd=%#02X; idx=%d; plen=%d\n",
            lux_packet.destination, lux_packet.command, lux_packet.index, lux_packet.payload_length);
    printf("%.*s\n", lux_packet.payload_length, lux_packet.payload);
}

int main(int argc, char ** argv) {
    if (lux_hal_init() != 0) return -1;
    lux_init();

    lux_stop_rx();
    memset(lux_packet.destination, 0xFF, 4);
    lux_packet.command = CMD_GET_ID;
    lux_packet.index = 0;
    lux_packet.payload_length = 0;
    lux_start_tx();

    for (;;) {
        lux_codec();
    }
    return 0;
}
*/

static int send_messages(int fd, size_t count, long usec) {
    struct timeval t1, t2;

    // ---- 
    struct lux_packet packet = {
        .destination = 0xFFFFFFFF,
        .command = CMD_FRAME,
        .index = 0,
        .payload_length = 1020,
    };

    gettimeofday(&t1, NULL);
    for (size_t i = 0; i < count; i++) {
        packet.destination = i % 10;
        memset(&packet.payload, i & 0xFF, packet.payload_length);
        int rc = lux_write(fd, &packet);
        if (rc < 0) {
            printf("Error: %s\n", strerror(errno));
            return -1;
        }
        usleep(usec);
    }
    // ---- 
    gettimeofday(&t2, NULL);

    long delta = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
    double ratio = 0.001 * ((double) delta) / ((double) count);
    printf("Time delta: %ld micros for %ld writes\n", delta, count);
    printf("Avg: %0.3lf ms/write; %0.3lf ms delay/write\n", ratio, 0.001 * (double) usec);

    return 0;
}

static int send_commands(int fd, int count) {
    struct timeval t1, t2;

    gettimeofday(&t1, NULL);
    // ---- 
    struct lux_packet packet = {
        .destination = 0xFFFFFFFF,
        .command = CMD_GET_ID,
        .index = 0,
        .payload_length = 0,
    };

    struct lux_packet response;

    int rc = 0;
    for (int i = 0; i < count; i++)
        rc |= lux_command(fd, &packet, 1, &response);
    // ---- 
    gettimeofday(&t2, NULL);


    long delta = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
    double ratio = 0.001 * ((double) delta) / ((double) count);
    printf("Time delta: %ld micros for %d commands\n", delta, count);
    printf("Avg: %0.3lf ms/write\n", ratio);

    if (rc < 0) {
        printf("Error: %s\n", strerror(errno));
        return -1;
    }

    printf("Recieved packet for %#08x; cmd=%#02x; idx=%d; plen=%d; data=",
            response.destination, response.command, response.index, response.payload_length);
    printf("'%.*s'\n", response.payload_length, response.payload);
    return 0;
}

static int reset_packet_count(int fd) {
    struct lux_packet response;
    struct lux_packet packet2 = {
        .destination = 0xFFFFFFFF,
        .command = CMD_RESET_PKTCNT,
        .index = 0,
        .payload_length = 0,
    };
    int rc = lux_command(fd, &packet2, 3, &response);
    if (rc < 0) {
        printf("Error: %s\n", strerror(errno));
        return -1;
    }

    printf("Reset packet counts\n");
    return 0;
}

static int check_packet_count(int fd) {
    struct lux_packet response;
    struct lux_packet packet2 = {
        .destination = 0xFFFFFFFF,
        .command = CMD_GET_PKTCNT,
        .index = 0,
        .payload_length = 0,
    };
    int rc = lux_command(fd, &packet2, 3, &response);
    if (rc < 0) {
        printf("Error: %s\n", strerror(errno));
        return -1;
    }

    printf("Recieved packet for %#08x; cmd=%#02x; idx=%d; plen=%d; data=",
            response.destination, response.command, response.index, response.payload_length);
    uint32_t *x = (uint32_t *) response.payload;
    /*
    uint32_t good_packet;
    uint32_t malformed_packet;
    uint32_t packet_overrun;
    uint32_t bad_checksum;
    uint32_t rx_interrupted;
    */
    printf("good:%d malfm:%d ovrun:%d badcrc:%d rxint:%d xaddr:%d\n", x[0], x[1], x[2], x[3], x[4], x[5]);
    return 0;
}

int main(void) {
    int fd = lux_network_open("127.0.0.1", 1365);
    //int fd = lux_network_open("192.168.8.1", 1365);
    //int fd = lux_serial_open();
    if (fd < 0) {
        printf("Error, no port: %s\n", strerror(errno));
        return -1;
    }
    /*
    int t = send(fd, "hello\n", 6, 0);
    if (t < 0) {
        printf("Error, cantsend: %s\n", strerror(errno));
        return -1;
    }
    printf("sent!\n");
    */

    /*
    */

    int rc = 0;
    rc = send_commands(fd, 100);
    if (rc < 0) goto fail;

    rc = check_packet_count(fd);
    if (rc < 0) goto fail;

    rc = reset_packet_count(fd);
    if (rc < 0) goto fail;

    rc = send_messages(fd, 1000, 0);
    if (rc < 0) goto fail;

    rc = check_packet_count(fd);
    if (rc < 0) goto fail;

    /*
    struct lux_packet packet3 = {
        .destination = 0xFFFFFFFF,
        .command = CMD_SET_LENGTH,
        .index = 0,
        .payload_length = 2,
        .payload = {140, 0}
    };

    rc = lux_command(fd, &packet3, 1, &response);
    if (rc < 0) {
        printf("Error: %s\n", strerror(errno));
        return -1;
    }
    */
    /*
    packet3.command = CMD_COMMIT_CONFIG;
    packet3.payload_length = 0;
    rc = lux_command(fd, &packet3, 1, &response);
    if (rc < 0) {
        printf("Error: %s\n", strerror(errno));
        return -1;
    }
    */

    lux_close(fd);
    return 0;
fail:
    printf("Failed.\n");
    return 1;
}


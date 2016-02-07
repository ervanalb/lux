#include "crc/crc.h"
#include "lux_hal.h"
#include "lux.h"

#include <errno.h>
#include <fcntl.h> 
#include <linux/serial.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static int serial_set_attribs(int fd)
{
        struct termios tty;

        memset (&tty, 0, sizeof tty);
        
        tcgetattr(fd, &tty);
        cfsetispeed(&tty, 0010015);
        cfsetospeed(&tty, 0010015);

        //ioctl(3, SNDCTL_TMR_TIMEBASE or SNDRV_TIMER_IOCTL_NEXT_DEVICE or TCGETS, {c_iflags=0, c_oflags=0x4, c_cflags=0x1cbd, c_lflags=0, c_line=0, c_cc[VMIN]=1, c_cc[VTIME]=2, c_cc="\x03\x1c\x7f\x15\x04\x02\x01\x00\x11\x13\x1a\x00\x12\x0f\x17\x16\x00\x00\x00"}) = 0

        tty.c_cflag &= ~CSIZE;     // 8-bit chars
        tty.c_cflag |= CS8;     // 8-bit chars
        tty.c_cflag |= (CLOCAL | CREAD);
        tty.c_cflag &= ~CSTOPB; // 1 stop bit
        tty.c_cflag &= ~(PARENB|PARODD);
        tty.c_iflag &= ~(INPCK|ISTRIP);
        tty.c_iflag &= ~(IXON | IXOFF);
        tty.c_iflag |= IXANY;

        if (tcsetattr(fd, TCSANOW, &tty) != 0) return -1;

        return 0;
}

static int serial_open()
{
    /* 
     * Initialize a new serial port (if an additional one exists)
     * Returns a ser_t instance if successful
     * Returns NULL if there are no available ports or there is an error
     */

    int fd;

    char dbuf[32];
    for(int i = 0; i < 9; i++)
    {
        sprintf(dbuf, "/dev/ttyUSB%d", i);
        fd = open(dbuf, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);
        if(fd >= 0)
        {
            printf("Found output on '%s', %d\n", dbuf, fd);
            break;
        }

        sprintf(dbuf, "/dev/ttyACM%d", i);
        fd = open(dbuf, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);
        if(fd >= 0)
        {
            printf("Found output on '%s', %d\n", dbuf, fd);
            break;
        }
    }

    if(fd < 0) {
        printf("Can't find serial port\n");
        return -1;
    }

    if(serial_set_attribs(fd) < 0) return -2;

    return fd;
}

static int output_fd = -1;
static enum {
    OUTPUT_SERIAL,
    OUTPUT_UDP,
} output_type;
uint8_t input_buffer[2048];
ssize_t input_buffer_size = 0;
ssize_t input_buffer_ptr = 0;
uint8_t output_buffer[2048];
ssize_t output_buffer_size = 0;
static crc_t crc;

int lux_hal_init() {
    output_fd = serial_open();
    //output_fd = open("/tmp/test", O_RDWR | O_CREAT, 777);
    if (output_fd < 0) return -1;

    output_type = OUTPUT_SERIAL;
    crc = crc_init();
    return 0;
}

int16_t lux_hal_bytes_to_read() {
    if (output_fd < 0) return -1;

    if (input_buffer_size == 0) {
        input_buffer_ptr = 0;
        switch(output_type) {
        case OUTPUT_SERIAL:
            input_buffer_size = read(output_fd, input_buffer, sizeof(input_buffer));
            break;
        case OUTPUT_UDP:
            input_buffer_size = recv(output_fd, input_buffer, sizeof(input_buffer), MSG_DONTWAIT);
            if (input_buffer_size == EAGAIN || input_buffer_size == EWOULDBLOCK)
                input_buffer_size = 0;
            if (input_buffer_size < 0)
                input_buffer_size = -1;
            break;
        }
    }
    return input_buffer_size;
}

uint8_t lux_hal_read_byte() {
    if (input_buffer_size < 0)
        lux_hal_bytes_to_read();
    if (input_buffer_size < 0)
        return 0; // ERROR!
    input_buffer_size--;
    return input_buffer[input_buffer_ptr++];
}

int16_t lux_hal_bytes_to_write() {
    return sizeof(output_buffer) - output_buffer_size;
}

void lux_hal_write_byte(uint8_t byte) {
    output_buffer[output_buffer_size++] = byte;
    if (sizeof(output_buffer) >= output_buffer_size)
        lux_hal_tx_flush();
}

uint8_t lux_hal_tx_flush() {
    switch(output_type) {
        case OUTPUT_SERIAL: {
            int rc = write(output_fd, output_buffer, output_buffer_size);
            break;
        }
        case OUTPUT_UDP: {
            int rc = send(output_fd, output_buffer, output_buffer_size, 0);
            break;
        }
    }
    output_buffer_size = 0;
    return 1;
}

void lux_hal_reset_crc() {
    crc = crc_init();
}

void lux_hal_crc(uint8_t byte) {
    crc = crc_update(crc, &byte, 1);
}

uint8_t lux_hal_crc_ok() {
    crc = crc_finalize(crc);
    return crc == 0x2144DF1C;
}

void lux_hal_write_crc(uint8_t * ptr) {
    crc = crc_finalize(crc);
    memcpy(ptr, &crc, 4);
}

void lux_hal_enable_rx() {}
void lux_hal_enable_tx() {}
void lux_hal_disable_rx() {}
void lux_hal_disable_tx() {}

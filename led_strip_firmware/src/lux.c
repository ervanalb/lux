#include "lux.h"

#include "lux_hal.h"

// Global variables
uint8_t lux_destination[LUX_DESTINATION_SIZE];
uint8_t lux_serial_buffer[LUX_SERIAL_BUFFER_SIZE];
uint8_t lux_packet[LUX_PACKET_MEMORY_ALLOCATED_SIZE];

int lux_packet_length;

int lux_malformed_packet_counter;
int lux_packet_overrun_counter;
int lux_bad_checksum_counter;
int lux_rx_while_tx_counter;

// Local variables
enum
{
    START,
    SKIP_CURRENT_PACKET,
    READ_DESTINATION,
    READ_PAYLOAD,
    WRITE_DESTINATION,
    WRITE_PAYLOAD,
    WRITE_CHECKSUM,
} codec_state;

int16_t packet_pointer;
int8_t destination_pointer;
uint32_t checksum;

// Local functions

uint8_t cobs_remaining;
uint8_t cobs_add_zero;
uint8_t cobs_last_zero;

static void reset_cobs()
{
    cobs_remaining=0;
    cobs_add_zero=0;
    cobs_last_zero=0;
}

static void do_checksum(uint8_t byte)
{
    checksum+=byte; // TODO something better here
}

static uint8_t checksum_ok()
{
    return !checksum;
}

static uint8_t cobs_decode(uint8_t src, uint8_t* dest)
{
    uint8_t result;
    if(!cobs_remaining)
    {
        cobs_remaining=src-1;
        result=cobs_add_zero;
        if(cobs_add_zero)
        {
            if(cobs_last_zero)
            {
                do_checksum(0);
            }
            cobs_last_zero=1;
            *dest=0;
        }
        cobs_add_zero = (src < 255);
        return result;
    }
    if(cobs_last_zero)
    {
        cobs_last_zero=0;
        do_checksum(0);
    }

    cobs_remaining--;
    do_checksum(src);
    *dest=src;
    return 1;
}

// Global functions

void lux_init()
{
    lux_malformed_packet_counter = 0;
    lux_packet_overrun_counter = 0;
    lux_bad_checksum_counter = 0;
    lux_rx_while_tx_counter = 0;

    codec_state=START;

    lux_hal_enable_rx_dma();
}

void lux_codec()
{
    uint8_t byte_read;
    uint8_t decoded_byte;

    switch(codec_state)
    {
        case START:
        read_destination:
            codec_state=READ_DESTINATION;
            destination_pointer=0;
            reset_cobs();
        case READ_DESTINATION:
            while(lux_hal_bytes_to_read())
            {
                byte_read=lux_hal_read_byte();
                if(!byte_read)
                {
                    lux_malformed_packet_counter++;
                    goto read_destination;
                }
                if(cobs_decode(byte_read,&decoded_byte))
                {
                    lux_destination[destination_pointer++]=decoded_byte;
                }
                if(destination_pointer == LUX_DESTINATION_SIZE)
                {
                    if(lux_fn_match_destination())
                    {
                        if(lux_packet_in_memory)
                        {
                            lux_packet_overrun_counter++;
                            goto skip_current_packet;
                        }
                        else
                        {
                            goto read_payload;
                        }
                    }
                    goto skip_current_packet;
                }
            }
            break;
        read_payload:
        codec_state=READ_PAYLOAD;
        packet_pointer=0;
        case READ_PAYLOAD:
            while(lux_hal_bytes_to_read())
            {
                byte_read=lux_hal_read_byte();
                if(!byte_read)
                {
                    if(checksum_ok())
                    {
                        lux_packet_length=packet_pointer-LUX_CHECKSUM_SIZE-1;
                        lux_packet_in_memory=1;
                        lux_fn_rx();
                        goto read_destination;
                    }
                    lux_bad_checksum_counter++;
                    goto read_destination;
                }
                if(cobs_decode(byte_read,&decoded_byte))
                {
                    if(packet_pointer < LUX_PACKET_MEMORY_ALLOCATED_SIZE)
                    {
                        lux_packet[packet_pointer++]=decoded_byte;
                    }
                    else
                    {
                        lux_malformed_packet_counter++;
                        goto skip_current_packet;
                    }
                }
            }
            break;
        skip_current_packet:
        codec_state=SKIP_CURRENT_PACKET;
        case SKIP_CURRENT_PACKET:
            while(lux_hal_bytes_to_read())
            {
                if(!lux_hal_read_byte()) goto read_destination;
            }
            break;
    }
}



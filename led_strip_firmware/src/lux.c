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

uint8_t lux_packet_in_memory;

// Local variables
enum
{
    CODEC_START,
    READ_DESTINATION,
    READ_PAYLOAD,
    SKIP_CURRENT_PACKET,
    ENCODE,
    ENCODER_STALLED,
} codec_state;

enum
{
    ENCODER_START,
    WRITE_DESTINATION,
    WRITE_PAYLOAD,
    WRITE_CHECKSUM,
    FLUSH,
} encoder_state;

int16_t packet_pointer;
int8_t destination_pointer;

// Local functions

uint8_t cobs_remaining;
uint8_t cobs_add_zero;

static void reset_cobs_decoder()
{
    cobs_remaining=0;
    cobs_add_zero=0;
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
            lux_hal_crc(0);
            *dest=0;
        }
        cobs_add_zero = (src < 255);
        return result;
    }

    cobs_remaining--;
    lux_hal_crc(src);
    *dest=src;
    return 1;
}

static uint8_t cobs_encoder_write_ptr;
static uint8_t cobs_encoder_read_ptr;
static uint8_t cobs_buffer[256];

static void reset_cobs_encoder()
{
    cobs_encoder_write_ptr=1;
}

// Returns: if message completely written
static uint8_t serial_write_continuation()
{
    while(cobs_encoder_read_ptr<cobs_encoder_write_ptr && lux_hal_bytes_to_write())
    {
        lux_hal_write_byte(cobs_buffer[cobs_encoder_read_ptr++]);
    }

    if(cobs_encoder_read_ptr == cobs_encoder_write_ptr)
    {
        reset_cobs_encoder();
        return 1;
    }
    return 0;
}

static uint8_t serial_write()
{
    cobs_encoder_read_ptr=0;
    return serial_write_continuation();
}

static uint8_t cobs_encode_and_send(uint8_t byte)
{
    lux_hal_crc(byte);

    if(byte == 0)
    {
        goto write;
    }
    else
    {
        cobs_buffer[cobs_encoder_write_ptr++]=byte;
    }
    if(cobs_encoder_write_ptr == 256)
    {
        cobs_encoder_write_ptr = 255;
        goto write;
    }
    return 1;

    write:
    cobs_buffer[0]=cobs_encoder_write_ptr;
    return serial_write();
}

static uint8_t cobs_encode_finish()
{
    cobs_buffer[0]=cobs_encoder_write_ptr;
    cobs_buffer[cobs_encoder_write_ptr++]=0;
    return serial_write();
}

// Global functions

void lux_init()
{
    lux_malformed_packet_counter = 0;
    lux_packet_overrun_counter = 0;
    lux_bad_checksum_counter = 0;
    lux_rx_while_tx_counter = 0;

    codec_state=CODEC_START;

    lux_hal_enable_rx_dma();
}

void lux_codec()
{
    uint8_t byte_read;
    uint8_t decoded_byte;

    switch(codec_state)
    {
        case CODEC_START:
        read_destination:
            codec_state=READ_DESTINATION;
            destination_pointer=0;
            lux_hal_reset_crc();
            reset_cobs_decoder();
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
                    if(lux_hal_crc_ok())
                    {
                        lux_packet_length=packet_pointer-LUX_CHECKSUM_SIZE;
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

        encode:
        codec_state=ENCODE;
        case ENCODE:
            switch(encoder_state)
            {
                case ENCODER_START:
                    destination_pointer=0;
                    reset_cobs_encoder();
                    encoder_state=WRITE_DESTINATION;
                case WRITE_DESTINATION:
                    while(destination_pointer<LUX_DESTINATION_SIZE)
                    {
                        if(!cobs_encode_and_send(lux_destination[destination_pointer++])) break;
                    }
                    packet_pointer=0;
                    encoder_state=WRITE_PAYLOAD;
                case WRITE_PAYLOAD:
                    while(packet_pointer<lux_packet_length)
                    {
                        if(!cobs_encode_and_send(lux_packet[packet_pointer++])) break;
                    }
                    lux_hal_write_crc(&lux_packet[packet_pointer]);
                    encoder_state=WRITE_CHECKSUM;
                case WRITE_CHECKSUM:
                    while(packet_pointer<lux_packet_length+LUX_CHECKSUM_SIZE)
                    {
                        if(!cobs_encode_and_send(lux_packet[packet_pointer++])) break;
                    }
                    encoder_state=FLUSH;
                    if(!cobs_encode_finish()) break; // flush COBS
                case FLUSH:
                    if(lux_hal_serial_ready())
                    {
                        lux_hal_enable_rx_dma();
                        goto read_destination;
                    }
            }
            break;

        case ENCODER_STALLED:
            if(serial_write_continuation()) goto encode;
            break;
    }
}

void lux_stop_rx()
{
    lux_hal_disable_rx_dma();
}

void lux_start_tx()
{
    codec_state=ENCODE;
    encoder_state=ENCODER_START;
}


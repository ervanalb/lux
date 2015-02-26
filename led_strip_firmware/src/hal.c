#include "hal.h"
#include "lux_hal.h"
#include "stm32f0xx.h"

#define SERIAL_BUFFER_SIZE 512

static DMA_InitTypeDef tx_DMA_InitStructure;
static uint8_t serial_buffer[SERIAL_BUFFER_SIZE];

// Bring up all hardware
void init()
{
    DMA_InitTypeDef DMA_InitStructure;
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB |
        RCC_AHBPeriph_GPIOF | RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_CRC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_SPI1 | 
        RCC_APB2Periph_SYSCFG, ENABLE);

    // LED
    led_off();
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF,&GPIO_InitStruct);

    // Button
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB,&GPIO_InitStruct);

    // USART
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA,&GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_1);

    GPIO_ResetBits(GPIOA,GPIO_Pin_1); // Serial Receive
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA,&GPIO_InitStruct);

    USART_DeInit(USART1);
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = 3000000;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1,&USART_InitStructure);
    USART_Cmd(USART1,ENABLE);

    // USART DMA
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_BufferSize = SERIAL_BUFFER_SIZE;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)serial_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->RDR);
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
    SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_USART1Rx,ENABLE);

    DMA_DeInit(DMA1_Channel2);
    tx_DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    tx_DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    tx_DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    tx_DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    tx_DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    tx_DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    tx_DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    tx_DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
    tx_DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->TDR);
    // do not init the TX channel, this is done later

    USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

    // CRC
    CRC_DeInit();
    CRC_ReverseOutputDataCmd(ENABLE);
    CRC_ReverseInputDataSelect(CRC_ReverseInputData_8bits);
}

void led_on()
{
    GPIO_SetBits(GPIOF,GPIO_Pin_1);
}

void led_off()
{
    GPIO_ResetBits(GPIOF,GPIO_Pin_1);
}

int16_t getchar()
{
    volatile int i;
    if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
    {
        i=USART_ReceiveData(USART1);
        //return USART_ReceiveData(USART1);
        return i;
    }
    else
    {
        return -1;
    }
}

uint8_t button()
{
    return !GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7);
}

// LUX HAL

// Pointer to serial buffer where data is being read in RX mode (lags DMA counter)
static int16_t serial_read_ptr;

// Poiter to serial buffer where data is being written in TX mode
static int16_t serial_write_ptr;

// 0 = write data to first half of buffer, 1 = write data to second half
static uint8_t fill_second_half;

// Whether or not a TX DMA operation is happening right now
static uint8_t transfer_in_progress;

// Zeros the read pointer, DMA counter, and enables endless DMA for RX
void lux_hal_enable_rx()
{
    serial_read_ptr=0;
    DMA_SetCurrDataCounter(DMA1_Channel5, SERIAL_BUFFER_SIZE);
    DMA_Cmd(DMA1_Channel5, ENABLE);
}

// Disables RX DMA request
void lux_hal_disable_rx()
{
    DMA_Cmd(DMA1_Channel5, DISABLE);
}

// Enable the driver on the RS-485 tranceiver
void lux_hal_enable_tx()
{
    GPIO_SetBits(GPIOA,GPIO_Pin_1); // Serial Transmit
}

// Disable the driver on the RS-485 tranceiver
void lux_hal_disable_tx()
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_1); // Serial Receive
}

// Calculate the number of bytes behind the DMA counter the read pointer currently is.
// Assume no overflows.
int16_t lux_hal_bytes_to_read()
{
    int16_t remaining = SERIAL_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5) - serial_read_ptr;
    if(remaining < 0) remaining += SERIAL_BUFFER_SIZE;
    return remaining;
}

// Return the value at read_pointer and increment it.
uint8_t lux_hal_read_byte()
{
    uint8_t datum = serial_buffer[serial_read_ptr];
    serial_read_ptr++;
    if(serial_read_ptr == SERIAL_BUFFER_SIZE) serial_read_ptr = 0;
    return datum;
}

// Read the TX DMA channel and see if the transfer_in_progress flag needs updating.
static void check_tc()
{
    if(transfer_in_progress && DMA_GetFlagStatus(DMA1_FLAG_TC2))
    {
        DMA_Cmd(DMA1_Channel2, DISABLE);
        transfer_in_progress = 0;
    }
}

// Starts a TX DMA transfer on the current half-buffer.
// Writes data up until serial_write_ptr and flips the fill_second_half flag.
static void begin_tx_dma()
{
    if(fill_second_half)
    {
        tx_DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)serial_buffer+SERIAL_BUFFER_SIZE/2;
    }
    else
    {
        tx_DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)serial_buffer;
    }
    tx_DMA_InitStructure.DMA_BufferSize = serial_write_ptr;
    DMA_Init(DMA1_Channel2, &tx_DMA_InitStructure);
    DMA_ClearFlag(DMA1_FLAG_TC2);
    DMA_Cmd(DMA1_Channel2, ENABLE);

    serial_write_ptr=0;
    transfer_in_progress=1;
    fill_second_half=!fill_second_half;
}

// Return number of characters remaining in this half-buffer.
// Triggers a buffer flip if the current half-buffer is full and the other is free.
int16_t lux_hal_bytes_to_write()
{
    check_tc();

    // If current half-buffer is full, and the other buffer is free,
    // output the current buffer and flip.
    if(serial_write_ptr == SERIAL_BUFFER_SIZE/2 && !transfer_in_progress)
    {
        begin_tx_dma();
    }

    // Calculate characters remaining in this half-buffer.
    int16_t remaining = SERIAL_BUFFER_SIZE/2-serial_write_ptr;
    return remaining;
}

// Writes a byte to the current half-buffer
void lux_hal_write_byte(uint8_t chr)
{
    if(fill_second_half)
    {
        serial_buffer[SERIAL_BUFFER_SIZE/2+serial_write_ptr]=chr;
    }
    else
    {
        serial_buffer[serial_write_ptr]=chr;
    }
    serial_write_ptr++;
}

// Transmit any remaining bytes
// and wait until they are fully sent
uint8_t lux_hal_tx_flush()
{
    // Wait for current transfer to finish
    check_tc();
    if(transfer_in_progress) return 0;

    // Output remaining bytes
    if(serial_write_ptr != 0)
    {
        begin_tx_dma();
        check_tc();
        if(transfer_in_progress) return 0;
    }

    // Wait for USART to be totally done
    if(!USART_GetFlagStatus(USART1,USART_FLAG_TXE) || !USART_GetFlagStatus(USART1,USART_FLAG_TC))
    {
        return 0;
    }
    return 1;
}

// Reset the CRC peripheral
void lux_hal_reset_crc()
{
    CRC_ResetDR();
}

// Update the CRC with a new byte
void lux_hal_crc(uint8_t byte)
{
    *(uint8_t*)(&CRC->DR) = byte;
}

// If the CRC peripheral equals the residue's complement, the CRC checks out.
uint8_t lux_hal_crc_ok()
{
    return CRC->DR == 0xdebb20e3;
}

// Write the CRC peripheral's complement to ptr[0..3]
void lux_hal_write_crc(uint8_t* ptr)
{
    uint32_t crc;
    crc=~CRC->DR;
    for(int i=0;i<LUX_HAL_CRC_SIZE;i++)
    {
        ptr[i]=((uint8_t*)&crc)[i];
    }
}

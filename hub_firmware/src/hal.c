#include "hal.h"
#include "stm32f0xx.h"

#define SERIAL_BUFFER_SIZE 1024

static DMA_InitTypeDef tx_DMA_InitStructure;
static uint8_t serial_buffer[SERIAL_BUFFER_SIZE];
static uint8_t enabled_channels;

#define REMAP_RESET(from, frombit, to, tobit) { \
    (to)->BSRR = ((from) & (frombit)) ? (tobit) << 16 : (tobit); \
}

#define REMAP_SET(from, frombit, to, tobit) { \
    (to)->BSRR = ((from) & (frombit)) ? (tobit) : (tobit) << 16; \
}

static void set_re()
{
    REMAP_RESET(enabled_channels, 0x01, GPIOA, GPIO_Pin_2);
    REMAP_RESET(enabled_channels, 0x02, GPIOA, GPIO_Pin_0);
    REMAP_RESET(enabled_channels, 0x04, GPIOF, GPIO_Pin_0);
    REMAP_RESET(enabled_channels, 0x08, GPIOB, GPIO_Pin_4);
    REMAP_RESET(enabled_channels, 0x10, GPIOB, GPIO_Pin_6);
    REMAP_RESET(enabled_channels, 0x20, GPIOB, GPIO_Pin_3);
}

static void reset_re()
{
    GPIOA->BSRR = GPIO_Pin_2 | GPIO_Pin_0;
    GPIOB->BSRR = GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_3;
    GPIOF->BSRR = GPIO_Pin_0;
}

static void set_de()
{
    REMAP_SET(enabled_channels, 0x01, GPIOA, GPIO_Pin_1);
    REMAP_SET(enabled_channels, 0x02, GPIOF, GPIO_Pin_1);
    REMAP_SET(enabled_channels, 0x04, GPIOB, GPIO_Pin_8);
    REMAP_SET(enabled_channels, 0x08, GPIOB, GPIO_Pin_5);
    REMAP_SET(enabled_channels, 0x10, GPIOB, GPIO_Pin_7);
    REMAP_SET(enabled_channels, 0x20, GPIOA, GPIO_Pin_15);
}

static void reset_de()
{
    GPIOA->BSRR = (GPIO_Pin_1 | GPIO_Pin_15) << 16;
    GPIOB->BSRR = (GPIO_Pin_8 | GPIO_Pin_5 | GPIO_Pin_7) << 16;
    GPIOF->BSRR = (GPIO_Pin_1) << 16;
}

void init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOF | RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // RX LED
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // LEDs, DE, and ~RE
    set_enabled_channels(0);
    reset_re();
    reset_de();
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
                               GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
                               GPIO_Pin_15;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 |
                               GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOF, &GPIO_InitStruct);

    // USART
    USART_DeInit(USART1);
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = 3000000;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1,&USART_InitStructure);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

    // USART DMA
    DMA_DeInit(DMA1_Channel3);
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
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);

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

    // DMA TX interrupt
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
    USART_Cmd(USART1,ENABLE);

    // Turn on RX LED
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
}

// Sets which channels serial comms go in/out over and sets LEDs
void set_enabled_channels(uint8_t channels)
{
    enabled_channels = channels;
    REMAP_SET(enabled_channels, 0x01, GPIOA, GPIO_Pin_3);
    REMAP_SET(enabled_channels, 0x02, GPIOA, GPIO_Pin_4);
    REMAP_SET(enabled_channels, 0x04, GPIOA, GPIO_Pin_5);
    REMAP_SET(enabled_channels, 0x08, GPIOA, GPIO_Pin_6);
    REMAP_SET(enabled_channels, 0x10, GPIOA, GPIO_Pin_7);
    REMAP_SET(enabled_channels, 0x20, GPIOB, GPIO_Pin_0);
}

// Pointer to serial buffer where data is being read in RX mode (lags DMA counter)
static int16_t serial_read_ptr;

// Pointer to serial buffer where data is being written in TX mode
static int16_t serial_write_ptr;

// 0 = write data to first half of buffer, 1 = write data to second half
static uint8_t fill_second_half;

// Whether or not a TX DMA operation is happening right now
static uint8_t tx_in_progress;

// Zeros the read pointer, DMA counter, and enables endless DMA for RX
void enable_rx()
{
    serial_read_ptr=0;
    DMA_SetCurrDataCounter(DMA1_Channel3, SERIAL_BUFFER_SIZE);
    DMA_Cmd(DMA1_Channel3, ENABLE);
    set_re();
}

// Disables RX DMA request
void disable_rx()
{
    reset_re();
    DMA_Cmd(DMA1_Channel3, DISABLE);
}

// Enable the driver on the RS-485 tranceiver
void enable_tx()
{
    GPIO_SetBits(GPIOB,GPIO_Pin_1); // Serial Transmit
    set_de();
}

// Disable the driver on the RS-485 tranceiver
void disable_tx()
{
    GPIO_ResetBits(GPIOB,GPIO_Pin_1); // Serial Receive
    reset_de();
}

// Calculate the number of bytes behind the DMA counter the read pointer currently is.
// Assume no overflows.
int16_t bytes_to_read()
{
    int16_t remaining = SERIAL_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel3) - serial_read_ptr;
    if(remaining < 0) remaining += SERIAL_BUFFER_SIZE;
    return remaining;
}

// Read n bytes from serial buffer into chr
void read_bytes(uint8_t* chr, int n)
{
    for(int i = 0; i < n; i++)
    {
        chr[i] = serial_buffer[serial_read_ptr];
        serial_read_ptr++;
        if(serial_read_ptr == SERIAL_BUFFER_SIZE) serial_read_ptr = 0;
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
    DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel2, ENABLE);

    serial_write_ptr=0;
    fill_second_half=!fill_second_half;
}

// Return number of characters remaining in this half-buffer.
int16_t bytes_to_write()
{
    // Calculate characters remaining in this half-buffer.
    int16_t remaining = SERIAL_BUFFER_SIZE/2-serial_write_ptr;
    return remaining;
}

// Writes a byte to the current half-buffer
void write_bytes(uint8_t* chr, int n)
{
    int offset = fill_second_half ? SERIAL_BUFFER_SIZE/2 : 0;
    offset += serial_write_ptr;

    for(int i = 0; i < n; i++)
    {
        serial_buffer[offset+i] = chr[i];
    }
    serial_write_ptr += n;

    if(!tx_in_progress)
    {
        enable_tx();
        disable_rx();
        tx_in_progress=1;
        begin_tx_dma();
    }
}

// Returns whether or not the USART receiver is idle (no characters received recently)
uint8_t rx_idle()
{
    return USART_GetFlagStatus(USART1,USART_FLAG_IDLE);
}

// Transmit complete
void DMA1_Channel2_3_IRQHandler()
{
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, DISABLE);
    if(serial_write_ptr)
    {
        begin_tx_dma();
    }
    else
    {
        tx_in_progress = 0;
        USART_ClearFlag(USART1, USART_IT_TC);
        USART_ITConfig(USART1, USART_IT_TC, ENABLE);
    }
}

void USART1_IRQHandler()
{
    USART_ITConfig(USART1, USART_IT_TC, DISABLE);
    if(!tx_in_progress && USART_GetFlagStatus(USART1,USART_FLAG_TXE) && USART_GetFlagStatus(USART1,USART_FLAG_TC))
    {
        disable_tx();
        enable_rx();
    }
}


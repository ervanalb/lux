#include "hal.h"
#include "lux_hal.h"
#include "stm32f0xx.h"

static DMA_InitTypeDef DMA_InitStructure;

void init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB |
        RCC_AHBPeriph_GPIOF | RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_CRC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

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

    // UART
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


    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_BufferSize = LUX_SERIAL_BUFFER_SIZE;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)lux_serial_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->RDR);
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);

    DMA_DeInit(DMA1_Channel2);
    DMA_InitStructure.DMA_BufferSize = LUX_SERIAL_BUFFER_SIZE/2;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)lux_serial_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->TDR);
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);

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

static int16_t serial_read_ptr;
static int16_t serial_write_ptr;
static uint8_t fill_second_half;
static uint8_t transfer_in_progress;

void lux_hal_enable_rx_dma()
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_1); // Serial Receive
    serial_read_ptr=0;
    DMA_SetCurrDataCounter(DMA1_Channel3, LUX_SERIAL_BUFFER_SIZE);
    DMA_Cmd(DMA1_Channel3, ENABLE);
}

void lux_hal_disable_rx_dma()
{
    DMA_Cmd(DMA1_Channel3, DISABLE);
    GPIO_SetBits(GPIOA,GPIO_Pin_1); // Serial Transmit
}

int16_t lux_hal_bytes_to_read()
{
    int16_t remaining = LUX_SERIAL_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel3) - serial_read_ptr;
    if(remaining < 0) remaining += LUX_SERIAL_BUFFER_SIZE;
    return remaining;
}

uint8_t lux_hal_read_byte()
{
    uint8_t datum = lux_serial_buffer[serial_read_ptr];
    serial_read_ptr++;
    if(serial_read_ptr == LUX_SERIAL_BUFFER_SIZE) serial_read_ptr = 0;
    return datum;
}

void lux_hal_crc(uint8_t byte)
{
    *(uint8_t*)(&CRC->DR) = byte;
}

uint8_t lux_hal_crc_ok()
{
    return CRC->DR == 0xdebb20e3;
}

void lux_hal_reset_crc()
{
    CRC_ResetDR();
}

void lux_hal_write_crc(uint8_t* ptr)
{
    uint32_t crc;
    crc=~CRC->DR;
    for(int i=0;i<4;i++)
    {
        ptr[i]=((uint8_t*)&crc)[i];
    }
}

static void check_tc()
{
    if(transfer_in_progress && DMA_GetFlagStatus(DMA1_FLAG_TC2))
    {
        DMA_Cmd(DMA1_Channel2, DISABLE);
        transfer_in_progress = 0;
    }
}

static void begin_tx_dma()
{
    if(fill_second_half)
    {
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)lux_serial_buffer+LUX_SERIAL_BUFFER_SIZE/2;
    }
    else
    {
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)lux_serial_buffer;
    }
    DMA_InitStructure.DMA_BufferSize = serial_write_ptr;
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);
    DMA_ClearFlag(DMA1_FLAG_TC2);
    DMA_Cmd(DMA1_Channel2, ENABLE);

    serial_write_ptr=0;
    transfer_in_progress=1;
    fill_second_half=!fill_second_half;
}

uint8_t lux_hal_serial_ready()
{
    if(transfer_in_progress)
    {
        check_tc();
        if(transfer_in_progress) return 0;
    }
    if(serial_write_ptr != 0)
    {
        begin_tx_dma();
        check_tc();
        if(transfer_in_progress) return 0;
    }
    if(!USART_GetFlagStatus(USART1,USART_FLAG_TXE) || !USART_GetFlagStatus(USART1,USART_FLAG_TC))
    {
        return 0;
    }
    return 1;
}

static void check_tx_dma()
{
    check_tc();

    if(serial_write_ptr == LUX_SERIAL_BUFFER_SIZE/2 && !transfer_in_progress)
    {
        begin_tx_dma();
    }
}

int16_t lux_hal_bytes_to_write()
{
    check_tx_dma();
    int16_t remaining = LUX_SERIAL_BUFFER_SIZE/2-serial_write_ptr;
    if(!transfer_in_progress) remaining += LUX_SERIAL_BUFFER_SIZE/2;
    return remaining;
}

void lux_hal_write_byte(uint8_t chr)
{
    if(fill_second_half)
    {
        lux_serial_buffer[LUX_SERIAL_BUFFER_SIZE/2+serial_write_ptr]=chr;
    }
    else
    {
        lux_serial_buffer[serial_write_ptr]=chr;
    }
    serial_write_ptr++;
}

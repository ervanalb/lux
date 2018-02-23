#ifdef LPD6803

#include "strip.h"
#include "stm32f0xx.h"
#include "lux_device.h"

#define STRIP_LENGTH (lux_device_config.strip_length)
#define STRIP_MEMORY_LENGTH (3+STRIP_LENGTH+STRIP_LENGTH/16)
#define MAX_STRIP_MEMORY_LENGTH (3+MAX_STRIP_LENGTH+MAX_STRIP_LENGTH/16)

static uint16_t strip_memory[MAX_STRIP_MEMORY_LENGTH];

void strip_init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    SPI_InitTypeDef SPI_InitStruct;
    DMA_InitTypeDef DMA_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    // SPI Out to Strip
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB,&GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource3,GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource5,GPIO_AF_0);

    SPI_I2S_DeInit(SPI1);
    SPI_InitStruct.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1,&SPI_InitStruct);
    SPI_Cmd(SPI1, ENABLE);

    // Strip Memory
    strip_memory[0]=0x0000;
    strip_memory[1]=0x0000;
    for(int i=0;i<MAX_STRIP_LENGTH;i++)
    {
        strip_memory[2+i]=0x8000;
    }
    for(int i=0;i<=MAX_STRIP_LENGTH/16;i++)
    {
        strip_memory[2+MAX_STRIP_LENGTH+i]=0xFFFF;
    }

    // DMA - SPI
    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI1->DR));
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)strip_memory;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = MAX_STRIP_MEMORY_LENGTH;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    DMA_Cmd(DMA1_Channel3, ENABLE);
}

void strip_write(uint8_t* rgb_data, uint16_t begin_pos, uint16_t end_pos)
{
    uint16_t* strip = &strip_memory[2];
    for(uint16_t i = begin_pos; i < end_pos; i++)
    {
        if (i >= MAX_STRIP_LENGTH)
            break;

        uint8_t r = (*rgb_data++) >> 3;
        uint8_t g = (*rgb_data++) >> 3;
        uint8_t b = (*rgb_data++) >> 3;

        *strip++ = 0x8000 | (b << 10) | (r << 5) | g;
    }
}

void strip_flush(void) {
    uint16_t* strip = &strip_memory[2];
    for(int i=0;i<=STRIP_LENGTH/16;i++)
    {
        strip_memory[2+STRIP_LENGTH+i]=0xFFFF;
    }

    DMA_Cmd(DMA1_Channel3, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_TC3);
    DMA_SetCurrDataCounter(DMA1_Channel3,STRIP_MEMORY_LENGTH);
    DMA_Cmd(DMA1_Channel3, ENABLE);
}

uint8_t strip_ready()
{
    return DMA_GetFlagStatus(DMA1_FLAG_TC3);
}

#endif

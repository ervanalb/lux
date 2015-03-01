#include "stm32f0xx.h"

#define PULSE_PERIOD 200

#define PULSE_BUFFER_LENGTH 64

uint8_t pulse_buffer[PULSE_BUFFER_LENGTH];

void main()
{
    DMA_InitTypeDef DMA_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // TIM3
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = PULSE_PERIOD;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

    TIM_Cmd(TIM3, ENABLE);

    // DMA - TIM3_UP
    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(TIM3->CCR2));
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pulse_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = PULSE_BUFFER_LENGTH;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);
    TIM_DMACmd(TIM3, TIM_DMA_Update, ENABLE);

    // NVIC
    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel2_3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    DMA_ITConfig(DMA1_Channel3, DMA_IT_TC | DMA_IT_HT, ENABLE);

    DMA_ClearFlag(DMA1_FLAG_TC3);
    DMA_ClearFlag(DMA1_FLAG_HT3);
    DMA_SetCurrDataCounter(DMA1_Channel3,PULSE_BUFFER_LENGTH);
    DMA_Cmd(DMA1_Channel3, ENABLE);

    for(;;);
}

void DMA1_Channel2_3_IRQHandler(void)
{
    for(;;);
}



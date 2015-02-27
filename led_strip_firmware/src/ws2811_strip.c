#ifdef WS2811

#include "strip.h"
#include "stm32f0xx.h"

#define PULSE_PERIOD 61
#define PULSE_WIDTH_0 18
#define PULSE_WIDTH_1 35

#define STRIP_MEMORY_LENGTH (STRIP_LENGTH*3)

#define PULSE_BUFFER_LENGTH 64

static uint8_t strip_memory[STRIP_MEMORY_LENGTH];
static uint8_t pulse_buffer[PULSE_BUFFER_LENGTH];

void strip_init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    DMA_InitTypeDef DMA_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // Pulses out to strip
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB,&GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource5,GPIO_AF_1);

    // TIM3 CH2
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = PULSE_PERIOD;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStruct.TIM_Pulse = 0;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStruct.TIM_OCNPolarity = TIM_OCNPolarity_Low;
    TIM_OCInitStruct.TIM_OCNIdleState = TIM_OCNIdleState_Reset;

    TIM_OC2Init(TIM3, &TIM_OCInitStruct);
    TIM_CtrlPWMOutputs(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);

    // DMA - SPI
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
}

static uint8_t* data_pointer;
static uint16_t bit_counter;
//static uint16_t byte_counter;

void strip_write(uint8_t* rgb_data)
{
    int i;

    for(i=0;i<STRIP_MEMORY_LENGTH;i++)
    {
        strip_memory[i]=rgb_data[i];
    }
    data_pointer=strip_memory;
    bit_counter=STRIP_MEMORY_LENGTH*8;

    //byte_counter=STRIP_MEMORY_LENGTH;
    //TIM3->CNT = 0;
    //TIM3->SR = ~TIM_FLAG_Update;
    //TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    for(int i=0;i<PULSE_BUFFER_LENGTH;i+=2)
    {
        pulse_buffer[i]=PULSE_WIDTH_0;
        pulse_buffer[i+1]=PULSE_WIDTH_1;
    }

    DMA_ClearFlag(DMA1_FLAG_TC3);
    DMA_ClearFlag(DMA1_FLAG_HT3);
    DMA_SetCurrDataCounter(DMA1_Channel3,PULSE_BUFFER_LENGTH);
    DMA_Cmd(DMA1_Channel3, ENABLE);
}

uint8_t strip_ready()
{
    return 1;
}

/*
void TIM3_IRQHandler(void)
{
    if(!bit_counter)
    {
        TIM3->CCR2 = 0;
        TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
        return;
    }

    bit_counter--;

    TIM3->CCR2 = PULSE_WIDTH_0;
    if(data_pointer[bit_counter>>3] & 0x80)
    {
        TIM3->CCR2 = PULSE_WIDTH_1;
    }
    *data_pointer <<= 1;

    TIM3->SR = ~TIM_FLAG_Update;
}
*/
/*
void TIM3_IRQHandler(void)
{
    if(!--bit_counter)
    {
        if(!--byte_counter)
        {
            TIM3->CCR2 = 0;
            TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
            return;
        }
        data_pointer++;
        bit_counter=8;
    }

    TIM3->CCR2 = PULSE_WIDTH_0;
    if(*data_pointer & 1)
    {
        TIM3->CCR2 = PULSE_WIDTH_1;
    }
    *data_pointer >>= 1;

    TIM3->SR = ~TIM_FLAG_Update;
}
*/

void DMA1_Channel2_3_IRQHandler(void)
{
    DMA_Cmd(DMA1_Channel3, DISABLE);
    if(DMA1->ISR & DMA1_IT_HT3)
    {
        DMA1->IFCR = DMA1_IT_HT3;
    }

    if(DMA1->ISR & DMA1_IT_TC3)
    {
        
        DMA1->IFCR = DMA1_IT_TC3;
    }
}

#endif

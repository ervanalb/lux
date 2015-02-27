#ifdef WS2811

#include "strip.h"
#include "stm32f0xx.h"

//#define PULSE_PERIOD 610
//#define PULSE_PERIOD 122
//#define PULSE_WIDTH_0 18
//#define PULSE_WIDTH_1 35

#define ONE_HIGH_T 18
#define ONE_LOW_T 35
#define ZERO_HIGH_T 35
#define ZERO_LOW_T 18

#define STRIP_MEMORY_LENGTH (STRIP_LENGTH*3)

static uint8_t strip_memory[STRIP_MEMORY_LENGTH];

void strip_init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // Pulses out to strip
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB,&GPIO_InitStruct);
    //GPIO_PinAFConfig(GPIOB,GPIO_PinSource5,GPIO_AF_1);

/*
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
*/

    // NVIC
    /*
    NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    */
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

    for(;;)
    {
        if(!bit_counter) break;

        bit_counter--;

        if(data_pointer[bit_counter>>3] & 0x80)
        {
            GPIOB->BSRR = GPIO_Pin_5;
            // 1 HIGH
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            GPIOB->BSRR = (GPIO_Pin_5 << 16);
            // 1 LOW
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
         }
        else
        {
            GPIOB->BSRR = GPIO_Pin_5;
            // 0 HIGH
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            GPIOB->BSRR = (GPIO_Pin_5 << 16);
            // 0 LOW
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
         }
        data_pointer[bit_counter>>3] <<= 1;
    }
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
#endif

#include "hal.h"
#include "stm32f0xx.h"

void init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOF,ENABLE);
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

    serial_receive();
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA,&GPIO_InitStruct);

    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1,&USART_InitStructure);
    USART_Cmd(USART1,ENABLE);
}

void led_on()
{
    GPIO_SetBits(GPIOF,GPIO_Pin_1);
}

void led_off()
{
    GPIO_ResetBits(GPIOF,GPIO_Pin_1);
}

void serial_receive()
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_1);
}

void serial_transmit()
{
    GPIO_SetBits(GPIOA,GPIO_Pin_1);
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

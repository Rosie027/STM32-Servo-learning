#include "stm32f10x.h"                  // Device header

//选择的接口：PB10 	I/O 	FT 	PB10 	I2C2_SCL/USART3_TX	TIM2_CH3

void PWM_Init(void)
{
	//开启定时器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);//开启时钟全在APB1上，开2-4就可以

	//开启 GPIOB 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//这次选用PB10的重定义吧，之前没试过
	
	//开启AFIO并重映射
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2,ENABLE);

    GPIO_InitTypeDef GPIO_Str;
    GPIO_Str.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Str.GPIO_Pin = GPIO_Pin_10;
    GPIO_Str.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_Str);

	//定时器
    TIM_InternalClockConfig(TIM2);
    TIM_TimeBaseInitTypeDef TIM_Str;
    TIM_Str.TIM_ClockDivision = TIM_CKD_DIV1;   // 时钟分频（滤波用），通常DIV1，可改
    TIM_Str.TIM_CounterMode = TIM_CounterMode_Up; // 计数模式（向上/中央对齐），可改
    TIM_Str.TIM_Period = 20000 - 1;  // ARR，决定周期，必须按需求计算
    TIM_Str.TIM_Prescaler = 72 - 1;  // PSC，决定计数频率，必须按需求计算
    TIM_Str.TIM_RepetitionCounter = 0; // 高级定时器专用，通用定时器无效，写0
    TIM_TimeBaseInit(TIM2, &TIM_Str);

    TIM_OCInitTypeDef TIM_OCStr;
    TIM_OCStructInit(&TIM_OCStr);
    //TIM_OCStr.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCStr.TIM_OCMode = TIM_OCMode_PWM2;//PWM1 是“CNT < CCR 时有效”，PWM2 是“CNT > CCR 时有效”。
	//TIM_OCStr.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCStr.TIM_OCPolarity = TIM_OCPolarity_High; // 高极性（有效电平为高），可改
    TIM_OCStr.TIM_OutputState = TIM_OutputState_Enable; // 输出使能
    TIM_OCStr.TIM_Pulse = 0; // CCR初始值，运行时通过函数修改
    TIM_OC3Init(TIM2, &TIM_OCStr); // 初始化通道3，对应PB10（TIM2_CH3）

    TIM_Cmd(TIM2, ENABLE);
}

//设置占空比函数
void PWM_SetCompare3(uint16_t Compare)
{
    TIM_SetCompare3(TIM2, Compare);
}

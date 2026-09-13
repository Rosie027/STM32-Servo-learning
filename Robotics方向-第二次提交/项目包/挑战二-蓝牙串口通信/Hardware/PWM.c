#include "stm32f10x.h"                  // Device header

void PWM_Init(void)
{ 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	//初始化GPIO
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	//结束GPIO接口的初始化
	
	TIM_InternalClockConfig(TIM2);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period=2000-1;  //ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler=720-1; //PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCPolarity_High;
	//high是高极性
	TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
	//输出状态
	TIM_OCInitStructure.TIM_Pulse=0;//完成呼吸灯效果，使用函数进行这个修改
	//CCR的值 16位的分为
	TIM_OC1Init(TIM2,&TIM_OCInitStructure);//通道初始化好，需要一个GPIO口，不能任意选择，TIM2——CH1只能在PA0上输出
	//在面临冲突的情况下，可能有重映射的情况，使用AFIO完成的
	//使用重印射可以使用PA15
	
	//PWM的频率 占空比 分辨率的
	
	
	TIM_Cmd(TIM2,ENABLE);
}

void PWM_SetCompare1(uint16_t Compare)
{
	TIM_SetCompare1(TIM2,Compare);
}


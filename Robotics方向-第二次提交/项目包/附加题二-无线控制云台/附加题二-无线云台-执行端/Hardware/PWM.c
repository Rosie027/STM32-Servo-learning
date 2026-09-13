// 附加题二 执行端：双通道 PWM（舵机驱动）
//
// 资源占用：TIM2_CH1=PA0, TIM2_CH2=PA1（不复用，无冲突）
// 时序：72MHz/(PSC+1)/(ARR+1) = 72MHz/72/20000 = 50Hz（20ms 周期）
// 舵机控制：高电平 0.5~2.5ms 对应 0~180°
//   → CCR = 500~2500，与控制端旧版公式 angle/180*2000+500 兼容

#include "stm32f10x.h"

void PWM_Init(void)
{
    // 1. 时钟：TIM2 在 APB1, GPIOA 在 APB2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 2. GPIO 初始化：PA0 + PA1 复用推挽输出
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. TIM2 时基：内部时钟 72MHz，向上计数，50Hz
    TIM_InternalClockConfig(TIM2);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_Prescaler = 72 - 1;        // PSC = 71
    TIM_TimeBaseStruct.TIM_Period = 20000 - 1;        // ARR = 19999
    TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);

    // 4. 输出比较：PWM1 模式，高电平有效，初始 CCR=0（无输出）
    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_OCStructInit(&TIM_OCInitStruct);
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0;

    TIM_OC1Init(TIM2, &TIM_OCInitStruct);   // PA0 = CH1
    TIM_OC2Init(TIM2, &TIM_OCInitStruct);   // PA1 = CH2

    // 5. 启动定时器
    TIM_Cmd(TIM2, ENABLE);
}

void PWM_SetCompare1(uint16_t Compare)
{
    TIM_SetCompare1(TIM2, Compare);
}

void PWM_SetCompare2(uint16_t Compare)
{
    TIM_SetCompare2(TIM2, Compare);
}

#include "stm32f10x.h"                  // Device header

void PWM_Init(void)
{
    // 问题：为什么是APB1？为什么是TIM2？其他可以吗？
    // 答：TIM2-TIM7挂在APB1总线上；TIM2是通用定时器，有4个PWM通道。可以换TIM3/4/5，但要改时钟和引脚。
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 问题：初始化GPIO，都是使用APB2吗？后面可以写GPIOA也可以写GPIOB是吗？
    // 答：所有GPIO时钟都在APB2上。可以写GPIOB，但必须保证该引脚是当前定时器通道的复用功能引脚（查手册）。
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    // 问题：这个为什么？这和点亮LED是一样的吧？
    // 答：不一样。LED用普通推挽(Out_PP)，CPU控制；PWM必须用复用推挽(AF_PP)，定时器硬件自动控制。
    // 归类：LED、蜂鸣器→Out_PP；PWM、SPI→AF_PP；按键→输入；模拟传感器→AIN；I2C→AF_OD。
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

    // 问题：对于选择的口子有什么要求吗？比如不能占用什么口？必须GPIOB之类的？
    // 答：必须选该定时器通道的专用复用引脚（数据手册）。例如TIM2_CH2默认是PA1，重映射后可PB3。
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;

    // 问题：这一参数不变化了吗？
    // 答：可变化，可选2/10/50MHz。原则：高频PWM用50MHz，低速信号用2MHz降低干扰。
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 问题：必须是TIM2吗？下面的参数分别什么意思，可以变化吗？
    // 答：不必须，可用其他定时器。参数含义和可变性见下方注释。
    TIM_InternalClockConfig(TIM2);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;   // 时钟分频（滤波用），通常DIV1，可改
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 计数模式（向上/中央对齐），可改
    TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1;  // ARR，决定周期，必须按需求计算
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;  // PSC，决定计数频率，必须按需求计算
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0; // 高级定时器专用，通用定时器无效，写0
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High; // 高极性（有效电平为高），可改
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 输出使能
    TIM_OCInitStructure.TIM_Pulse = 0; // CCR初始值，运行时通过函数修改
    TIM_OC2Init(TIM2, &TIM_OCInitStructure); // 初始化通道2，对应PA1（TIM2_CH2）

    // 问题：什么作用？没有会怎么样？什么时候不需要？
    // 答：启动定时器计数器。没有则CNT永远为0，无PWM波形（固定电平）。需要暂停输出时可设为DISABLE。
    TIM_Cmd(TIM2, ENABLE);
}

// 问题：这是最常用的函数吗？还有什么很重要的PWM相关的函数吗？这一函数什么用？Compare1-4的区别是什么？
// 答：是，运行时改占空比最常用。其他重要：TIM_SetAutoreload()改频率，TIM_Cmd()启停。
//    该函数修改CCR2的值（占空比）。Compare1-4对应四个独立通道，各控制不同引脚，输出不同占空比（共用频率）。
void PWM_SetCompare2(uint16_t Compare)
{
    TIM_SetCompare2(TIM2, Compare);
}

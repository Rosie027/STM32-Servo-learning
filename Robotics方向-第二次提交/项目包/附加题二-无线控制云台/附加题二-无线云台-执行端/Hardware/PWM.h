// 附加题二 执行端：双通道 PWM（舵机驱动）
//
// 硬件资源：
//   TIM2_CH1 = PA0  → 下层舵机（Roll + Yaw 补偿）
//   TIM2_CH2 = PA1  → 上层舵机（Pitch）
//   ARR=20000, PSC=72 → 1MHz 计数 / 20ms 周期 = 50Hz 舵机标准频率
//   CCR 范围 500~2500 → 0.5~2.5ms 高电平 → 0~180° 舵机角度

#ifndef __PWM_H
#define __PWM_H

#include "stm32f10x.h"

void PWM_Init(void);
void PWM_SetCompare1(uint16_t Compare);
void PWM_SetCompare2(uint16_t Compare);

#endif

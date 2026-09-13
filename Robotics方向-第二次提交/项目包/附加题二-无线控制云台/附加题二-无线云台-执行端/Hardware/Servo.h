// 附加题二 执行端：双路舵机控制
//
// channel 0 → 下层舵机（Roll + Yaw 补偿）→ PA0 = TIM2_CH1
// channel 1 → 上层舵机（Pitch）         → PA1 = TIM2_CH2

#ifndef __SERVO_H
#define __SERVO_H

#include <stdint.h>

void Servo_Init(void);

// channel: 0 或 1
// angle:   0~180°
void Servo_SetAngle(uint8_t channel, float Angle);

#endif

#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_Init(void)
{
	PWM_Init();
}

void Servo_SetAngle(float Angle)
{
	PWM_SetCompare1(Angle/180*2000+500);
}

// 定义一个静态变量，记录上一次输出的角度（放在 Servo.c 顶部）
//static float last_output_angle = -999.0f; 

//void Servo_SetAngle(float Angle)
//{
//    if (Angle < 0) Angle = 0;
//    if (Angle > 180) Angle = 180;
//    
//    // ★★★ 防抖动死区：如果角度变化小于 0.3°，忽略本次更新 ★★★
//    // （防止传感器噪声导致舵机高频微震）
//    if (fabs(Angle - last_output_angle) < 0.3f) 
//    {
//        return; // 直接返回，不更新 PWM
//    }
//    last_output_angle = Angle;
//    
//    // ★★★ 使用你经过验证的旧公式（完美匹配你的 ARR=20000） ★★★
//    uint16_t ccr = (uint16_t)(Angle / 180.0f * 2000.0f + 500.0f);
//    
//    // 安全限幅（防止意外溢出）
//    if (ccr > 2500) ccr = 2500;
//    if (ccr < 500) ccr = 500;
//    
//    PWM_SetCompare1(ccr);
//}

//void Servo_SetAngle(float Angle)
//{
//    if (Angle < 0) Angle = 0;
//    if (Angle > 180) Angle = 180;
//    
//    // 标准舵机映射：0°->CCR=50, 180°->CCR=250
//    uint16_t ccr = 50 + (uint16_t)(Angle * 200.0f / 180.0f);
//    PWM_SetCompare1(ccr);
//}

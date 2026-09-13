// 附加题二 执行端：双路舵机控制
//
// 角度映射（与控制端旧版公式一致）：
//   CCR = angle / 180 * 2000 + 500
//   0° → 500（0.5ms 高电平）
//   90° → 1500（1.5ms 高电平，标准中位）
//   180° → 2500（2.5ms 高电平）
//
// 限幅 + 死区：避免抖动和溢出

#include "stm32f10x.h"
#include "Servo.h"
#include "PWM.h"

static float last_angle[2] = {-999.0f, -999.0f};   // 上次输出角度（死区用）

void Servo_Init(void)
{
    PWM_Init();
    last_angle[0] = -999.0f;
    last_angle[1] = -999.0f;
}

void Servo_SetAngle(uint8_t channel, float Angle)
{
    if (channel > 1) return;

    // 限幅
    if (Angle < 0.0f)   Angle = 0.0f;
    if (Angle > 180.0f) Angle = 180.0f;

    // 防抖死区：变化 < 0.5° 时不更新（避免传感器噪声导致舵机微震）
    // ★ 首次调用（last_angle = -999）必须更新，否则舵机不动
    if (last_angle[channel] > -100.0f && (Angle - last_angle[channel] < 0.5f && Angle - last_angle[channel] > -0.5f))
    {
        return;
    }
    last_angle[channel] = Angle;

    uint16_t ccr = (uint16_t)(Angle / 180.0f * 2000.0f + 500.0f);
    if (ccr > 2500) ccr = 2500;
    if (ccr < 500)  ccr = 500;

    if (channel == 0)
    {
        PWM_SetCompare1(ccr);
    }
    else
    {
        PWM_SetCompare2(ccr);
    }
}

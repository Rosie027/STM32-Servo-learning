// 附加题二 执行端：状态机模块
//
// 数据流：
//   HC-05 收到控制端数据 → DMA1_Ch5 搬入 rx_buf
//                        → 一帧结束 IDLE 中断 → 置 RxComplete
//                        → AppState_Actuator_Poll 检测 RxComplete
//                        → Protocol_Parse 解析三轴角度
//                        → Servo_SetAngle(下层, Roll + k*Yaw)
//                        → Servo_SetAngle(上层, Pitch)
//                        → 清 RxComplete，回 ST_A_IDLE

#include "AppState_Actuator.h"
#include "DMA_UART_RX.h"
#include "Protocol.h"
#include "Servo.h"
#include <stdio.h>

// Yaw 补偿系数：Yaw 范围 ±180°，Roll 范围 ±90°
// k=0.3 时 Yaw ±180° 可补偿 ±54°，过大；选 0.2 → ±36°，更稳定
#define YAW_COMPENSATE_K   0.2f

// 舵机角度中位（90°）
#define SERVO_CENTER       90.0f

// 角度限制到 0~180
static float clamp_angle(float angle)
{
    if (angle < 0.0f)   angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;
    return angle;
}

static volatile AppState_Actuator_t act_state = ST_A_IDLE;

static float last_roll  = 0.0f;
static float last_pitch = 0.0f;
static float last_yaw   = 0.0f;
static uint32_t valid_count = 0;   // 累计有效帧数（OLED 调试用）

void AppState_Actuator_Init(void)
{
    act_state = ST_A_IDLE;
    last_roll = last_pitch = last_yaw = 0.0f;
    valid_count = 0;

    // 初始姿态：双舵机居中
    Servo_SetAngle(0, SERVO_CENTER);
    Servo_SetAngle(1, SERVO_CENTER);
}

void AppState_Actuator_Poll(void)
{
    switch (act_state)
    {
        case ST_A_IDLE:
            if (RxComplete)
            {
                act_state = ST_A_PARSE;
            }
            break;

        case ST_A_PARSE:
        {
            // 解析协议包（packet_buf 由 DMA_UART_RX 中断填充）
            // ★ 协议格式为整数（角度×100），解析后需除以 100 还原
            float r, p, y;
            if (Protocol_Parse((const char*)packet_buf, &r, &p, &y))
            {
                last_roll  = r / 100.0f;
                last_pitch = p / 100.0f;
                last_yaw   = y / 100.0f;
                valid_count++;
                act_state = ST_A_DRIVE;
            }
            else
            {
                // 解析失败（可能是部分包或噪声），丢弃
                act_state = ST_A_IDLE;
            }
            // 不论成功失败都清标志，避免下次循环又来一次
            RxComplete = 0;
            break;
        }

        case ST_A_DRIVE:
        {
            // ★ 舵机联动策略：
            //   下层舵机（channel 0）= Roll + Yaw 补偿
            //     SERVO_CENTER + Roll + k * Yaw
            //   上层舵机（channel 1）= Pitch
            //     SERVO_CENTER + Pitch
            //
            // ★ 角度范围限制 0~180°：
            //   Roll  常见范围 ±90°  → 舵机 0~180
            //   Pitch 常见范围 ±90°  → 舵机 0~180
            //   Yaw   补偿 ±36°      → 不超界
            float lower = SERVO_CENTER + last_roll + YAW_COMPENSATE_K * last_yaw;
            float upper = SERVO_CENTER + last_pitch;

            Servo_SetAngle(0, clamp_angle(lower));
            Servo_SetAngle(1, clamp_angle(upper));

            act_state = ST_A_IDLE;
            break;
        }
    }
}

AppState_Actuator_t AppState_Actuator_GetState(void)
{
    return act_state;
}

void AppState_Actuator_GetLatestEuler(float *roll, float *pitch, float *yaw)
{
    if (roll)  *roll  = last_roll;
    if (pitch) *pitch = last_pitch;
    if (yaw)   *yaw   = last_yaw;
}

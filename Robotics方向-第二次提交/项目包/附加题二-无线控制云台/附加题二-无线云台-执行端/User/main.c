// 附加题二 执行端入口
//
// 硬件：
//   STM32F103C8T6 + HC-05(USART1 PA9/PA10) + 双舵机(TIM2_CH1 PA0 / TIM2_CH2 PA1) + OLED(I2C1 PB6/PB7)
//
// 数据流：
//   HC-05 收到控制端数据 → DMA1_Ch5 搬入 rx_buf → IDLE 中断置 RxComplete
//                        → AppState_Actuator_Poll 检测 → Protocol_Parse 解析 R/P/Y
//                        → Servo_SetAngle(下层, Roll + k*Yaw)
//                        → Servo_SetAngle(上层, Pitch)

#include "stm32f10x.h"
#include "OLED.h"
#include "Delay.h"
#include "Serial.h"
#include "DMA_UART_RX.h"
#include "Servo.h"
#include "AppState_Actuator.h"
#include <stdio.h>

int main(void)
{
    // ==== 模块初始化 ====
    OLED_Init();
    OLED_ShowString(1, 1, "Actuator Ready ");
    OLED_ShowString(2, 1, "HC05 + 2xServo");
    OLED_ShowString(3, 1, "Waiting data..");
    OLED_ShowString(4, 1, "               ");

    Serial_Init();              // USART1 + HC-05（38400 波特率）
    DMA_UART_RX_Init();         // DMA1_Ch5 + IDLE 中断
    Servo_Init();                // 双通道 PWM 舵机
    AppState_Actuator_Init();    // 状态机初始化（含舵机居中）

    Delay_ms(200);

    // ==== 主循环 ====
    uint16_t display_divider = 0;
    char line_buf[20];

    while (1)
    {
        AppState_Actuator_Poll();

        // OLED 刷新：每 500 次 Poll 刷一次（约 200~500ms 一次，避免拖累接收）
        if (++display_divider >= 500)
        {
            display_divider = 0;

            float roll, pitch, yaw;
            AppState_Actuator_GetLatestEuler(&roll, &pitch, &yaw);

            // 整数显示（microlib 不支持 %+d，手动处理符号）
            int r = (int)roll;
            int p = (int)pitch;
            int y = (int)yaw;

            if (r >= 0) snprintf(line_buf, sizeof(line_buf), "R:+%03d   ", r);
            else        snprintf(line_buf, sizeof(line_buf), "R:-%03d   ", -r);
            OLED_ShowString(2, 1, line_buf);

            if (p >= 0) snprintf(line_buf, sizeof(line_buf), "P:+%03d   ", p);
            else        snprintf(line_buf, sizeof(line_buf), "P:-%03d   ", -p);
            OLED_ShowString(3, 1, line_buf);

            if (y >= 0) snprintf(line_buf, sizeof(line_buf), "Y:+%03d   ", y);
            else        snprintf(line_buf, sizeof(line_buf), "Y:-%03d   ", -y);
            OLED_ShowString(4, 1, line_buf);
        }
    }
}

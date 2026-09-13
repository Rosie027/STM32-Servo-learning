#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "PWM.h"
#include "stdlib.h"

// 串口接收缓冲区
#define RX_BUF_SIZE  20                 // 缓冲区大小（最多存19个字符+'\0'）
char RxString[RX_BUF_SIZE];
uint8_t Rx_num_flag = 0;                // 当前接收字符索引

int main(void)
{
    /* 模块初始化 */
    OLED_Init();                        // OLED初始化
    PWM_Init();                         // PWM初始化（PA0输出）
    Serial_Init();                      // 串口初始化（需与HC-05波特率一致）

    /* 显示静态标题 */
    OLED_ShowString(1, 1, "Angle:");    // 第一行显示 "Angle:"
    OLED_ShowString(2, 1, "PWM:");      // 第二行显示 "PWM:"（可选）

    while (1)
    {
        // 检查串口接收标志（必须确保读取后自动清零，否则需手动清）
        if (Serial_GetRxFlag() == 1)
        {
            uint8_t rxChar = Serial_GetRxData();   // 读取一个字节
            Serial_SendByte(rxChar);               // 回传，用于调试

            // 判断是否为换行符（一行结束）
            if (rxChar == '\n')
            {
                // 将接收到的字符串用 '\0' 结尾（换行符不存入）
                RxString[Rx_num_flag] = '\0';

                // 改进1：检查字符串是否以数字开头（过滤无效指令）
                if (RxString[0] >= '0' && RxString[0] <= '9')
                {
                    // 将字符串转换为整数（角度值）
                    uint8_t angle = atoi(RxString);

                    // 改进2：OLED显示解析后的角度（纯数字，占3位）
                    OLED_ShowNum(1, 8, angle, 3);      // 第1行第8列起显示角度

                    // 改进3：根据角度计算PWM占空比（CCR值）
                    // 原公式：angle * 2 / 45 + 5  → 适用于 0~180° 对应 CCR 5~13
                    // 若舵机响应范围不合适，可调整系数，例如：
                    // 角度0°  → CCR = 5   (对应0.5ms脉宽)
                    // 角度180° → CCR = 13  (对应2.5ms脉宽)
                    // 若需更大范围，可修改 ARR 或重新计算系数
                    //uint16_t ccr = angle * 2 / 45 + 5;
					uint16_t ccr = 50 + (uint16_t)((uint32_t)angle * 200 / 180);
                    PWM_SetCompare1(ccr);

                    // 可选：在第二行显示当前PWM值
                    OLED_ShowNum(2, 8, ccr, 3);
                }
                else
                {
                    // 无效指令（不以数字开头），忽略，但可显示错误提示
                    OLED_ShowString(1, 8, "Err ");
                }

                // 重置索引，准备接收下一行
                Rx_num_flag = 0;
            }
            else
            {
                // 普通字符，存入缓冲区（防止溢出）
                if (Rx_num_flag < RX_BUF_SIZE - 1)
                {
                    RxString[Rx_num_flag++] = rxChar;
                }
                else
                {
                    // 缓冲区满，丢弃当前字符并重置（避免溢出）
                    Rx_num_flag = 0;
                }
            }
        }
    }
}

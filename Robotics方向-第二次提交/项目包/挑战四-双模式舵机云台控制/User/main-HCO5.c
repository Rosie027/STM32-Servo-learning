//#include "stm32f10x.h"
//#include "Serial.h"
//#include "Delay.h"

//int main(void)
//{
//    Serial_Init();  // 初始化串口 PA9/PA10, 波特率9600
//    
//    while (1)
//    {
//        // 如果接收标志被置位
//        if (Serial_GetRxFlag() == 1)
//        {
//            uint8_t data = Serial_GetRxData();
//            
//            // 1. 立刻回传给电脑（XCOM 会显示你发的字符）
//            Serial_SendByte(data);
//            
//            // 2. 为了让你确认，再发一个 OK 的缩写
//            Serial_SendString(" OK ");
//        }
//    }
//}

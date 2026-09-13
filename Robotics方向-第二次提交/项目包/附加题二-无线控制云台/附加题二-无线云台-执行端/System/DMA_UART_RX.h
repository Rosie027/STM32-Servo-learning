// 附加题二 执行端：USART1 RX DMA + IDLE 空闲中断
//
// 设计：
//   1. DMA1_Channel5 配置为 USART1_RX（外设→内存，64 字节循环缓冲）
//   2. USART1 使能 IDLE 空闲中断（一帧结束触发）
//   3. IDLE 中断里：关闭 DMA → 读 CNDTR 得本帧长度 → 拷出 → 重启 DMA
//   4. 主循环检测 RxComplete 标志后调用 Protocol_Parse 解析
//
// ★ 关键：执行端不读 I2C2，DMA1_Channel5 可自由分配给 USART1_RX
//   （控制端才是 DMA1_Channel5 = I2C2_RX）

#ifndef __DMA_UART_RX_H
#define __DMA_UART_RX_H

#include "stm32f10x.h"

#define RX_BUF_SIZE 64

extern volatile uint8_t rx_buf[RX_BUF_SIZE];          // DMA 写入缓冲区
extern volatile uint8_t packet_buf[RX_BUF_SIZE + 1];  // 一帧完整数据（带 '\0' 终止）
extern volatile uint16_t packet_len;                  // 本帧长度
extern volatile uint8_t RxComplete;                   // 一帧就绪标志（IDLE 置1，主循环清0）

void DMA_UART_RX_Init(void);

#endif

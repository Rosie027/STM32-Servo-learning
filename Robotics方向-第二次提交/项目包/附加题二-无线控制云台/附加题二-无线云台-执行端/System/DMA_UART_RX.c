// 附加题二 执行端：USART1 RX DMA + IDLE 空闲中断
//
// 状态流：
//   USART1 收到 HC-05 数据 → DMA1_Ch5 自动搬入 rx_buf[64]
//                            → 一帧结束后总线上无新数据 → USART1 触发 IDLE 标志
//                            → 进 USART1_IRQHandler
//                            → 关 DMA、读 CNDTR 得本帧字节数、拷出、重启 DMA
//
// 时序注意：
//   1. 清 IDLE 标志的标准流程：先读 SR1，再读 DR（顺序不可颠倒）
//   2. 关 DMA 后 CNDTR 才会稳定，读出本帧长度 = RX_BUF_SIZE - CNDTR
//   3. 重启 DMA 必须：DISABLE → 重设 CNDTR → ENABLE

#include "DMA_UART_RX.h"

volatile uint8_t rx_buf[RX_BUF_SIZE];
volatile uint8_t packet_buf[RX_BUF_SIZE + 1];
volatile uint16_t packet_len = 0;
volatile uint8_t RxComplete = 0;

void DMA_UART_RX_Init(void)
{
    // 1. DMA1 时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // 2. DMA1_Channel5 配置（USART1_RX）
    DMA_InitTypeDef DMA_InitStruct;
    DMA_DeInit(DMA1_Channel5);

    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;     // 源：USART1 DR
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)rx_buf;               // 目标：接收缓冲区
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;                    // 外设→内存
    DMA_InitStruct.DMA_BufferSize = RX_BUF_SIZE;                        // 64 字节
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;      // DR 地址固定
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;               // 内存递增
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;                          // 单次模式（IDLE 重启）
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStruct);

    // 3. 使能 USART1 的 DMA 接收请求
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

    // 4. 使能 USART1 IDLE 中断
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

    // 5. NVIC：USART1 中断
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 6. 启动 DMA（开始接收）
    DMA_Cmd(DMA1_Channel5, ENABLE);
}

// USART1 中断服务函数（处理 IDLE 空闲中断）
// ★ 此函数与 stm32f10x_it.c 中的 USART1_IRQHandler 二选一，本项目在此实现
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        // 1. 清 IDLE 标志：先读 SR1，再读 DR（F103 硬件规定顺序）
        (void)USART1->SR;
        (void)USART1->DR;

        // 2. 关闭 DMA，CNDTR 才会稳定
        DMA_Cmd(DMA1_Channel5, DISABLE);

        // 3. 计算本帧长度
        uint16_t remaining = DMA_GetCurrDataCounter(DMA1_Channel5);
        uint16_t len = RX_BUF_SIZE - remaining;

        if (len > 0 && len <= RX_BUF_SIZE && !RxComplete)
        {
            // 4. 拷出本帧到 packet_buf（带 '\0' 终止便于 sscanf）
            for (uint16_t i = 0; i < len; i++)
            {
                packet_buf[i] = rx_buf[i];
            }
            packet_buf[len] = '\0';
            packet_len = len;
            RxComplete = 1;
        }
        // else：上一帧未消费或 0 字节，丢弃避免覆盖

        // 5. 重启 DMA 等下一帧
        DMA_SetCurrDataCounter(DMA1_Channel5, RX_BUF_SIZE);
        DMA_Cmd(DMA1_Channel5, ENABLE);
    }

    // 注意：RXNE 中断被 Serial.c 关闭（Serial.c 改用 DMA 后不依赖 RXNE）
    // 但 stm32f10x_it.c 中可能仍有 RXNE 处理逻辑，这里保留兼容
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        (void)USART_ReceiveData(USART1);   // 读 DR 清 RXNE，丢弃（DMA 已处理）
    }
}

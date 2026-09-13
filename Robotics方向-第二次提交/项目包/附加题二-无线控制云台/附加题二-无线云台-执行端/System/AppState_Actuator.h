// 附加题二 执行端：状态机模块
//
// 职责：
//   1. 监测 DMA 接收完成标志 RxComplete
//   2. 解析 R/P/Y 字符串
//   3. 驱动双舵机：下层 = Roll + k*Yaw，上层 = Pitch
//
// 状态流转：
//   ST_A_IDLE --(RxComplete)--> ST_A_PARSE --(成功)--> ST_A_DRIVE --> ST_A_IDLE
//                              --(失败)--> ST_A_IDLE

#ifndef __APPSTATE_ACTUATOR_H
#define __APPSTATE_ACTUATOR_H

#include "stm32f10x.h"

typedef enum {
    ST_A_IDLE = 0,    // 等待 DMA 接收完成
    ST_A_PARSE,       // 解析协议包
    ST_A_DRIVE        // 驱动舵机
} AppState_Actuator_t;

void AppState_Actuator_Init(void);
void AppState_Actuator_Poll(void);
AppState_Actuator_t AppState_Actuator_GetState(void);

// 调试用：返回最近一次解析的三轴角度
void AppState_Actuator_GetLatestEuler(float *roll, float *pitch, float *yaw);

#endif

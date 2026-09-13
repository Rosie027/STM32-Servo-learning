// 附加题二 执行端：协议解析模块
//
// 控制端发送格式："R:+045.20 P:-012.34 Y:+178.90\n"
// 执行端用 sscanf 解析三个 float

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>

// 解析一帧字符串为三轴角度
// 参数：buf 输入字符串（以 '\0' 终止）
//       roll, pitch, yaw 输出三个角度（°）
// 返回 1=成功, 0=失败（格式不符）
uint8_t Protocol_Parse(const char *buf, float *roll, float *pitch, float *yaw);

#endif

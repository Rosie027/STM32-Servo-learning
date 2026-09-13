// 附加题二 执行端：协议解析模块
//
// ★ microlib 限制说明：
//   Keil 默认 microlib 支持 sscanf，但浮点 sscanf 不一定可用（取决于版本）
//   为稳妥起见，本实现手动解析整数部分+小数部分，避免依赖浮点 sscanf
//   支持格式：R:+045.20 P:-012.34 Y:+178.90  或 R:45 P:-12 Y:0
//
// 解析策略：
//   1. 在 buf 中查找 'R:' 'P:' 'Y:' 三个标记
//   2. 从标记后开始解析 float（手写解析，支持正负号、整数+小数）
//   3. 任意一项缺失返回失败

#include "Protocol.h"
#include <string.h>
#include <stdlib.h>

// 在 buf 中查找 target（如 "R:"），找到返回其后位置指针，找不到返回 NULL
static const char* find_token(const char *buf, const char *target)
{
    return strstr(buf, target);
}

// 从 str 开始解析一个 float（支持 [+-]?digits[.digits]?）
// 成功返回 1 并通过 *out 返回值，失败返回 0
static uint8_t parse_float(const char *str, float *out)
{
    if (str == NULL || *str == '\0') return 0;

    float sign = 1.0f;
    if (*str == '+') str++;
    else if (*str == '-') { sign = -1.0f; str++; }

    if (*str < '0' || *str > '9') return 0;   // 必须以数字开头

    // 整数部分
    float int_part = 0.0f;
    while (*str >= '0' && *str <= '9')
    {
        int_part = int_part * 10.0f + (float)(*str - '0');
        str++;
    }

    // 小数部分
    float frac_part = 0.0f;
    float frac_scale = 0.1f;
    if (*str == '.')
    {
        str++;
        while (*str >= '0' && *str <= '9')
        {
            frac_part += (float)(*str - '0') * frac_scale;
            frac_scale *= 0.1f;
            str++;
        }
    }

    *out = sign * (int_part + frac_part);
    return 1;
}

uint8_t Protocol_Parse(const char *buf, float *roll, float *pitch, float *yaw)
{
    if (buf == NULL) return 0;

    const char *p_roll  = find_token(buf, "R:");
    const char *p_pitch = find_token(buf, "P:");
    const char *p_yaw   = find_token(buf, "Y:");

    if (p_roll == NULL || p_pitch == NULL || p_yaw == NULL) return 0;

    // 跳过 "R:" "P:" "Y:" 标记（2 个字符）
    float r, p, y;
    if (!parse_float(p_roll + 2,  &r)) return 0;
    if (!parse_float(p_pitch + 2, &p)) return 0;
    if (!parse_float(p_yaw + 2,   &y)) return 0;

    if (roll)  *roll  = r;
    if (pitch) *pitch = p;
    if (yaw)   *yaw   = y;
    return 1;
}

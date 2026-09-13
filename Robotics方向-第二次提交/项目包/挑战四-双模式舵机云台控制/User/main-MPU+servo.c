#include "stm32f10x.h"
#include "OLED.h"
#include "Delay.h"
#include "Servo.h"
#include "MPU6050.h"
#include <math.h>      // 用于 atan2f、sqrtf

// ======================== 全局变量 ========================

// MPU6050滤波后的俯仰角
float filtered_Pitch = 0.0f;
float filtered_Roll = 0.0f;

// 错误计数器（用于自动复位）
uint8_t MPU_Error_Count = 0;

// ======================== MPU6050更新函数（带异常处理） ========================

void MPU6050_Update(void)
{
    int16_t AX, AY, AZ, GX, GY, GZ;
    static float last_Pitch = 0.0f;
    static float last_Roll = 0.0f;
    float alpha = 0.4f;  // 一阶滤波系数
    
    // (1) 读取原始数据
    MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
    
    // ★★★ 关键：数据有效性检查 ★★★
    // 如果三个轴全是 0（通信失败）或全是 0xFFFF（-1，总线异常），则认为读取失败
    if ((AX == 0 && AY == 0 && AZ == 0) || 
        (AX == -1 && AY == -1 && AZ == -1))
    {
        MPU_Error_Count++;  // 累加错误次数
        
        // 如果连续失败超过 5 次（约 5 * 20ms = 100ms），则复位 MPU6050
        if (MPU_Error_Count > 5)
        {
            MPU6050_Init();     // 重新初始化 MPU6050
            MPU_Error_Count = 0; // 清空计数器
            
            // 显示复位提示（可选）
            OLED_ShowString(4, 1, "RESET");
        }
        
        // 不管是否复位，本次都跳过角度更新，舵机保持上一帧的位置
        return;
    }
    
    // 数据正常，清空错误计数器
    MPU_Error_Count = 0;
    OLED_ShowString(4, 1, "     ");  // 清除复位提示
    
    // (2) 换算为物理值（量程 ±16g，灵敏度 2048 LSB/g）
    float accX_g = (float)AX / 2048.0f;
    float accY_g = (float)AY / 2048.0f;
    float accZ_g = (float)AZ / 2048.0f;
    
    // (3) 计算 Pitch 和 Roll（单位：度）
    float pitch = atan2f(-accX_g, sqrtf(accY_g*accY_g + accZ_g*accZ_g)) * (180.0f / 3.1415926f);
    float roll  = atan2f(accY_g, accZ_g) * (180.0f / 3.1415926f);
    
    // (4) 一阶滞后滤波
    filtered_Pitch = alpha * pitch + (1.0f - alpha) * last_Pitch;
    filtered_Roll  = alpha * roll  + (1.0f - alpha) * last_Roll;
    
    last_Pitch = filtered_Pitch;
    last_Roll = filtered_Roll;
}

// ======================== 主函数 ========================

int main(void)
{
    // ---------- 初始化所有外设 ----------
    OLED_Init();          // OLED 显示
    MPU6050_Init();       // MPU6050 姿态传感器
    Servo_Init();         // 舵机 PWM（PA0）
    
    // ---------- 检查 MPU6050 是否正常 ----------
    uint8_t id = MPU6050_GetID();
    OLED_ShowString(1, 1, "MPU ID:");
    if (id == 0x68) {
        OLED_ShowString(1, 9, "OK");
    } else {
        OLED_ShowString(1, 9, "ERR");  // 如果显示 ERR，检查硬件接线
    }
    Delay_ms(500);  // 延时让用户看到 ID 状态
    
    OLED_Clear();    // 清屏，准备进入主循环
    
    // ---------- 主循环 ----------
    while (1)
    {
        // ========== 第1步：更新 MPU6050 姿态 ==========
        MPU6050_Update();
        
        // ========== 第2步：将俯仰角映射到舵机角度（0°~180°） ==========
        float Target_Angle = filtered_Pitch + 90.0f;
        
        // 安全限幅
        if (Target_Angle < 0.0f)   Target_Angle = 0.0f;
        if (Target_Angle > 180.0f) Target_Angle = 180.0f;
        
        // ========== 第3步：驱动舵机 ==========
        Servo_SetAngle(Target_Angle);
        
        // ========== 第4步：OLED 显示 ==========
        // 第一行：显示模式（固定为 MPU6050）
        OLED_ShowString(1, 1, "Mode:MPU6050");
        
        // 第二行：显示当前目标角度
        int int_part = (int)Target_Angle;
        int dec_part = (int)((Target_Angle - int_part) * 10.0f);
        if (dec_part < 0) dec_part = -dec_part;
        
        OLED_ShowString(2, 1, "Ang:");
        OLED_ShowNum(2, 5, int_part, 3);     // 显示整数，3位
        OLED_ShowChar(2, 8, '.');
        OLED_ShowNum(2, 9, dec_part, 1);
        
        // 第三行：显示滤波后的 Pitch 原始值（调试用）
        int pitch_int = (int)filtered_Pitch;
        int pitch_dec = (int)((filtered_Pitch - pitch_int) * 10.0f);
        if (pitch_dec < 0) pitch_dec = -pitch_dec;
        
        OLED_ShowString(3, 1, "Pitch:");
        OLED_ShowSignedNum(3, 7, pitch_int, 3);
        OLED_ShowChar(3, 10, '.');
        OLED_ShowNum(3, 11, pitch_dec, 1);
        
        // 第四行显示 MPU 状态（OK 或 RESET）
        // 该行由 MPU6050_Update() 内部维护
        
        // ========== 第5步：延时，控制刷新率 ==========
        Delay_ms(20);  // 20ms 刷新一次（50Hz）
    }
}

// ======================== 说明 ========================
// 1. 本文件独立测试 MPU6050 + 舵机，无蓝牙、无按键干扰
// 2. 加入了数据有效性检查，通信异常时会自动复位 MPU6050
// 3. 舵机映射公式使用你原来的 Servo_SetAngle（由 Servo.c 提供）
// 4. OLED 显示角度和 Pitch 原始值，便于调试
// 5. 如果舵机依然卡死，请检查舵机供电（必须外接 5V 电源，且与 STM32 共地）
// 6. 如果 MPU6050 频繁复位（OLED 闪烁 "RESET"），请在 MPU6050 的 VCC/GND 之间并联 100uF 电容


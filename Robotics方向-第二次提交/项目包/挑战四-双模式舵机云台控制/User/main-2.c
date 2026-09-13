#include "stm32f10x.h"
#include "OLED.h"
#include "Key.h"
#include "Delay.h"
#include "Servo.h"
#include "MPU6050.h"
#include "Serial.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// ======================== 全局变量 ========================

// 工作模式：0=蓝牙模式，1=MPU6050姿态模式（默认 MPU 模式，方便直接测试）
uint8_t Work_Mode = 0;

// 蓝牙接收到的目标角度
float Bluetooth_TargetAngle = 0.0f;

// MPU6050 滤波后的角度
float filtered_Pitch = 0.0f;
float filtered_Roll = 0.0f;

// 蓝牙接收缓冲区（沿用你的挑战二代码）
#define RX_BUF_SIZE  32
char RxString[RX_BUF_SIZE];
uint8_t Rx_num_flag = 0;

// MPU 错误计数器（用于自动复位）
uint8_t MPU_Error_Count = 0;

// ======================== MPU6050 更新函数（带防卡死机制） ========================

void MPU6050_Update(void)
{
    int16_t AX, AY, AZ, GX, GY, GZ;
    static float last_Pitch = 0.0f, last_Roll = 0.0f;
    float alpha = 0.4f;  // 滤波系数，保持跟手
    
    MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
    
    // ★ 数据有效性检查：读到全 0 或全 -1 代表 I2C 通信异常
    if ((AX == 0 && AY == 0 && AZ == 0) || (AX == -1 && AY == -1 && AZ == -1))
    {
        MPU_Error_Count++;
        if (MPU_Error_Count > 5) // 连续失败 5 次（约 100ms），复位 MPU
        {
            MPU6050_Init();
            MPU_Error_Count = 0;
            OLED_ShowString(4, 1, "RESET");
        }
        return; // 本次跳过角度更新，舵机保持原位
    }
    MPU_Error_Count = 0;
    OLED_ShowString(4, 1, "     "); // 清除复位提示
    
    // 换算物理值（±16g，灵敏度 2048）
    float accX_g = (float)AX / 2048.0f;
    float accY_g = (float)AY / 2048.0f;
    float accZ_g = (float)AZ / 2048.0f;
    
    // 计算 Pitch 和 Roll
    float pitch = atan2f(-accX_g, sqrtf(accY_g*accY_g + accZ_g*accZ_g)) * (180.0f / 3.1415926f);
    float roll  = atan2f(accY_g, accZ_g) * (180.0f / 3.1415926f);
    
    // 一阶滞后滤波
    filtered_Pitch = alpha * pitch + (1.0f - alpha) * last_Pitch;
    filtered_Roll  = alpha * roll  + (1.0f - alpha) * last_Roll;
    
    last_Pitch = filtered_Pitch;
    last_Roll = filtered_Roll;
}

// ======================== 主函数 ========================

int main(void)
{
    // ---------- 初始化所有外设 ----------
    OLED_Init();
    Serial_Init();      // 初始化蓝牙串口
    MPU6050_Init();
    Servo_Init();       // 初始化舵机 PWM（PA0）
    
    // ---------- 检查 MPU6050 ----------
    uint8_t id = MPU6050_GetID();
    OLED_ShowString(1, 1, "MPU ID:");
    if (id == 0x68) OLED_ShowString(1, 9, "OK");
    else OLED_ShowString(1, 9, "ERR");
    Delay_ms(500);
    OLED_Clear();
    
    // ---------- 主循环 ----------
    while (1)
    {
        // ========== 1. 按键检测（模式切换） ==========
        uint8_t key = Key_GetNum();  // 只检测 PB1
        if (key == 1) {
            Work_Mode = !Work_Mode;  // 蓝牙 ↔ MPU6050 切换
        }
        
        // ========== 2. 更新 MPU6050 姿态 ==========
        MPU6050_Update();
        
        // ========== 3. 处理蓝牙接收数据 ==========
		// ========== 处理蓝牙接收（中断已拼好完整字符串） ==========
		if (RxComplete == 1)
		{
			// 解析字符串为整数
			uint8_t angle = atoi(RxString);
			Bluetooth_TargetAngle = (float)angle;
			if (Bluetooth_TargetAngle > 180.0f) Bluetooth_TargetAngle = 180.0f;
			
			// OLED 显示角度
			OLED_ShowNum(2, 8, angle, 3);
			
			// 清除完成标志，准备接收下一条
			RxComplete = 0;
			// 注意：RxString 不用清空，下次会被覆盖
		}
        
        // ========== 4. 根据模式选择目标角度 ==========
        float Target_Angle = 0.0f;
        
        if (Work_Mode == 0) // 蓝牙模式
        {
            Target_Angle = Bluetooth_TargetAngle;
            OLED_ShowString(1, 1, "Bluetooth Mode ");
        }
        else // MPU6050 姿态模式
        {
            Target_Angle = filtered_Pitch + 90.0f; // -90°~90° 映射到 0°~180°
            if (Target_Angle < 0.0f) Target_Angle = 0.0f;
            if (Target_Angle > 180.0f) Target_Angle = 180.0f;
            OLED_ShowString(1, 1, "MPU6050 Mode  ");
        }
        
        // ========== 5. 驱动舵机（使用你验证过的公式） ==========
        // 直接在外部做防抖死区（滤除 0.3° 以内的微小变化，避免高频抖动）
        static float last_output_angle = -999.0f;
        if (fabs(Target_Angle - last_output_angle) > 0.3f)
        {
            Servo_SetAngle(Target_Angle);
            last_output_angle = Target_Angle;
        }
        
        // ========== 6. OLED 显示角度 ==========
        // 第二行显示当前目标角度
        int int_part = (int)Target_Angle;
        int dec_part = (int)((Target_Angle - int_part) * 10.0f);
        if (dec_part < 0) dec_part = -dec_part;
        
        OLED_ShowString(2, 1, "Ang:");
        OLED_ShowNum(2, 5, int_part, 3);
        OLED_ShowChar(2, 8, '.');
        OLED_ShowNum(2, 9, dec_part, 1);
        
        // 第三行显示 Pitch 原始值（调试用）
        int pitch_int = (int)filtered_Pitch;
        int pitch_dec = (int)((filtered_Pitch - pitch_int) * 10.0f);
        if (pitch_dec < 0) pitch_dec = -pitch_dec;
        OLED_ShowString(3, 1, "Pitch:");
        OLED_ShowSignedNum(3, 7, pitch_int, 3);
        OLED_ShowChar(3, 10, '.');
        OLED_ShowNum(3, 11, pitch_dec, 1);
        
        // ========== 7. 延时，控制刷新率 ==========
        Delay_ms(20); // 50Hz 刷新
    }
}

// ======================== 关键备注 ========================
// 1. Servo_SetAngle 内部使用你验证的旧公式：PWM_SetCompare1(Angle/180*2000+500)
// 2. ARR = 20000-1, PSC = 72-1, 产生 50Hz PWM
// 3. 蓝牙默认波特率 9600，字符串以 '\n' 结尾
// 4. 按 PB1 切换模式，OLED 第一行显示当前模式


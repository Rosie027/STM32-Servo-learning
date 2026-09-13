#include "stm32f10x.h"
#include "OLED.h"
#include "Key.h"
#include "PWM.h"
#include "Delay.h"
#include "Servo.h"
#include "MPU6050.h"
#include "Serial.h"
#include <math.h>      // 用于 atan2f、sqrtf
#include <stdlib.h>    // 用于 atoi
#include <string.h>    // 用于字符串操作

// ======================== 1. 定义全局变量 ========================

// 工作模式：0=蓝牙模式，1=MPU6050姿态模式
uint8_t Work_Mode = 1;

// 蓝牙接收到的目标角度（由蓝牙解析后存入）
float Bluetooth_TargetAngle = 0.0f;

// MPU6050滤波后的俯仰角（由 MPU6050_Update 更新）
float filtered_Pitch = 0.0f;
float filtered_Roll = 0.0f;   // 横滚角，暂时不用但保留

// ========== 蓝牙接收缓冲区（完全沿用你挑战二的代码） ==========
#define RX_BUF_SIZE  32
char RxString[RX_BUF_SIZE];
uint8_t Rx_num_flag = 0;

// ================================================================
// ======================== 2. MPU6050更新函数 ========================
// ================================================================

void MPU6050_Update(void)
{
    int16_t AX, AY, AZ, GX, GY, GZ;
    static float last_Pitch = 0.0f;
    static float last_Roll = 0.0f;
    
    // 滤波系数：你的DLPF=0x02（92Hz），软件用0.4刚刚好
    float alpha = 0.4f;
    
    // (1) 读取原始数据（调用你已有的函数）
    MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
    
    // (2) 换算为物理值（你配置的量程是 ±16g，灵敏度 2048 LSB/g）
    float accX_g = (float)AX / 2048.0f;
    float accY_g = (float)AY / 2048.0f;
    float accZ_g = (float)AZ / 2048.0f;
    
    // (3) 利用加速度计计算 Pitch 和 Roll（单位：度）
    float pitch = atan2f(-accX_g, sqrtf(accY_g*accY_g + accZ_g*accZ_g)) * (180.0f / 3.1415926f);
    float roll  = atan2f(accY_g, accZ_g) * (180.0f / 3.1415926f);
    
    // (4) 一阶滞后滤波
    filtered_Pitch = alpha * pitch + (1.0f - alpha) * last_Pitch;
    filtered_Roll  = alpha * roll  + (1.0f - alpha) * last_Roll;
    
    last_Pitch = filtered_Pitch;
    last_Roll = filtered_Roll;
}

// ================================================================
// ======================== 3. 主函数 ==============================
// ================================================================

int main(void)
{
	
		// ---------- 初始化所有外设 ----------
    OLED_Init();
    Serial_Init();          // 初始化串口（蓝牙）
    MPU6050_Init();         // 初始化MPU6050
    Servo_Init();           // 初始化舵机PWM
    
    // ---------- 检查MPU6050是否正常（可选） ----------
//    uint8_t id = MPU6050_GetID();
//    OLED_ShowString(1, 1, "MPU ID:");
//    if (id == 0x68) {
//        OLED_ShowString(1, 8, "OK   ");
//    } else {
//        OLED_ShowString(1, 8, "ERR  ");
//    }
//	Delay_s(5000);
//	OLED_Clear();
	
	
    
    // 延时等待一下，让OLED显示稳定（简单的延时循环）
    //for (uint32_t i = 0; i < 500000; i++);
	
	// 清屏，准备进入主循环
    //OLED_Clear();
	
	
    // ========== 新增：按键检测（轮询方式） ==========
    uint8_t key = Key_GetNum();   // 获取按键值（1 或 2）
    if (key == 1)     // 按键按下都切换模式
    {
        Work_Mode = !Work_Mode;   // 模式翻转
        // 可选：切换时蜂鸣器响一下或LED闪一下提示（如果你有的话）
    }

	
	
    

    
    // ---------- 主循环 ----------
    while (1)
    {
		// ★★★ 按键检测（每次循环都检测） ★★★
//        uint8_t key = Key_GetNum();
//        if (key == 1) {
//            Work_Mode = !Work_Mode;
//        }
		
        // ========== 第一步：始终在后台更新MPU6050姿态 ==========
        MPU6050_Update();
        
        // ========== 第二步：处理蓝牙接收（完全沿用你的代码，只改了一行） ==========
        if (Serial_GetRxFlag() == 1)
        {
            uint8_t rxChar = Serial_GetRxData();   // 读取一个字节
            Serial_SendByte(rxChar);               // 回传，用于调试

            // 判断是否为换行符（一行结束）
            if (rxChar == '\n')
            {
                // 将接收到的字符串用 '\0' 结尾
                RxString[Rx_num_flag] = '\0';

                // 检查字符串是否以数字开头（过滤无效指令）
                if (RxString[0] >= '0' && RxString[0] <= '9')
                {
                    // 将字符串转换为整数（角度值）
                    uint8_t angle = atoi(RxString);
                    
                    // ★★★ 唯一改动：不再直接驱动舵机，而是把角度存入全局变量 ★★★
                    Bluetooth_TargetAngle = (float)angle;
                    if (Bluetooth_TargetAngle > 180.0f) {
                        Bluetooth_TargetAngle = 180.0f;
                    }
                    
                    // OLED显示解析后的角度（纯数字，占3位）
                    OLED_ShowNum(2, 8, angle, 3);
                }
                else
                {
                    // 无效指令（不以数字开头），显示错误提示
                    OLED_ShowString(2, 8, "Err ");
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
                    // 缓冲区满，丢弃当前字符并重置
                    Rx_num_flag = 0;
                }
            }
        }
        
        // ========== 第三步：根据工作模式选择目标角度 ==========
        float Target_Angle = 0.0f;
        
        if (Work_Mode == 0)   // 蓝牙模式
        {
            Target_Angle = Bluetooth_TargetAngle;
            OLED_ShowString(1, 1, "Mode:Bluetooth");
        }
        else                  // MPU6050姿态模式
        {
            // 将 filtered_Pitch（-90° ~ +90°）映射到舵机范围（0° ~ 180°）
            Target_Angle = filtered_Pitch + 90.0f;
            
            // 安全限幅（虽然 Servo_SetAngle 内部也会限幅，但这里先做更保险）
            if (Target_Angle < 0.0f)   Target_Angle = 0.0f;
            if (Target_Angle > 180.0f) Target_Angle = 180.0f;
            
            OLED_ShowString(1, 1, "Mode:MPU6050");
        }
        
        // ========== 第四步：驱动舵机转动 ==========
        Servo_SetAngle(Target_Angle);
        
        // ========== 第五步：OLED显示当前角度（浮点数拆成字符串） ==========
        // 第一行已经显示模式了，我们在第二行显示角度
        // 拆分整数部分和小数部分（保留1位小数）
        int int_part = (int)Target_Angle;
        int dec_part = (int)((Target_Angle - int_part) * 10.0f);
        if (dec_part < 0) dec_part = -dec_part;   // 防止负数取小数出问题
        
        OLED_ShowString(2, 1, "Ang:");
        OLED_ShowSignedNum(2, 5, int_part, 3);    // 显示整数，占3位
        OLED_ShowChar(2, 9, '.');                 // 小数点
        OLED_ShowNum(2, 10, dec_part, 1);          // 显示1位小数
        OLED_ShowChar(2, 11, ' ');                 // 补个空格
        
        // 可选：在第三行显示滤波后的Pitch原始值（调试用）
        int pitch_int = (int)filtered_Pitch;
        int pitch_dec = (int)((filtered_Pitch - pitch_int) * 10.0f);
        if (pitch_dec < 0) pitch_dec = -pitch_dec;
        OLED_ShowString(3, 1, "Pitch:");
        OLED_ShowSignedNum(3, 7, pitch_int, 3);
        OLED_ShowChar(3, 11, '.');
        OLED_ShowNum(3, 12, pitch_dec, 1);
        
        // 给舵机一点反应时间，同时避免OLED刷新太快导致闪烁
        for (uint32_t i = 0; i < 200000; i++);   // 约20ms的粗略延时
    }
}

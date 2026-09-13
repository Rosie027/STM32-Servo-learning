#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"
#include "math.h"

uint8_t ID;								//定义用于存放ID号的变量
int16_t AX, AY, AZ, GX, GY, GZ;			//定义用于存放各个数据的变量

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	MPU6050_Init();		//MPU6050初始化
	
	/*显示ID号*/
	OLED_ShowString(1, 1, "ID:");		//显示静态字符串
	ID = MPU6050_GetID();				//获取MPU6050的ID号
	OLED_ShowHexNum(1, 4, ID, 2);		//OLED显示ID号
	
	//float filtered_Pitch = 0, filtered_Roll = 0;
	// 一阶滞后滤波（如果你改了硬件DLPF为0x00，α用0.3~0.5；如果没改硬件，α用0.8）
	//float alpha = 0.4; 
	while (1)
	{
		MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);		//获取MPU6050的数据
		//每个得到的参数都是16位
		/*加速度计配置 MPU6050_ACCEL_CONFIG = 0x18 → 对应 ±16g 量程，灵敏度为 2048 LSB/g。
陀螺仪配置 MPU6050_GYRO_CONFIG = 0x18 → 对应 ±2000°/s 量程，灵敏度为 16.4 LSB/°/s。
		*/
		// 加速度换算（单位：g）
		float AccX_g = (float)AX / 2048.0;
		float AccY_g = (float)AY / 2048.0;
		float AccZ_g = (float)AZ / 2048.0;

		// 陀螺仪换算（单位：度/秒），留给附加题卡尔曼用
		float GyroX_dps = (float)GX / 16.4;
		float GyroY_dps = (float)GY / 16.4;
		float GyroZ_dps = (float)GZ / 16.4;

		// 计算Pitch和Roll（公式通用，不受量程影响）
		float pitch = atan2f(-AccX_g, sqrtf(AccY_g*AccY_g + AccZ_g*AccZ_g)) * (180.0 / 3.14159);
		float roll  = atan2f(AccY_g, AccZ_g) * (180.0 / 3.14159);
		
		OLED_ShowSignedNum(2, 1, pitch, 5);					//OLED显示数据
		OLED_ShowSignedNum(3, 1, roll, 5);
		
		// 定义静态变量存储上一次滤波值
		static float filtered_Pitch = 0, filtered_Roll = 0;

		// 一阶滞后滤波（如果你改了硬件DLPF为0x00，α用0.3~0.5；如果没改硬件，α用0.8）
		float alpha = 0.5; 

		filtered_Pitch = alpha * pitch + (1.0f - alpha) * filtered_Pitch;
		filtered_Roll  = alpha * roll  + (1.0f - alpha) * filtered_Roll;

		// 1. 拆出整数部分（直接强转，C语言向零取整）
		int pitch_int = (int)filtered_Pitch;        // 15.7 -> 15
		// 2. 拆出小数部分（取绝对值，防止负号干扰）
		int pitch_dec = (int)((filtered_Pitch - pitch_int) * 10); // 15.7-15=0.7 -> 7
		if(pitch_dec < 0) pitch_dec = -pitch_dec;   // 如果原数是-15.7，小数部分强制变正7

		// 显示：调用 OLED_ShowSignedNum 显示整数（保留4个字符宽度，靠右对齐）
		OLED_ShowSignedNum(2, 8, pitch_int, 4);     // 显示 "  15" (宽度4)
		OLED_ShowChar(2, 13, '.');                   // 显示 "."
		OLED_ShowNum(2, 14, pitch_dec, 1);           // 显示 "7" (宽度1)

		// ========== 显示 Roll（第3行，第0列开始） ==========
		int roll_int = (int)filtered_Roll;
		int roll_dec = (int)((filtered_Roll - roll_int) * 10);
		if(roll_dec < 0) roll_dec = -roll_dec;

		OLED_ShowSignedNum(3, 8, roll_int, 4);      // 显示整数
		OLED_ShowChar(3, 13, '.');                   // 显示点
		OLED_ShowNum(3, 14, roll_dec, 1);            // 显示小数
		
		
//		OLED_ShowSignedNum(2, 1, AccX_g, 5);					//OLED显示数据
//		OLED_ShowSignedNum(3, 1, AccY_g, 5);
//		OLED_ShowSignedNum(4, 1, AccZ_g, 5);
//		OLED_ShowSignedNum(2, 8, GyroX_dps, 5);
//		OLED_ShowSignedNum(3, 8, GyroY_dps, 5);
//		OLED_ShowSignedNum(4, 8, GyroZ_dps, 5);
	}
}

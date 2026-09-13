#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"

extern const uint8_t BMP_ICON[];

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	OLED_ShowCN(2,3,3,1);
	OLED_ShowCN(2,5,4,1);
	
	for (int i=0;i<100;i++)
	{
		Delay_ms(10);;
	}
	OLED_Clear();
	
	OLED_ShowCN(1,1,0,1);
	OLED_ShowCN(1,2,1,1);
	OLED_ShowCN(1,3,2,1);
	OLED_ShowString(2,1,"20250909");
	OLED_ShowString(3,1,"11027");
	for (int i=0;i<100;i++)
	{
		Delay_ms(10);;
	}
	
	OLED_ShowBMP(75,0, 51, 64, BMP_ICON,0);
	while (1)
	{	
//		OLED_ShowCN(1,1,0,1);
//		OLED_ShowCN(1,2,1,1);
//		OLED_ShowCN(1,3,2,1);
		//OLED_ShowCN(3,7,3,1);
		//OLED_ShowCN(3,8,4,1);
		//OLED_ShowBMP(0, 0, 64, 64, BMP_ICON,0);
		//Delay_ms(1);
	}
}




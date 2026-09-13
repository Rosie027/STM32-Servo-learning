#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "PWM.h"
#include "LED.h"
#include "Delay.h"

uint8_t i;

int main(void)
{ 
	OLED_Init();
//	OLED_ShowString(1,1,"wonderful!");
//	OLED_ShowHexNum(2,1,0xAA55,4);
//	OLED_ShowBinNum(3,1,0xAA55,16);
	
	PWM_Init();
	while(1)
	{
		for (i=0;i<=100;i++)
		{
			PWM_SetCompare1(i);
			Delay_ms(10);
		}
		for (i=100;i>0;i--)
		{
			PWM_SetCompare1(i);
			Delay_ms(10);
		}
	}

}


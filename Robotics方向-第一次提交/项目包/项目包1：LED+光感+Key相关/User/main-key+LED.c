#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "Key.h"

//使用按键控制的LED等的点亮和熄灭

int main(void)
{
	//任务四：用按键实现LED的可控闪烁
	uint8_t KeyNum;		//定义用于接收按键键码的变量
	
	/*模块初始化*/
	LED_Init();		//LED初始化1和2在GPIOA
	Key_Init();		//按键初始化1和11在GPIOB
		//一个在1和11，一个在1和2，是否有问题？——没有
		/*
		时钟冲突？ 没有，LED用GPIOA，按键用GPIOB，各自开启自己的时钟，互不影响。
		引脚重叠？ 没有，PA1/PA2 与 PB1/PB11 不重叠。
		*/
	
	while (1)
	{
		KeyNum = Key_GetNum();		//获取按键键码
		
		if (KeyNum == 1)			//按键1按下
		{
			LED_Turn_one(GPIO_Pin_1);			//LED1翻转
			//GPIO_ResetBits(GPIOA, GPIO_Pin_1);
		}
		
//		if (KeyNum == 2)			//按键2按下
//		{
//			LED_Turn_one(GPIO_Pin_2);
//			//GPIO_SetBits(GPIOA, GPIO_Pin_1);			//LED2翻转
//		}
	}
}


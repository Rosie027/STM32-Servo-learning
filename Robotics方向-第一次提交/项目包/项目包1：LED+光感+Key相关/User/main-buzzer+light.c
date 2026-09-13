#include "stm32f10x.h"                  // Device header
#include "Delay.h"
//#include "Buzzer.h"
#include "LightSensor.h"
#include "LED.h"

int main(void)
{
	//任务七：使用光传感器实现“天亮灯灭，天暗灯亮”的功能
	/*模块初始化*/
//	Buzzer_Init();			//蜂鸣器初始化
	LightSensor_Init();		//光敏传感器初始化
	LED_Init();
	while (1)
	{
		if (LightSensor_Get() == 1)		//如果当前光敏输出1
		{
//			Buzzer_ON();				//蜂鸣器开启
			LED_ON(GPIO_Pin_1);
		}
		else							//否则
		{
//			Buzzer_OFF();				//蜂鸣器关闭
			LED_OFF(GPIO_Pin_1);
		}
	}
}

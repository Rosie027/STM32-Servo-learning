#include "stm32f10x.h"                  // Device header

/*
函数：LED初始化
参数：无
返回值：无
 注释：GPIOA  GPIO_Pin_1 | GPIO_Pin_2   GPIO_Mode_AF_PP
*/
void LED_Init(void)
{
	//1.
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
		//为什么有时候是A，有时候是B
		/*开发板出厂时会设计好哪些外设连接到了哪些引脚，根据电路图（原理图）编写对应的程序即可。*/

	//2.
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
		//为什么这个就是使用推挽输出？——见得多了就明白电路逻辑了
		/*
		普通推挽：控制LED、蜂鸣器、继电器、普通数字信号。
		普通开漏：软件模拟I2C、模拟单总线、电平转换、多个输出“线与”。
		复用推挽：UART TX、SPI MOSI/SCK、PWM输出。
		复用开漏：硬件I2C（SDA/SCL）。
		*/
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//3.
	GPIO_SetBits(GPIOA,GPIO_Pin_1 | GPIO_Pin_2); //设置PA1和PA2引脚为高电平
		//解释：高电平就是不亮状态？——不是，是针对这种低电平点亮电路
		/*
		低电平点亮（最常见，尤其是开发板上的LED）
		电路接法：LED的正极（阳极）通过一个电阻连接到电源（VCC，比如3.3V）
			LED的负极（阴极）连接到STM32的引脚（比如PA1）。
		工作情况：

			引脚输出低电平（0V）：电流从VCC → LED → 引脚，LED点亮。
			引脚输出高电平（3.3V）：LED两端电压相等（都是3.3V），没有电流通过，LED熄灭。
		结论：在这种电路下，高电平 = 不亮。
		*/
}

/**
  * 函    数：LED开启
  * 参    数：GPIO_Pin_X(X:0--15)  
				由于不建议all拉低电平，所以这里不标记All了
  * 返 回 值：无
  */
void LED_ON(uint16_t GPIO_Pin_X)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_X);		//设置PAX引脚为低电平
}

/**
  * 函    数：LED关闭
  * 参    数：GPIO_Pin_X(X:0--15)  
  * 返 回 值：无
  */
void LED_OFF(uint16_t GPIO_Pin_X)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_X);		//设置PAX引脚为高电平
}

/**
  * 函    数：一个LED状态翻转,不能针对多个LED
  * 参    数：GPIO_Pin_X(X:0--15)  
  * 返 回 值：无
  */
void LED_Turn_one(uint16_t GPIO_Pin_X)
{
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_X) == 0)		//获取输出寄存器的状态，如果当前引脚输出低电平
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_X);					//则设置PA1引脚为高电平
	}
	else													//否则，即当前引脚输出高电平
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_X);					//则设置PA1引脚为低电平
	}
}


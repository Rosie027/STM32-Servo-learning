#include "stm32f10x.h"                  // Device header
# include "Delay.h"
/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  */
void Key_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		//为什么使用IPU了？
		/*
		按键是“输入设备”，不需要输出电流去驱动负载，只需要读取引脚上的电平（高/低）。
		
		代码里 GPIO_Mode_IPU 就是“内部上拉输入”。
		电路通常这样接：按键一端接 PB1，另一端接 GND（地）。
		没有按下：内部上拉电阻把 PB1 拉到 高电平（3.3V），单片机读到 1。
		按下：PB1 和 GND 接通，引脚被强行拉到 低电平（0V），单片机读到 0。
		*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //| GPIO_Pin_11
		//为什么是1和11？
		//只要避开一些特殊功能的引脚，可以自由选择大部分普通 GPIO
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);						//将PB1和PB11引脚初始化为上拉输入
}


/**
  * 函    数：按键获取键码
  * 参    数：无
  * 返 回 值：按下按键的键码值，范围：0~2，返回0代表没有按键按下
  * 注意事项：此函数是阻塞式操作
				当按键按住不放时，函数会卡住，直到按键松手

和上面的函数一样，都是选择的1和11 的两个引脚
  */
//uint8_t Key_GetNum(void)
//{
//	uint8_t KeyNum = 0;		//定义变量，默认键码值为0
//	
//	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)			//读PB1输入寄存器的状态，如果为0，则代表按键1按下
//	{
//		Delay_ms(20);											//延时消抖
//		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0);	//等待按键松手
//		Delay_ms(20);											//延时消抖
//		KeyNum = 1;												//置键码为1
//	}
//	
////	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)			//读PB11输入寄存器的状态，如果为0，则代表按键2按下
////	{
////		Delay_ms(20);											//延时消抖
////			//为什么需要延时消抖
////			/*
////			等信号稳定后再读取，确保只识别一次有效按键。
////			经验值：5~20ms 是常用区间。太短（<5ms）可能消不掉抖动；太长（>50ms）会感觉按键“迟钝”。
////			*/
////		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0);	//等待按键松手
////		Delay_ms(20);											//延时消抖
////		KeyNum = 2;												//置键码为2
////	}
//	
//	return KeyNum;			//返回键码值，如果没有按键按下，所有if都不成立，则键码为默认值0
//}


//------------这一版可以实现按键后只变化一次，但是需要松手后等待一秒多才行动------------
uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;		//定义变量，默认键码值为0
	
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)			//读PB1输入寄存器的状态，如果为0，则代表按键1按下
	{
		Delay_ms(20);											//延时消抖
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0);	//等待按键松手
		Delay_ms(20);											//延时消抖
		KeyNum = 1;												//置键码为1
	}
	
	
	return KeyNum;			//返回键码值，如果没有按键按下，所有if都不成立，则键码为默认值0
}



//--------------这一班可以实现按键操作，但每次按下会有多次的按键状态切换，但是BT和MPU状态一直切换，开始延时较少，但是按键结束后仍有很长时间的状态移动，延时长---------
//uint8_t Key_GetNum(void)
//{
//    uint8_t KeyNum = 0;
//    uint32_t timeout = 0;  // 超时计数器

//    // 检测按键1 (PB1)
//    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
//    {
//        Delay_ms(20);  // 消抖
//        
//        // ★★★ 关键改动：带超时的等待松手 ★★★
//        timeout = 0;
//        while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
//        {
//            timeout++;
//            if (timeout > 50000)  // 约 50ms 超时（防止死锁）
//            {
//                break;  // 强行退出，不再死等
//            }
//        }
//        
//        Delay_ms(20);  // 消抖
//        KeyNum = 1;    // 记录按键按下
//    }

//    return KeyNum;  // 返回键码
//}



#include "stm32f10x.h"                  // Device header
#include "Delay.h" //如果忘记包含路径就找不到头文件

int main(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE ); //打开时钟
	
	GPIO_InitTypeDef GPIO_InitStructure1;
	GPIO_InitStructure1.GPIO_Mode=GPIO_Mode_Out_PP;
	//GPIO_InitStructure1.GPIO_Pin=GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5; //流水灯加上多个端口，使用或进行初始化
	GPIO_InitStructure1.GPIO_Pin=GPIO_Pin_1;
	GPIO_InitStructure1.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure1);
	
	//GPIO_SetBits(GPIOB,GPIO_Pin_12);
	while(1)
	{
		//任务五：使用delay函数完成led以特定频率闪烁
		GPIO_ResetBits(GPIOA,GPIO_Pin_1); //点亮
		Delay_ms(500);
		GPIO_SetBits(GPIOA,GPIO_Pin_1); //熄灭
		Delay_ms(500);
		
//		任务六：五个灯泡的流水灯效果——write函数
//		GPIO_Write(GPIOA,~0x0001); //0000 0000 0000 0001  GPIOA 0的引脚
//		Delay_ms(500);
//		GPIO_Write(GPIOA,~0x0002);
//		Delay_ms(500);
//		GPIO_Write(GPIOA,~0x0004);
//		Delay_ms(500);
//		GPIO_Write(GPIOA,~0x0008);
//		Delay_ms(500);
//		GPIO_Write(GPIOA,~0x0010);
//		Delay_ms(500);
		
	}
}

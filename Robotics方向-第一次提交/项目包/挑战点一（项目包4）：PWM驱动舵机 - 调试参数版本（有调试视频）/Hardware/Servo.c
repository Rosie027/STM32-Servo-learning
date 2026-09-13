#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_Init(void)
{
	PWM_Init();
}

void Servo_SetAngle(float Angle)
{
	//PWM_SetCompare3(Angle/180*2000+500);
	//PWM_SetCompare3(20000 - (Angle/180*2000 + 500));
	//PWM_SetCompare3(Angle/180*20000+5000);
	PWM_SetCompare3(Angle/180*200+50);//这个公式天生是为 极性 High 且 PSC=720-1（10µs/格） 设计的
}

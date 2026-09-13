#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "LED.h"
#include "string.h"
#include <math.h>
#include <stdlib.h>

uint8_t a = 123;
float x, y1, y2;

int main(void)
{
	OLED_Init();
	LED_Init();
	Serial_Init();
	
	OLED_ShowString(1, 1, "TxPacket");
	OLED_ShowString(3, 1, "RxPacket");
	
//	printf("[display,0,0,Hello World]");
//	printf("[display,0,20,a:%03d]", a);
	
	while (1)
	{
//		x += 0.1;
//		y1 = sin(x);
//		y2 = cos(x);
//		printf("[plot,%f,%f]", y1, y2);
		
		if (Serial_RxFlag == 1)
		{
			OLED_ShowString(4, 1, "                ");
			OLED_ShowString(4, 1, Serial_RxPacket);
			
			//strtok strcmp atoi/atof
			
			char *Tag = strtok(Serial_RxPacket, ",");
			if (strcmp(Tag, "key") == 0)
			{
				char *Name = strtok(NULL, ",");
				char *Action = strtok(NULL, ",");
				
				if (strcmp(Name, "1") == 0 && strcmp(Action, "up") == 0)
				{
					printf("key,1,up\r\n");
				}
				else if (strcmp(Name, "2") == 0 && strcmp(Action, "down") == 0)
				{
					printf("key,2,down\r\n");
				}
			}
			else if (strcmp(Tag, "slider") == 0)
			{
				char *Name = strtok(NULL, ",");
				char *Value = strtok(NULL, ",");
				
				if (strcmp(Name, "1") == 0)
				{
					uint8_t IntValue = atoi(Value);
					
					printf("slider,1,%d\r\n", IntValue);
				}
				else if (strcmp(Name, "2") == 0)
				{
					float FloatValue = atof(Value);
					
					printf("slider,2,%f\r\n", FloatValue);
				}
			}
			else if (strcmp(Tag, "joystick") == 0)
			{
				int8_t LH = atoi(strtok(NULL, ","));
				int8_t LV = atoi(strtok(NULL, ","));
				int8_t RH = atoi(strtok(NULL, ","));
				int8_t RV = atoi(strtok(NULL, ","));
				
				printf("joystick,%d,%d,%d,%d\r\n", LH, LV, RH, RV);
			}
			
			Serial_RxFlag = 0;
		}
	}
}

#include "main.h"
#include "Encoder.h"

/* CubeMX生成的TIM3/TIM4句柄（定义在tim.c中） */
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

/**
  * @brief  编码器接口初始化
  * @note   TIM3(PA6/PA7)/TIM4(PB6/PB7)的编码器模式、输入滤波(0xF)、
  *         TIM4_CH2反相极性 均由CubeMX的MX_TIM3_Init()/MX_TIM4_Init()配置，
  *         此处仅启动编码器接口。
  */
void Encoder_Init(void)
{
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
}

/* n=1为左电机(TIM3),n=2为右电机(TIM4) */
int16_t Encoder_Get(uint8_t n)
{
	int16_t Temp;
	if(n==1)
	{
		Temp = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
		__HAL_TIM_SET_COUNTER(&htim3, 0);
		return Temp;
	}
	else if(n==2)
	{
		Temp = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);
		__HAL_TIM_SET_COUNTER(&htim4, 0);
		return Temp;
	}
	return 0;
}

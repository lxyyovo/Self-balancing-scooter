#include "main.h"
#include "PWM.h"

/* CubeMX生成的TIM2句柄（定义在tim.c中） */
extern TIM_HandleTypeDef htim2;

/**
  * @brief  PWM输出初始化
  * @note   TIM2的时基（ARR=100-1, PSC=36-1 -> 20kHz）与CH1/CH2(PA0/PA1)
  *         由CubeMX的MX_TIM2_Init()配置，此处仅启动PWM输出。
  */
void PWM_Init(void)
{
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
}

void PWM_SetCompare1(uint16_t Compare)
{
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, Compare);
}

void PWM_SetCompare2(uint16_t Compare)
{
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, Compare);
}

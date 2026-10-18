#include "main.h"
#include "Timer.h"

/* CubeMX生成的TIM1句柄（定义在tim.c中） */
extern TIM_HandleTypeDef htim1;

/**
  * @brief  定时中断初始化
  * @note   TIM1时基（1ms）与NVIC优先级（抢占2/子1，Group2）
  *         均由CubeMX的MX_TIM1_Init()配置，此处仅启动更新中断。
  *         1ms到期的处理逻辑放在main.c的HAL_TIM_PeriodElapsedCallback()中。
  */
void Timer_Init(void)
{
	HAL_TIM_Base_Start_IT(&htim1);
}

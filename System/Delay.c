#include "main.h"
#include "Delay.h"

/**
  * @brief  初始化DWT内核周期计数器（用于微秒级延时）
  * @note   改用DWT而非SysTick，避免与HAL的1ms时基（HAL_IncTick）冲突
  */
static void DWT_Init(void)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，单位微秒
  */
void Delay_us(uint32_t xus)
{
	static uint8_t inited = 0;
	uint32_t start, ticks;

	if (!inited)
	{
		DWT_Init();
		inited = 1;
	}

	start = DWT->CYCCNT;								//记录起始周期数
	ticks = xus * (SystemCoreClock / 1000000U);			//微秒换算为内核周期数
	while ((DWT->CYCCNT - start) < ticks);				//等待计数到达
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，单位毫秒
  */
void Delay_ms(uint32_t xms)
{
	HAL_Delay(xms);
}

/**
  * @brief  秒级延时
  * @param  xs 延时时长，单位秒
  */
void Delay_s(uint32_t xs)
{
	while (xs--)
	{
		HAL_Delay(1000);
	}
}

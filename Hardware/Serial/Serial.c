#include "main.h"
#include "Serial.h"
#include <stdio.h>
#include <stdarg.h>

/* CubeMX生成的USART1句柄（定义在usart.c中） */
extern UART_HandleTypeDef huart1;

uint8_t Serial_RxData;
uint8_t Serial_RxFlag;

/**
  * @brief  串口1初始化
  * @note   USART1(PA9/PA10, 9600, 8N1)由CubeMX的MX_USART1_UART_Init()配置，
  *         此处仅启动单字节接收中断。
  */
void Serial_Init(void)
{
	HAL_UART_Receive_IT(&huart1, &Serial_RxData, 1);
}

void Serial_SendByte(uint8_t Byte)
{
	HAL_UART_Transmit(&huart1, &Byte, 1, HAL_MAX_DELAY);
}

void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	HAL_UART_Transmit(&huart1, Array, Length, HAL_MAX_DELAY);
}

void Serial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		Serial_SendByte(String[i]);
	}
}

uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y --)
	{
		Result *= X;
	}
	return Result;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)
	{
		Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/* 重定向printf到串口1 */
int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);
	return ch;
}

void Serial_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	Serial_SendString(String);
}

uint8_t Serial_GetRxFlag(void)
{
	if (Serial_RxFlag == 1)
	{
		Serial_RxFlag = 0;
		return 1;
	}
	return 0;
}

uint8_t Serial_GetRxData(void)
{
	return Serial_RxData;
}

/* 接收完成回调:置标志并重新装载下一次接收 */
void Serial_RxCpltCallback(void)
{
	Serial_RxFlag = 1;
	HAL_UART_Receive_IT(&huart1, &Serial_RxData, 1);
}

#include "main.h"
#include "BlueSerial.h"
#include <stdio.h>
#include <stdarg.h>

/* CubeMX生成的USART2句柄（定义在usart.c中） */
extern UART_HandleTypeDef huart2;

char BlueSerial_RxPacket[100];
uint8_t BlueSerial_RxFlag;
static uint8_t BlueSerial_RxData;		//接收中断单字节缓存

/**
  * @brief  蓝牙串口初始化
  * @note   USART2(PA2/PA3, 9600, 8N1)由CubeMX的MX_USART2_UART_Init()配置，
  *         此处仅启动单字节接收中断。
  */
void BlueSerial_Init(void)
{
	HAL_UART_Receive_IT(&huart2, &BlueSerial_RxData, 1);
}

void BlueSerial_SendByte(uint8_t Byte)
{
	HAL_UART_Transmit(&huart2, &Byte, 1, HAL_MAX_DELAY);
}

void BlueSerial_SendArray(uint8_t *Array, uint16_t Length)
{
	HAL_UART_Transmit(&huart2, Array, Length, HAL_MAX_DELAY);
}

void BlueSerial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		BlueSerial_SendByte(String[i]);
	}
}

uint32_t BlueSerial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y --)
	{
		Result *= X;
	}
	return Result;
}

void BlueSerial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)
	{
		BlueSerial_SendByte(Number / BlueSerial_Pow(10, Length - i - 1) % 10 + '0');
	}
}

void BlueSerial_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	BlueSerial_SendString(String);
}

/**
  * @brief  接收完成回调:逐字节解析 [ ... ] 数据包
  * @note   由 main.c 的 HAL_UART_RxCpltCallback() 在USART2收到1字节后调用，
  *         逻辑与原标准库 USART2_IRQHandler 完全一致。
  */
void BlueSerial_RxCpltCallback(void)
{
	static uint8_t RxState = 0;
	static uint8_t pRxPacket = 0;
	uint8_t RxData = BlueSerial_RxData;

	if (RxState == 0)
	{
		if (RxData == '[' && BlueSerial_RxFlag == 0)
		{
			RxState = 1;
			pRxPacket = 0;
		}
	}
	else if (RxState == 1)
	{
		if (RxData == ']')
		{
			RxState = 0;
			BlueSerial_RxPacket[pRxPacket] = '\0';
			BlueSerial_RxFlag = 1;
		}
		else
		{
			BlueSerial_RxPacket[pRxPacket] = RxData;
			pRxPacket ++;
		}
	}

	HAL_UART_Receive_IT(&huart2, &BlueSerial_RxData, 1);	//重新装载接收
}

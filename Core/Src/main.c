/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "Key.h"
#include "MPU6050.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include "BlueSerial.h"
#include "PID.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/*MPU6050测试*/
int16_t AX,AY,AZ,GX,GY,GZ;
uint8_t TimerErrorFlag;
uint16_t TimerCount;
uint8_t KeyNum,RunFlag;		//小车启动停止标志位

float AngleAcc;					//加速度计测定角度
float AngleGyro;				//陀螺仪测定角度
float Angle;						//滤波后的稳定角度

int16_t LeftPWM,RightPWM;		//左右电机PWM
int16_t AvePWM,DifPWM;			//平均PWM,差分PWM

float LeftSpeed,RightSpeed;		//左右电机速度
float AveSpeed,DifSpeed;				//平均速度,差分速度

PID_t AnglePID={				//角度环PID(离散形式)
		.Kp=3.41,
		.Ki=0.17,
		.Kd=3.22,
	
		.OutMax=100,
		.OutMin=-100,
	
		.OutOffset=0,
	
		.ErrorIntMax=600,
		.ErrorIntMin=-600,
};

PID_t SpeedPID={				//速度环PID(离散形式)
		.Kp=2.02,
		.Ki=0.07,
		.Kd=0,
	
		.OutMax=20,
		.OutMin=-20,
	
		.ErrorIntMax=150,
		.ErrorIntMin=-150,
};

PID_t TurnPID={				//角度环PID(离散形式)
		.Kp=4.38,
		.Ki=3.82,
		.Kd=0,
	
		.OutMax=50,
		.OutMin=-50,
	
		.ErrorIntMax=20,
		.ErrorIntMin=-20,
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	OLED_Init();
	LED_Init();
	Timer_Init();
	Key_Init();
	MPU6050_Init();
	Motor_Init();
	Encoder_Init();
	Serial_Init();
	BlueSerial_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
			if(RunFlag==1) {LED_ON();} else {LED_OFF();}
			KeyNum=Key_GetNum();
			if(KeyNum==1)
			{
					if(RunFlag==0)
					{
							PID_Init(&AnglePID);
							PID_Init(&SpeedPID);
							PID_Init(&TurnPID);
							RunFlag=1;
					}
					else
					{
							RunFlag=0;
					}
					
			}
			
			OLED_Clear();
			OLED_Printf(0,0,OLED_6X8,"  Angle");
			OLED_Printf(0,8,OLED_6X8,"P:%05.2f",AnglePID.Kp);
			OLED_Printf(0,16,OLED_6X8,"I:%05.2f",AnglePID.Ki);
			OLED_Printf(0,24,OLED_6X8,"D:%05.2f",AnglePID.Kd);
			OLED_Printf(0,32,OLED_6X8,"T:%+05.1f",AnglePID.Target);
			OLED_Printf(0,40,OLED_6X8,"A:%+05.1f",Angle);
			OLED_Printf(0,48,OLED_6X8,"O:%+05.0f",AnglePID.Out);
			OLED_Printf(0,56, OLED_6X8, "GY:%+05d", GY);
			OLED_Printf(56,56, OLED_6X8, "Offset:%02.0f",AnglePID.OutOffset);
			
			OLED_Printf(50,0,OLED_6X8,"Speed");
			OLED_Printf(50,8,OLED_6X8,"%05.2f",SpeedPID.Kp);
			OLED_Printf(50,16,OLED_6X8,"%05.2f",SpeedPID.Ki);
			OLED_Printf(50,24,OLED_6X8,"%05.2f",SpeedPID.Kd);
			OLED_Printf(50,32,OLED_6X8,"%+05.1f",SpeedPID.Target);
			OLED_Printf(50,40,OLED_6X8,"%+05.1f",AveSpeed);
			OLED_Printf(50,48,OLED_6X8,"%+05.0f",SpeedPID.Out);
			
			OLED_Printf(88,0,OLED_6X8,"Turn");
			OLED_Printf(88,8,OLED_6X8,"%05.2f",TurnPID.Kp);
			OLED_Printf(88,16,OLED_6X8,"%05.2f",TurnPID.Ki);
			OLED_Printf(88,24,OLED_6X8,"%05.2f",TurnPID.Kd);
			OLED_Printf(88,32,OLED_6X8,"%+05.1f",TurnPID.Target);
			OLED_Printf(88,40,OLED_6X8,"%+05.1f",DifSpeed);
			OLED_Printf(88,48,OLED_6X8,"%+05.0f",TurnPID.Out);
			OLED_Update();
			
			if (BlueSerial_RxFlag == 1)
			{
					char *Tag = strtok(BlueSerial_RxPacket, ",");
					if (strcmp(Tag, "key") == 0)//收到按键数据包
					{
							char *Name = strtok(NULL, ",");
							char *Action = strtok(NULL, ",");
	
					}
					else if (strcmp(Tag, "slider") == 0)//滑杆数据包
					{
							char *Name = strtok(NULL, ",");
							char *Value = strtok(NULL, ",");
							
							if(strcmp(Name,"AngleKp")==0)
							{
									AnglePID.Kp=atof(Value);
							}
							else if(strcmp(Name,"AngleKi")==0)
							{
									AnglePID.Ki=atof(Value);
							}
							else if(strcmp(Name,"AngleKd")==0)
							{
									AnglePID.Kd=atof(Value);
							}
							else if(strcmp(Name,"TurnKp")==0)
							{
									TurnPID.Kp=atof(Value);
							}
							else if(strcmp(Name,"TurnKi")==0)
							{
									TurnPID.Ki=atof(Value);
							}
							else if(strcmp(Name,"TurnKd")==0)
							{
									TurnPID.Kd=atof(Value);
							}
							else if(strcmp(Name,"Offset")==0)
							{
									AnglePID.OutOffset=atof(Value);
							}
					}
					else if (strcmp(Tag, "joystick") == 0)//摇杆数据包
					{
							int8_t LH = atoi(strtok(NULL, ","));
							int8_t LV = atoi(strtok(NULL, ","));
							int8_t RH = atoi(strtok(NULL, ","));
							int8_t RV = atoi(strtok(NULL, ","));
	
							SpeedPID.Target=LV/25.0;		//-4~+4转/秒
							TurnPID.Target=RH/25.0;
					}
	
					BlueSerial_RxFlag = 0;
			}

			
			BlueSerial_Printf("[plot,%f,%f,%f]",AnglePID.ErrorInt,SpeedPID.ErrorInt,TurnPID.ErrorInt);
			//BlueSerial_Printf("%f,%f,%f\n",AnglePID.ErrorInt,SpeedPID.ErrorInt,TurnPID.ErrorInt);//VOFA+波形图显示

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/**
  * @brief  定时器更新中断回调: TIM1每1ms进入一次，承载按键扫描与平衡控制
  * @note   对应原标准库中的 TIM1_UP_IRQHandler
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM1)
	{
		static uint16_t Count0,Count1;

		Key_Tick();//按键

		Count0++;
		if(Count0>=10)		//PID调控周期(10ms)
		{
				Count0=0;
				MPU6050_GetData(&AX,&AY,&AZ,&GX,&GY,&GZ);

				GY+=3;																					//抑制零漂
			
				AngleAcc=-atan2(AX,AZ)/3.14159*180;						//俯仰角(弧度转角度)
				AngleAcc+=8.1;
			
				AngleGyro=Angle+GY/32768.0*2000*0.01;     		  //俯仰角积分公式
			
				float Alpha=0.01;																//滤波系数(陀螺仪为主,加速度计为辅)
				Angle=Alpha*AngleAcc+(1-Alpha)*AngleGyro;				//互补滤波公式
	
				if(Angle>50||Angle<-50)		//俯仰角过高停止
				{
						RunFlag=0;
				}
			
				if(RunFlag==1)//小车启动
				{
						//角度环PID调控代码
						AnglePID.Actual=Angle;
						PID_Update(&AnglePID);
						AvePWM=-AnglePID.Out;		//输出控制量
				
						LeftPWM=AvePWM+DifPWM/2;
						RightPWM=AvePWM-DifPWM/2;
				
						//PWM判断
						if(LeftPWM>100) {LeftPWM=100;} else if(LeftPWM<-100) {LeftPWM=-100;}
						if(RightPWM>100) {RightPWM=100;} else if(RightPWM<-100) {RightPWM=-100;}
				
						Motor_SetPWM(1,LeftPWM);
						Motor_SetPWM(2,RightPWM);
				}
				else
				{
						//左右电机停止
						Motor_SetPWM(1,0);
						Motor_SetPWM(2,0);
				}
		}
		
		Count1++;
		if(Count1>=50)		//编码器读取电机速度(50ms)
		{
				Count1=0;
				LeftSpeed=Encoder_Get(1)/44.0/0.05/9.27666;
				RightSpeed=Encoder_Get(2)/44.0/0.05/9.27666;
			
				AveSpeed=(LeftSpeed+RightSpeed)/2.0;
				DifSpeed=LeftSpeed-RightSpeed;
			
				if(RunFlag==1)
				{
						//速度环PID调控代码
						SpeedPID.Actual=AveSpeed;
						PID_Update(&SpeedPID);
						AnglePID.Target=SpeedPID.Out;
					
						//转向环PID调控代码
						TurnPID.Actual=DifSpeed;
						PID_Update(&TurnPID);
						DifPWM=TurnPID.Out;
				}
		}

		TimerErrorFlag=1;
		TimerCount=__HAL_TIM_GET_COUNTER(&htim1);
	}
}

/**
  * @brief  串口接收完成回调: 按外设分发到对应驱动
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
	{
		Serial_RxCpltCallback();
	}
	else if (huart->Instance == USART2)
	{
		BlueSerial_RxCpltCallback();
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

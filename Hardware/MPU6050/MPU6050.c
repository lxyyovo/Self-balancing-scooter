#include "main.h"
#include "MyI2C.h"
#include "MPU6050_Reg.h"
#include "MPU6050.h"

#define MPU6050_ADDRESS		0xD0

void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(Data);
	MyI2C_ReceiveAck();
	MyI2C_Stop();
}

uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01);
	MyI2C_ReceiveAck();
	Data = MyI2C_ReceiveByte();
	MyI2C_SendAck(1);
	MyI2C_Stop();
	
	return Data;
}

uint8_t MPU6050_ReadRegs(uint8_t RegAddress,uint8_t*DataArray,uint8_t Count) //读取多字节
{
	uint8_t i;
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01);
	MyI2C_ReceiveAck();
	for(i=0;i<Count;i++)
	{
			DataArray[i] = MyI2C_ReceiveByte();
			if(i<Count-1)
			{
					MyI2C_SendAck(0);		//应答
			}
			else
			{
					MyI2C_SendAck(1);		//非应答
			}
	}
	
	MyI2C_Stop();
	return 0;
	
}

void MPU6050_Init(void)
{
	MyI2C_Init();
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x07);			// 8000/(1+7)=1000Hz=1ms
	MPU6050_WriteReg(MPU6050_CONFIG, 0x00);				//不使用低通滤波器(降低延迟)
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);
}

uint8_t MPU6050_GetID(void)
{
	return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
		uint8_t Data[14];
	
		MPU6050_ReadRegs(MPU6050_ACCEL_XOUT_H,Data,14);
		*AccX = (Data[0] << 8) | Data[1];
		*AccY = (Data[2] << 8) | Data[3];
		*AccZ = (Data[4] << 8) | Data[5];
	
		*GyroX = (Data[8] << 8) | Data[9];
		*GyroY = (Data[10] << 8) | Data[11];
		*GyroZ = (Data[12] << 8) | Data[13];
}

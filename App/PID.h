#ifndef __PID_H
#define __PID_H

typedef struct {
	float Target;
	float Actual;
	float Actual1;	//上次实际值
	float Out;
	
	float Kp;
	float Ki;
	float Kd;
	
	float Error0;
	float Error1;
	float ErrorInt;
	float ErrorIntMax;	//限幅值
	float ErrorIntMin;
	
	float OutMax;
	float OutMin;
	
	float OutOffset;//输出偏移值
} PID_t;

void PID_Init(PID_t *p);
void PID_Update(PID_t *p);


#endif

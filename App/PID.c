#include "main.h"
#include "PID.h"

void PID_Init(PID_t *p)
{
	p->Actual=0;
	p->Actual1=0;
	p->Target=0;
	p->Out=0;
	p->Error0=0;
	p->Error1=0;
	p->ErrorInt=0;
}

void PID_Update(PID_t *p)
{
	p->Error1 = p->Error0;
	p->Error0 = p->Target - p->Actual;
	
	if (p->Ki != 0)
	{
		p->ErrorInt += p->Error0;
		if(p->ErrorInt>p->ErrorIntMax) {p->ErrorInt=p->ErrorIntMax;}	//积分限幅
		if(p->ErrorInt<p->ErrorIntMin) {p->ErrorInt=p->ErrorIntMin;}
	}
	else
	{
		p->ErrorInt = 0;
	}
	
	p->Out = p->Kp * p->Error0
		   + p->Ki * p->ErrorInt
//		 + p->Kd * (p->Error0 - p->Error1);
			 - p->Kd * (p->Actual - p->Actual1);	//微分先行
	
	if(p->Out>0) {p->Out+=p->OutOffset;}	//输出偏移
	if(p->Out<0) {p->Out-=p->OutOffset;}
	
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}
	
	p->Actual1=p->Actual;
}

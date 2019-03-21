#pragma once

#include <list>
#include <Windows.h>


#define BP_INIT 1       //��ʼ�ϵ�
#define BP_CODE 2       //�����еĶϵ�
#define BP_USER 3       //�û����õĶϵ�
#define BP_STEP_OVER 4  //StepOverʹ�õĶϵ�
#define BP_STEP_OUT 5   //StepOutʹ�õĶϵ�


typedef struct {

	DWORD address;    //�ϵ��ַ
	BYTE content;     //ԭָ���һ���ֽ�

} BREAK_POINT;



void InitializeBreakPointHelper();

int GetBreakPointType(DWORD address);

BOOL SetUserBreakPointAt(DWORD address);
BOOL RecoverUserBreakPoint(DWORD address);
BOOL CancelUserBreakPoint(DWORD address);

void SaveResetUserBreakPoint(DWORD address);
void ResetUserBreakPoint();
BOOL NeedResetBreakPoint();

const std::list<BREAK_POINT>& GetUserBreakPoints();

void SetTrapFlag();

void SetStepOverBreakPointAt(DWORD address);
void CancelStepOverBreakPoint();

void SetStepOutBreakPointAt(DWORD address);
void CancelStepOutBreakPoint();
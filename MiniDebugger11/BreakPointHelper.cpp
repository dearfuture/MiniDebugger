#include <list>
#include <iostream>
#include "BreakPointHelper.h"
#include "DebugSession.h"


BYTE SetBreakPointAt(DWORD address);
void RecoverBreakPoint(const BREAK_POINT* pBreakPoint);



static BREAK_POINT g_stepOverBp;
static BREAK_POINT g_stepOutBp;

static std::list<BREAK_POINT> g_userBpList;

static BOOL g_isInitBpOccured = FALSE;

static DWORD g_resetUserBpAddress;




//��ʼ���ϵ����
void InitializeBreakPointHelper() {

	g_userBpList.clear();
	g_isInitBpOccured = FALSE;

	g_stepOverBp.address = 0;
	g_stepOverBp.content = 0;

	g_stepOutBp.address = 0;
	g_stepOutBp.content = 0;

	g_resetUserBpAddress = 0;
}



//��ȡ�ϵ������
int GetBreakPointType(DWORD address) {

	//�����ʼ�ϵ�δ��������ض��ǳ�ʼ�ϵ�
	if (g_isInitBpOccured == FALSE) {
		g_isInitBpOccured = TRUE;
		return BP_INIT;
	}

	//����Ƿ�StepOver�ϵ�
	//ע��������Ҫ�����û��ϵ�ļ�飬�������
	//�û��ϵ���StepOver�ϵ㶼����ͬһ����ַ�Ļ���
	//�ᷢ�����ҡ�
	if (g_stepOverBp.address == address) {
		return BP_STEP_OVER;
	}

	//����Ƿ�StepOut�ϵ�
	if (g_stepOutBp.address == address) {
		return BP_STEP_OUT;
	}

	//���û����õĶϵ���Ѱ��
	for (std::list<BREAK_POINT>::const_iterator it = g_userBpList.cbegin();
		 it != g_userBpList.cend();
		 ++it) {

		if (it->address == address) {
			return BP_USER;
		}
	}

	//���������������㣬���Ǳ����Խ����еĶϵ�
	return BP_CODE;
}



//��ָ���ĵ�ַ���öϵ�
BOOL SetUserBreakPointAt(DWORD address) {
	
	//����Ƿ��Ѵ��ڸöϵ�
	for (std::list<BREAK_POINT>::const_iterator it = g_userBpList.cbegin();
		 it != g_userBpList.cend();
		 ++it) {

		if (it->address == address) {

			std::wcout << TEXT("Break point has existed.") << std::endl;
			return FALSE;
		}
	}

	BREAK_POINT newBp;
	newBp.address = address;
	newBp.content = SetBreakPointAt(newBp.address);

	g_userBpList.push_back(newBp);

	return TRUE;
}



//����StepOverʹ�õĶϵ�
void SetStepOverBreakPointAt(DWORD address) {

	g_stepOverBp.address = address;
	g_stepOverBp.content = SetBreakPointAt(address);
}



//ȡ��ָ����ַ�Ķϵ�
BOOL CancelUserBreakPoint(DWORD address) {

	for (std::list<BREAK_POINT>::const_iterator it = g_userBpList.cbegin();
		 it != g_userBpList.cend();
		 ++it) {

		if (it->address == address) {

			RecoverBreakPoint(&*it);
			g_userBpList.erase(it);

			return TRUE;
		}
	}

	std::wcout << TEXT("Break point does not exist.") << std::endl;

	return FALSE;
}



//ȡ��StepOverʹ�õĶϵ�
void CancelStepOverBreakPoint() {

	if (g_stepOverBp.address != 0) {

		RecoverBreakPoint(&g_stepOverBp);

		g_stepOverBp.address = 0;
		g_stepOverBp.content = 0;
	}
}



//�ָ��ϵ�����ָ���һ���ֽڵ�����
BOOL RecoverUserBreakPoint(DWORD address) {
	
	for (std::list<BREAK_POINT>::const_iterator it = g_userBpList.cbegin();
		 it != g_userBpList.cend();
		 ++it) {

		if (it->address == address) {

			RecoverBreakPoint(&*it);
			return TRUE;
		}
	}

	std::wcout << TEXT("Break point does not exist") << std::endl;

	return FALSE;
}



//���������������õ��û��ϵ��ַ
void SaveResetUserBreakPoint(DWORD address) {

	g_resetUserBpAddress = address;
}



//���������ѱ�����û��ϵ�
void ResetUserBreakPoint() {

	for (std::list<BREAK_POINT>::iterator it = g_userBpList.begin(); 
		 it != g_userBpList.end();
		 ++it) {

		if (it->address == g_resetUserBpAddress) {

			SetBreakPointAt(it->address);

			g_resetUserBpAddress = 0;
		}
	}
}



//�����Ƿ���Ҫ���������û��ϵ��ָʾֵ
BOOL NeedResetBreakPoint() {

	return g_resetUserBpAddress != 0;
}



//�����û����öϵ������
const std::list<BREAK_POINT>& GetUserBreakPoints() {

	return g_userBpList;
}



//����TF��־λ
void SetTrapFlag() {

	CONTEXT context;

	GetDebuggeeContext(&context);

	context.EFlags |= 0x100;

	SetDebuggeeContext(&context);
}



//�����������е��û��ϵ�
void ResetUserBreakPoints() {

	for (std::list<BREAK_POINT>::iterator iterator = g_userBpList.begin();
		 iterator != g_userBpList.end();
		 ++iterator) {

		BYTE byte;
		ReadDebuggeeMemory(iterator->address, 1, &byte);

		if (byte != 0xCC) {
			SetBreakPointAt(iterator->address);
		}
	}
}



//��ָ����ַ���öϵ㣬����ֵΪԭָ��ĵ�һ���ֽ�
BYTE SetBreakPointAt(DWORD address) {

	BYTE byte;
	ReadDebuggeeMemory(address, 1, &byte);
	
	BYTE intInst = 0xCC;
	WriteDebuggeeMemory(address, 1, &intInst);

	return byte;
}



//�ָ��ϵ�����ָ��ĵ�һ���ֽ�
void RecoverBreakPoint(const BREAK_POINT* pBreakPoint) {

	WriteDebuggeeMemory(pBreakPoint->address, 1, &pBreakPoint->content);
}



//����StepOutʹ�õĶϵ�
void SetStepOutBreakPointAt(DWORD address) {

	g_stepOutBp.address = address;
	g_stepOutBp.content = SetBreakPointAt(address);
}



//ȡ��StepOutʹ�õĶϵ�
void CancelStepOutBreakPoint() {

	if (g_stepOutBp.address != 0) {

		RecoverBreakPoint(&g_stepOutBp);

		g_stepOutBp.address = 0;
		g_stepOutBp.content = 0;
	}
}


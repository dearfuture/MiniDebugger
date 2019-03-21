#include <Windows.h>
#include <DbgHelp.h>
#include "DebugSession.h"
#include "SingleStepHelper.h"
#include "BreakPointHelper.h"



static BOOL GetCurrentLineInfo(LINE_INFO* pLineInfo);



//�Ƿ�����StepOver��ָʾֵ
static BOOL g_isBeingStepOver;

//�Ƿ�����StepOut��ָʾֵ 
static BOOL g_isBeingStepOut;

//�����к���Ϣ�ı���
static LINE_INFO g_lineInfo;

//�Ƿ���������ָ��ִ�е�ָʾֵ
static BOOL g_isBeingSingleInstruction;



//��ʼ������ִ�����
void InitializeSingleStepHelper() {

	g_lineInfo.fileName[0] = 0;
	g_lineInfo.lineNumber = 0;

	g_isBeingSingleInstruction = FALSE;
	g_isBeingStepOut = FALSE;
	g_isBeingStepOver = FALSE;
}



////��ʼ����ִ��
//void BeginSingleStep(BOOL stepOver) {
//
//	g_isBeingStepOver = stepOver;
//
//	g_isBeingSingleInstruction = FALSE;
//
//	GetCurrentLineInfo(&g_lineInfo);
//}
//
//
//
//
////��������ִ��
//void EndSingleStep() {
//
//	if (g_isBeingStepOver == TRUE) {
//		CancelStepOverBreakPoint();
//	}
//
//	g_isBeingStepOver = FALSE;
//}



//���浱ǰ�е���Ϣ
void SaveCurrentLineInfo() {

	GetCurrentLineInfo(&g_lineInfo);
}



//��ȡ����Ϣ�Ƿ��Ѹı��ָʾֵ
BOOL IsLineChanged() {

	LINE_INFO lineInfo;

	//�����ǰָ��û�ж�Ӧ������Ϣ������Ϊ����ͬһ����
	if (GetCurrentLineInfo(&lineInfo) == FALSE) {
		return FALSE;	
	}

	//����ж�Ӧ������Ϣ����Ƚ��Ƿ�����ͬһ��
	if (lineInfo.lineNumber == g_lineInfo.lineNumber && wcscmp(lineInfo.fileName, g_lineInfo.fileName) == 0) {
		return FALSE;
	}

	return TRUE;
}



//��ȡ��ǰEIP��ָ��ַ��Ӧ���к���Ϣ������TRUE��
//�����ǰ��ַû�е��Է��ţ���û�ж�Ӧ���к���Ϣ��
//pLineInfo->fileName[0]��pLineInfo->lineNumber��Ϊ0��
//����FALSE��
BOOL GetCurrentLineInfo(LINE_INFO* pLineInfo) {

	CONTEXT context;
	GetDebuggeeContext(&context);

	DWORD displacement;
	IMAGEHLP_LINE64 lineInfo = { 0 };
	lineInfo.SizeOfStruct = sizeof(lineInfo);

	if (SymGetLineFromAddr64(
		GetDebuggeeHandle(),
		context.Eip,
		&displacement,
		&lineInfo) == TRUE) {
			
		wcscpy_s(pLineInfo->fileName, lineInfo.FileName);
		pLineInfo->lineNumber = lineInfo.LineNumber;

		return TRUE;
	}
	else {

		pLineInfo->fileName[0] = 0;
		pLineInfo->lineNumber = 0;

		return FALSE;
	}
}



//���ָ����ַ����ָ����CALL������CALLָ��ĳ���
//���򷵻�0
//�жϵķ����ο��ˡ�CALLָ���ж�����д����һ��
// http://blog.ftofficer.com/2010/04/n-forms-of-call-instructions
int IsCallInstruction(DWORD address) {

	BYTE instruction[10];

	ReadDebuggeeMemory(
		address,
		sizeof(instruction) / sizeof(BYTE),
		instruction);

	switch (instruction[0]) {

		case 0xE8:
			return 5;

		case 0x9A:
			return 7;

		case 0xFF:
			switch (instruction[1]) {

				case 0x10:
				case 0x11:
				case 0x12:
				case 0x13:
				case 0x16:
				case 0x17:
				case 0xD0:
				case 0xD1:
				case 0xD2:
				case 0xD3:
				case 0xD4:
				case 0xD5:
				case 0xD6:
				case 0xD7:
					return 2;

				case 0x14:
				case 0x50:
				case 0x51:
				case 0x52:
				case 0x53:
				case 0x54:
				case 0x55:
				case 0x56:
				case 0x57:
					return 3;

				case 0x15:
				case 0x90:
				case 0x91:
				case 0x92:
				case 0x93:
				case 0x95:
				case 0x96:
				case 0x97:
					return 6;

				case 0x94:
					return 7;
			}

		default:
			return 0;
	}
}


//�����Ƿ�����StepOver
void SetStepOver(BOOL value) {
	g_isBeingStepOver = value;
}


//��ȡ�Ƿ�����StepOver��ָʾֵ
BOOL IsBeingStepOver() {
	return g_isBeingStepOver;
}



//�����Ƿ���������ָ��ִ��
void SetSingleInstruction(BOOL value) {

	g_isBeingSingleInstruction = value;
}



//��ȡ�Ƿ���������ָ��ִ�е�ָʾֵ
//�����������ָ��ִ�еĻ���
//�������е���������Ķϵ���û����õĶϵ�
//�Լ�����TFλ
BOOL IsBeingSingleInstruction() {

	return g_isBeingSingleInstruction;
}


//�����Ƿ�����StepOut
void SetStepOut(BOOL value) {
	g_isBeingStepOut = value;
}

//��ȡ�Ƿ�����StepOut��ָʾֵ
BOOL IsBeingStepOut() {
	return g_isBeingStepOut;
}


//��ȡ��ǰ������retָ��ĵ�ַ
DWORD GetRetInstructionAddress() {

	CONTEXT context;
	GetDebuggeeContext(&context);

	DWORD64 displacement;
	SYMBOL_INFO symBol = { 0 };
	symBol.SizeOfStruct = sizeof(SYMBOL_INFO);

	if (SymFromAddr(
		GetDebuggeeHandle(),
		context.Eip, 
		&displacement,
		&symBol) == FALSE) {
			
		return 0;
	}

	DWORD endAddress = DWORD(symBol.Address + symBol.Size);
	int retLen;

	//����Ƿ������ֽڵ�RETָ��
	retLen = IsRetInstruction(endAddress - 3);

	if (retLen == 3) {
		return endAddress - retLen;
	}

	//����Ƿ�һ���ֽڵ�RETָ��
	retLen = IsRetInstruction(endAddress - 1);

	if (retLen == 1) {
		return endAddress - retLen;
	}

	return 0;
}



//���ָ����ַ����ָ���Ƿ�RETָ��
//����ǣ�����RETָ��ĳ���
//���򷵻�0
int IsRetInstruction(DWORD address) {

	BYTE byte;
	ReadDebuggeeMemory(address, 1, &byte);

	if (byte == 0xC3 || byte == 0xCB) {
		return 1;
	}

	if (byte == 0xC2 || byte == 0xCA) {
		return 3;
	}

	return 0;
}
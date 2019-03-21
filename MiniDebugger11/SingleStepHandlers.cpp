#include <iostream>
#include "CmdHandlers.h"
#include "DebugSession.h"
#include "SingleStepHelper.h"
#include "BreakPointHelper.h"



void OnStepIn(const Command& cmd) {

	CHECK_DEBUGGEE;

	SaveCurrentLineInfo();

	SetTrapFlag();

	SetSingleInstruction(TRUE);

	ContinueDebugSession();
}



void OnStepOver(const Command& cmd) {

	CHECK_DEBUGGEE;

	SaveCurrentLineInfo();

	SetStepOver(TRUE);

	//��鵱ǰָ���Ƿ�CALLָ��
	CONTEXT context;
	GetDebuggeeContext(&context);

	int callLen = IsCallInstruction(context.Eip);

	//����ǣ�������һ��ָ����öϵ�
	if (callLen != 0) {

		SetStepOverBreakPointAt(context.Eip + callLen);
		SetSingleInstruction(FALSE);
	}
	//������ǣ�������TFλ
	else {

		SetTrapFlag();
		SetSingleInstruction(TRUE);
	}

	ContinueDebugSession();
}



void OnStepOut(const Command& cmd) {

	CHECK_DEBUGGEE;

	SetStepOut(TRUE);

	SetSingleInstruction(FALSE);

	DWORD retAddress = GetRetInstructionAddress();

	if (retAddress != 0) {

		SetStepOutBreakPointAt(retAddress);
	}

	ContinueDebugSession();
}
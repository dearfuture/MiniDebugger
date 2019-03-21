#include <iostream>
#include <string>
#include "CmdHandlers.h"
#include "DebugSession.h"
#include "SingleStepHelper.h"



//���������Խ���ִ������Ĵ�������
//�����ʽΪ g [c]
//���ʡ����c����������ζ�ŵ�����δ�����쳣��
//������ζ�ŵ������������쳣��
void OnGo(const Command& cmd) {

	CHECK_DEBUGGEE;

	SetSingleInstruction(FALSE);

	if (cmd.size() < 2) {

		HandledException(FALSE);
		ContinueDebugSession();
		return;
	}

	if (cmd[1] == TEXT("c")) {

		HandledException(TRUE);
		ContinueDebugSession();
	}
	else {

		std::wcout << TEXT("Invalid params.") << std::endl;
	}
}
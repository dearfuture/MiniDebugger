#include <iostream>
#include <string>
#include "CmdHandlers.h"
#include "DebugSession.h"
#include "BreakPointHelper.h"
#include "HelperFunctions.h"


void DisplayBreakPoints();


//�ϵ�����Ĵ�������
//�����ʽ�� b [address [d]]
//addressʹ��16������
//���ʡ����address������ʾ�����õĶϵ�
//���ʡ���˲���d�������öϵ�
//û��ʡ����ɾ���ϵ�
void OnBreakPoint(const Command& cmd) {

	CHECK_DEBUGGEE;

	if (cmd.size() < 2) {

		DisplayBreakPoints();
		return;
	}

	int address = 0;
	BOOL isDel = FALSE;

	address = wcstoul(cmd[1].c_str(), NULL, 16);

	if (cmd.size() >= 3) {

		if (cmd[2] == TEXT("d")) {
			isDel = TRUE;
		}
		else {
			std::wcout << TEXT("Invalid params.") << std::endl;
			return;
		}
	}

	if (isDel == FALSE) {

		if (SetUserBreakPointAt(address) == TRUE) {
			std::wcout << TEXT("Set break point succeeded.") << std::endl;
		}
		else {
			std::wcout << TEXT("Set break point failed.") << std::endl;
		}
	}
	else {

		if (CancelUserBreakPoint(address) == TRUE) {
			std::wcout << TEXT("Cancel break point succeeded.") << std::endl;
		}
		else {
			std::wcout << TEXT("Cancel break point failed.") << std::endl;
		}
	}
}





//��ʾ�����õĶϵ�
void DisplayBreakPoints() {

	const std::list<BREAK_POINT> bpList = GetUserBreakPoints();

	if (bpList.size() == 0) {
		std::wcout << TEXT("No break point.") << std::endl;
		return;
	}

	for (std::list<BREAK_POINT>::const_iterator it = bpList.cbegin();
		 it != bpList.cend();
		 ++it) {

		PrintHex(it->address, FALSE);
		std::wcout << std::endl;
	}
}
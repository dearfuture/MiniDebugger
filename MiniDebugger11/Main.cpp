#include <Windows.h>
#include <DbgHelp.h>
#include <iostream>
#include <string>
#include <sstream>
#include "Command.h"
#include "CmdHandlers.h"





void SeparateCommand(const std::wstring& cmdLine, Command& cmd);
BOOL DispatchCommand(const Command& cmd);
void ClearCommand(Command& cmd);



int wmain(int argc, wchar_t** argv) {

	std::wcout.imbue(std::locale("chs", std::locale::ctype));

	std::wcout << TEXT("MiniDebugger by Zplutor") << std::endl << std::endl;

	Command cmd;
	std::wstring cmdLine;

	while (true) {
		
		std::wcout << std::endl << TEXT("> ");
		std::getline(std::wcin, cmdLine);
		
		SeparateCommand(cmdLine, cmd);

		if (DispatchCommand(cmd) == FALSE) {
			break;
		}

		ClearCommand(cmd);
	}

	return 0;
}



//�Կհ��ַ���Ϊ�ָ������������в�ֳɼ������֣���ӵ�Command�С�
//����˫�����ڵ�������Ϊһ���֡�
//�÷���δ����ȫ���ԣ�������BU����
void SeparateCommand(const std::wstring& cmdLine, Command& cmd) {

	std::wistringstream cmdStream(cmdLine);

	std::wstring partialArg;
	std::wstring fullArg;
	BOOL isFullArg = TRUE;

	cmdStream >> partialArg;
	cmd.push_back(partialArg);

	while (cmdStream >> partialArg) {

		if (partialArg.at(0) == TEXT('\"')) {

			isFullArg = FALSE;

			if (partialArg.at(partialArg.length() - 1) != TEXT('\"')) {

				fullArg.append(partialArg);
				fullArg.append(L" ");
				continue;
			}
		}

		if (isFullArg == FALSE) {
			
			if (partialArg.at(partialArg.length() - 1) == L'\"') {

				isFullArg = TRUE;
				fullArg.append(partialArg);
				fullArg.erase(0, 1);
				fullArg.erase(fullArg.length() - 1, 1);
				cmd.push_back(fullArg);
			}
			else {

				fullArg.append(partialArg);
				fullArg.append(L" ");
			}

			continue;
		}

		cmd.push_back(partialArg);
	}
}



//��������ĵ�һ�����ֵ�����Ӧ�Ĵ�������
//�������Ϊ��q�����򷵻�FALSE�����ߵ���
//���˳������������������TRUE��
BOOL DispatchCommand(const Command& cmd) {

	static struct {
		LPCTSTR cmd;
		void (*handler)(const Command&);
	} cmdMap[] = {
	
		{ TEXT("s"), OnStartDebug},
		{ TEXT("g"), OnGo },
		{ TEXT("d"), OnDump },
		{ TEXT("l"), OnShowSourceLines },
		{ TEXT("b"), OnBreakPoint },
		{ TEXT("r"), OnShowRegisters },
		{ TEXT("in"), OnStepIn },
		{ TEXT("over"), OnStepOver },
		{ TEXT("out"), OnStepOut },
		{ TEXT("lv"), OnShowLocalVariables },
		{ TEXT("gv"), OnShowGlobalVariables },
		{ TEXT("f"), OnFormatMemory },
		{ TEXT("w"), OnShowStackTrack },
		{ TEXT("t"), OnStopDebug },
		{ NULL, NULL },
	};

	if (cmd.size() == 0) {
		return TRUE;
	}
	else if (cmd[0] == TEXT("q")) {
		return FALSE;
	}

	for (int index = 0; cmdMap[index].cmd != NULL; ++index) {

		if (cmd[0] == cmdMap[index].cmd) {

			cmdMap[index].handler(cmd);
			return TRUE; 
		}
	}

	std::wcout << TEXT("Invalid command.") << std::endl;
	return TRUE;
}



//�������ṹ��
void ClearCommand(Command& cmd) {

	cmd.clear();
}
#include <iostream>
#include "DebugSession.h"
#include "CmdHandlers.h"



//���������Խ�������Ĵ�������
//�����ʽΪ s path
//���·�����пո���Ӧ����˫���Ž�����ס��
void OnStartDebug(const Command& cmd) {

	if (cmd.size() < 2) {

		std::wcout << TEXT("Lack path.") << std::endl;
		return;
	}

	StartDebugSession(cmd[1].c_str());
}
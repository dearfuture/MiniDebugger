#include <iostream>
#include "DebugSession.h"
#include "CmdHandlers.h"



//���������Խ�������Ĵ�������
//�����ʽΪ t
void OnStopDebug(const Command& cmd) {

	CHECK_DEBUGGEE;

	StopDebugSeesion();
}
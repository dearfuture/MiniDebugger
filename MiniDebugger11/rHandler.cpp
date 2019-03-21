#include <iostream>
#include <iomanip>
#include "DebugSession.h"
#include "CmdHandlers.h"
#include "HelperFunctions.h"



//��ʾ�Ĵ���ֵ����Ĵ�������
//�����ʽΪ r
void OnShowRegisters(const Command& cmd) {

	CHECK_DEBUGGEE;

	CONTEXT context;

	if (GetDebuggeeContext(&context) == FALSE) {
		std::wcout << TEXT("Show registers failed.") << std::endl;
		return;
	}

	//���EAX
	std::wcout << TEXT("EAX = ");
	PrintHex(context.Eax, FALSE);

	//���EBX
	std::wcout << TEXT("    EBX = ");
	PrintHex(context.Ebx, FALSE);
	std::wcout << std::endl;

	//���ECX
	std::wcout << TEXT("ECX = ");
	PrintHex(context.Ecx, FALSE);

	//���EDX
	std::wcout << TEXT("    EDX = ");
	PrintHex(context.Edx, FALSE);
	std::wcout << std::endl;
	
	//���ESI
	std::wcout << TEXT("ESI = ");
	PrintHex(context.Esi, FALSE);

	//���EDI
	std::wcout << TEXT("    EDI = ");
	PrintHex(context.Edi, FALSE);
	std::wcout << std::endl;

	//���EBP
	std::wcout << TEXT("EBP = ");
	PrintHex(context.Ebp, FALSE);

	//���ESP
	std::wcout << TEXT("    ESP = "); 
	PrintHex(context.Esp, FALSE); 
	std::wcout << std::endl;
			
	//���EIP
	std::wcout << TEXT("EIP = ");
	PrintHex(context.Eip, FALSE);
	std::wcout << TEXT("    ");

	//CF �ڵ�0λ
	if ((context.EFlags & 0x1) != 0) {

		std::wcout << L"CF  ";
	}

	//PF �ڵ�2λ
	if ((context.EFlags & 0x4) != 0) {

		std::wcout << L"PF  ";
	}

	//AF �ڵ�4λ
	if ((context.EFlags & 0x10) != 0) {
		
		std::wcout << L"AF  ";
	}

	//ZF �ڵ�6λ
	if ((context.EFlags & 0x40) != 0) {

		std::wcout << L"ZF  ";
	}

	//SF �ڵ�7λ
	if ((context.EFlags & 0x80) != 0) {

		std::wcout << L"SF  ";
	}

	//OF �ڵ�11λ
	if ((context.EFlags & 0x400) != 0) {

		std::wcout << L"OF  ";
	}

	//DF �ڵ�10λ
	if ((context.EFlags & 0x200) != 0) {

		std::wcout << L"DF  ";
	}

	std::wcout << std::endl;
}
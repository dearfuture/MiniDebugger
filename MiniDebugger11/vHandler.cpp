#include <Windows.h>
#include <DbgHelp.h>
#include <iostream>
#include <list>
#include "TypeHelper.h"
#include "CmdHandlers.h"
#include "DebugSession.h"
#include "HelperFunctions.h"


//��������һЩ������Ϣ�Ľṹ��
struct VARIABLE_INFO {
	DWORD address;
	DWORD modBase;
	DWORD size;
	DWORD typeID;
	std::wstring name;
};



void EnumSymbols(LPCWSTR expression);
BOOL CALLBACK EnumVariablesCallBack(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext);
DWORD GetSymbolAddress(PSYMBOL_INFO pSymbol);
BOOL IsSimpleType(DWORD typeID, DWORD modBase);
void ShowVariables(const std::list<VARIABLE_INFO>& varInfoList);
void ShowVariableSummary(const VARIABLE_INFO* pVarInfo);
void ShowVariableValue(const VARIABLE_INFO* pVarInfo);



//��ʾ�ֲ���������Ĵ�������
//�����ʽΪ lv [expression]
//����expression��ʾ����ʾ����ָ��ͨ������ʽ�ı��������ʡ������ʾȫ��������
void OnShowLocalVariables(const Command& cmd) {

	CHECK_DEBUGGEE;

	//��ȡ��ǰ�����Ŀ�ʼ��ַ
	CONTEXT context;
	GetDebuggeeContext(&context);
	
	//����ջ֡
	IMAGEHLP_STACK_FRAME stackFrame = { 0 };
	stackFrame.InstructionOffset = context.Eip;

	if (SymSetContext(GetDebuggeeHandle(), &stackFrame, NULL) == FALSE) {

		if (GetLastError() != ERROR_SUCCESS) {
			std::wcout << TEXT("No debug info in current address.") << std::endl;
			return;
		}
	}

	LPCWSTR expression = NULL;

	if (cmd.size() >= 2) {
		expression = cmd[1].c_str();
	}

	//ö�ٷ���
	std::list<VARIABLE_INFO> varInfoList;

	if (SymEnumSymbols(
		GetDebuggeeHandle(),
		0,
		expression,
		EnumVariablesCallBack,
		&varInfoList) == FALSE) {

		std::wcout << TEXT("SymEnumSymbols failed: ") << GetLastError() << std::endl;
	}

	ShowVariables(varInfoList);
}



//��ʾȫ�ֱ�������Ĵ�������
//�����ʽΪ gv [expression]
//����expression��ʾ����ʾ����ָ��ͨ������ʽ�ı��������ʡ������ʾȫ��������
void OnShowGlobalVariables(const Command& cmd) {

	CHECK_DEBUGGEE;

	LPCWSTR expression = NULL;

	if (cmd.size() >= 2) {
		expression = cmd[1].c_str();
	}

	//��ȡ��ǰEIP
	CONTEXT context;
	GetDebuggeeContext(&context);

	//��ȡEIP��Ӧ��ģ��Ļ���ַ
	DWORD modBase = (DWORD)SymGetModuleBase64(GetDebuggeeHandle(), context.Eip);

	if (modBase == 0) {
		std::wcout << TEXT("SymGetModuleBase64 failed: ") << GetLastError() << std::endl;
		return;
	}

	std::list<VARIABLE_INFO> varInfoList;

	if (SymEnumSymbols(
		GetDebuggeeHandle(),
		modBase,
		expression,
		EnumVariablesCallBack,
		&varInfoList) == FALSE) {

		std::wcout << TEXT("SymEnumSymbols failed: ") << GetLastError() << std::endl;
	}

	ShowVariables(varInfoList);
}





BOOL CALLBACK EnumVariablesCallBack(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext) {

	std::list<VARIABLE_INFO>* pVarInfoList = (std::list<VARIABLE_INFO>*)UserContext;

	VARIABLE_INFO varInfo;

	if (pSymInfo->Tag == SymTagData) {

		varInfo.address = GetSymbolAddress(pSymInfo);
		varInfo.modBase = (DWORD)pSymInfo->ModBase;
		varInfo.size = SymbolSize;
		varInfo.typeID = pSymInfo->TypeIndex;
		varInfo.name = pSymInfo->Name;

		pVarInfoList->push_back(varInfo);
	}

	return TRUE;
}



//��ʾ�����б�
void ShowVariables(const std::list<VARIABLE_INFO>& varInfoList) {

	//���ֻ��һ������������ʾ�����е���Ϣ
	if (varInfoList.size() == 1) {

		ShowVariableSummary(&*varInfoList.cbegin());
		
		std::wcout << TEXT("  ");

		if (IsSimpleType(varInfoList.cbegin()->typeID, varInfoList.cbegin()->modBase) == FALSE) {
			std::wcout << std::endl;
		}

		ShowVariableValue(&*varInfoList.cbegin());

		std::wcout << std::endl;

		return;
	}
		
	for (auto iterator = varInfoList.cbegin(); iterator != varInfoList.cend(); ++iterator) {

		ShowVariableSummary(&*iterator);

		if (IsSimpleType(iterator->typeID, iterator->modBase) == TRUE) {

			std::wcout << TEXT("  ");
			ShowVariableValue(&*iterator);
		}

		std::wcout << std::endl;
	}
}




//��ʾ�����ĸ�Ҫ��Ϣ
void ShowVariableSummary(const VARIABLE_INFO* pVarInfo) {

	std::wcout << GetTypeName(pVarInfo->typeID, pVarInfo->modBase) << TEXT("  ") 
			   << pVarInfo->name << TEXT("  ") << pVarInfo->size << TEXT("  ");

	PrintHex(pVarInfo->address, FALSE);
}



//��ʾ������ֵ
void ShowVariableValue(const VARIABLE_INFO* pVarInfo) {

	//��ȡ���ŵ��ڴ�
	BYTE* pData = (BYTE*)malloc(sizeof(BYTE) * pVarInfo->size);
	ReadDebuggeeMemory(pVarInfo->address, pVarInfo->size, pData);

	std::wcout << GetTypeValue(
				     pVarInfo->typeID,
					 pVarInfo->modBase,
					 pVarInfo->address,
					 pData);
	
	free(pData);
}





//��ȡ���ŵ������ַ
//���������һ���ֲ��������߲�����
//pSymbol->Address�������EBP��ƫ�ƣ�
//��������Ӿ��Ƿ��ŵ������ַ
DWORD GetSymbolAddress(PSYMBOL_INFO pSymbolInfo) {

	if ((pSymbolInfo->Flags & SYMFLAG_REGREL) == 0) {
		return DWORD(pSymbolInfo->Address);
	}

	//�����ǰEIPָ�����ĵ�һ��ָ���EBP��ֵ��Ȼ������
	//��һ�������ģ����Դ�ʱ����ʹ��EBP����Ӧ��ʹ��ESP-4��
	//Ϊ���ŵĻ���ַ��

	CONTEXT context;
	GetDebuggeeContext(&context);

	//��ȡ��ǰ�����Ŀ�ʼ��ַ
	DWORD64 displacement;
	SYMBOL_INFO symbolInfo = { 0 };
	symbolInfo.SizeOfStruct = sizeof(SYMBOL_INFO);

	SymFromAddr(
		GetDebuggeeHandle(),
		context.Eip,
		&displacement,
		&symbolInfo);

	//����Ǻ����ĵ�һ��ָ�����ʹ��EBP
	if (displacement == 0) {
		return DWORD(context.Esp - 4 + pSymbolInfo->Address);
	}

	return DWORD(context.Ebp + pSymbolInfo->Address);
}

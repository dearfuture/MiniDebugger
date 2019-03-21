#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <Windows.h>
#include <DbgHelp.h>
#include "CmdHandlers.h"
#include "DebugSession.h"
#include "HelperFunctions.h"



void DisplaySourceLines(LPCTSTR sourceFile, int lineNum, unsigned int address, int after, int before);
void DisplayLine(LPCTSTR sourceFile, const std::wstring& line, int lineNumber, BOOL isCurLine);



//��ʾԴ��������Ĵ�������
//�����ʽΪ l [after] [before]
//after��ʾ��ʾ��ǰ������֮�������������С��0
//before��ʾ��ʾ��ǰ������֮ǰ������������С��0
//���ʡ����after��before�����ǵ�ȡֵ����10
void OnShowSourceLines(const Command& cmd) {

	CHECK_DEBUGGEE;

	int afterLines = 10;
	int beforeLines = 10;

	if (cmd.size() > 1) {

		afterLines = _wtoi(cmd[1].c_str());

		if (afterLines < 0) {

			std::wcout << TEXT("Invalid params.") << std::endl;
			return;
		}
	}

	if (cmd.size() > 2) {

		beforeLines = _wtoi(cmd[2].c_str());

		if (beforeLines < 0) {

			std::wcout << TEXT("Invalid params.") << std::endl;
			return;
		}
	}

	//��ȡEIP
	CONTEXT context;
	GetDebuggeeContext(&context);

	//��ȡԴ�ļ��Լ�����Ϣ
	IMAGEHLP_LINE64 lineInfo = { 0 };
	lineInfo.SizeOfStruct = sizeof(lineInfo);
	DWORD displacement = 0;

	if (SymGetLineFromAddr64(
		GetDebuggeeHandle(),
		context.Eip,
		&displacement,
		&lineInfo) == FALSE) {

		DWORD errorCode = GetLastError();
		
		switch (errorCode) {

			// 126 ��ʾ��û��ͨ��SymLoadModule64����ģ����Ϣ
			case 126:
				std::wcout << TEXT("Debug info in current module has not loaded.") << std::endl;
				return;

			// 487 ��ʾģ��û�е��Է���
			case 487:
				std::wcout << TEXT("No debug info in current module.") << std::endl;
				return;

			default:
				std::wcout << TEXT("SymGetLineFromAddr64 failed: ") << errorCode << std::endl;
				return;
		}
	}

	DisplaySourceLines(
		lineInfo.FileName, 
		lineInfo.LineNumber,
		(unsigned int)lineInfo.Address,
		afterLines, 
		beforeLines);
}



//��ʾԴ�ļ���ָ������
void DisplaySourceLines(LPCTSTR sourceFile, int lineNum, unsigned int address, int after, int before) {

	std::wcout << std::endl;

	std::wifstream inputStream(sourceFile);

	if (inputStream.fail() == true) {

		std::wcout << TEXT("Open source file failed.") << std::endl 
				   << TEXT("Path: ") << sourceFile << std::endl;
		return;
	}

	inputStream.imbue(std::locale("chs", std::locale::ctype));

	int curLineNumber = 1;

	//����ӵڼ��п�ʼ���
	int startLineNumber = lineNum - before;
	if (startLineNumber < 1) {
		startLineNumber = 1;
	}

	std::wstring line;

	//��������Ҫ��ʾ����
	while (curLineNumber < startLineNumber) {

		std::getline(inputStream, line);
		++curLineNumber;
	}

	//�����ʼ�е���ǰ��֮�����
	while (curLineNumber < lineNum) {

		std::getline(inputStream, line);
		DisplayLine(sourceFile, line, curLineNumber, FALSE);
		++curLineNumber;
	}

	//�����ǰ��
	getline(inputStream, line);
	DisplayLine(sourceFile, line, curLineNumber, TRUE);
	++curLineNumber;

	//�����ǰ�е������֮�����
	int lastLineNumber = lineNum + after;
	while (curLineNumber <= lastLineNumber) {

		if (!getline(inputStream, line)) {
			break;
		}

		DisplayLine(sourceFile, line, curLineNumber, FALSE);
		++curLineNumber;
	}

	inputStream.close();
}



//��ʾԴ�ļ��е�һ�С�
void DisplayLine(LPCTSTR sourceFile, const std::wstring& line, int lineNumber, BOOL isCurLine) {

	if (isCurLine == TRUE) {
		std::wcout << TEXT("=>");
	}
	else {
		std::wcout << TEXT("  ");
	}

	LONG displacement;
	IMAGEHLP_LINE64 lineInfo = { 0 };
	lineInfo.SizeOfStruct = sizeof(lineInfo);

	if (SymGetLineFromName64(
		GetDebuggeeHandle(),
		NULL,
		sourceFile,
		lineNumber, 
		&displacement,
		&lineInfo) == FALSE) {

		std::wcout << TEXT("SymGetLineFromName64 failed: ") << GetLastError() << std::endl;
		return;
	}

	std::wcout << std::setw(4) << std::setfill(TEXT(' ')) << lineNumber << TEXT("  ");

	if (displacement == 0) {

		PrintHex((unsigned int)lineInfo.Address, FALSE);
	}
	else {

		std::wcout << std::setw(8) << TEXT(" ");
	}

	std::wcout << TEXT("  ") << line << std::endl;
}
#include <iostream>
#include <sstream>
#include <iomanip>
#include "HelperFunctions.h"
#include "DebugSession.h"
#include "CmdHandlers.h"



void DumpHex(unsigned int address, unsigned int length);
char ByteToChar(BYTE byte);



//��ʾ�ڴ����ݵ����������
//�����ʽΪ d [address] [length]
//addressΪ16���ƣ�lengthΪ10���ơ�
//���ʡ���˵�������������lengthΪ128��
//���ʡ���˵ڶ�����������addressΪEIPָ��ĵ�ַ��
void OnDump(const Command& cmd) {

	CHECK_DEBUGGEE;

	unsigned int address = 0;
	unsigned int length = 128;

	if (cmd.size() < 2) {

		CONTEXT context;
		GetDebuggeeContext(&context);

		address = context.Eip;
	}
	else {

		address = wcstoul(cmd[1].c_str(), NULL, 16);
	}

	if (cmd.size() > 2) {

		length = _wtoi(cmd[2].c_str());
	}

	DumpHex(address, length);	
}



//��ʮ�����Ƶĸ�ʽ��ʾָ����ַ�����ڴ档ÿһ�й̶���ʾ16���ֽڣ�����ĵ�ַҪ���뵽16�ı�����
//��ʾ�������������һ��wostringstream�����У�
//����������������е��ַ�����
void DumpHex(unsigned int address, unsigned int length) {

	std::wostringstream dumpStr;
	dumpStr.imbue(std::locale("chs", std::locale::ctype));
	dumpStr << std::hex << std::uppercase;

	//�洢ÿһ���ֽڶ�Ӧ��ASCII�ַ���
	char content[17] = { 0 };

	//����ʼ����ĵ�ַ���뵽16�ı�����
	unsigned int blankLen = address % 16;
	unsigned int startAddress = address - blankLen;

	unsigned int lineLen = 0;
	unsigned int contentLen = 0;

	//���������һ�еĿհס�
	if (blankLen != 0) {

		dumpStr << std::setw(8) << std::setfill(TEXT('0')) << startAddress << TEXT("  ");
		startAddress += 16;

		for (unsigned int len = 0; len < blankLen; ++len) {

			dumpStr << TEXT("   ");
			content[contentLen] = TEXT(' ');
			++contentLen;
			++lineLen;
		}
	}

	//����ֽڶ�ȡ�����Խ��̵��ڴ棬�������
	BYTE byte;

	for (unsigned int memLen = 0; memLen < length; ++memLen) {

		unsigned int curAddress = address + memLen;

		//�����ÿ�еĵ�һ���ֽڣ�����������ַ��
		if (lineLen % 16 == 0) {

			dumpStr << std::setw(8) << std::setfill(TEXT('0')) << startAddress << TEXT("  ");
			startAddress += 16;

			lineLen = 0;
		}

		//��ȡ�ڴ棬����ɹ��Ļ�����ԭ�����������ȡ���Ӧ��ASCII�ַ���
		if (ReadDebuggeeMemory(curAddress, 1, &byte) == TRUE) {
			
			dumpStr << std::setw(2) << std::setfill(TEXT('0')) << byte << TEXT(' ');
			content[contentLen] = ByteToChar(byte);
		}
		//�����ȡʧ�ܣ����� ?? ���档
		else {

			dumpStr << TEXT("?? ");
			content[contentLen] = TEXT('.');
		}

		//���һ������16���ֽڣ���������з������ڴ�0��ʼ����������������15�Ƚϡ�
		if (contentLen == 15) {
			dumpStr << TEXT(' ') << content << std::endl;
		}

		++contentLen;
		contentLen %= 16;

		++lineLen;
	}

	//������һ�еĿհף�Ϊ��ʹ���һ�е�ASCII�ַ�������һ�С�
	for (unsigned int len = 0; len < 16 - lineLen; ++len) {

		dumpStr << TEXT("   ");
	}

	//������һ�е�ASCII�ַ���
	dumpStr << TEXT(' ');
	content[contentLen] = 0;
	dumpStr << content;

	std::wcout << dumpStr.str() << std::endl;
}



//���ֽ�ת�����ַ���������ʾ���ַ��ֱ��� . �� �� ��ʾ��
char ByteToChar(BYTE byte) {

	if (byte >= 0x00 && byte <= 0x1F) {
		return '.';
	}

	if (byte >= 0x81 && byte <= 0xFF) {
		return '?';
	}

	return (char)byte;
}
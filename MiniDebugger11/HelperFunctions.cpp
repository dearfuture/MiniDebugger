#include <iostream>
#include <iomanip>
#include "HelperFunctions.h"



//�ڱ�׼��������16����ֵ��
void PrintHex(unsigned int value, BOOL hasPrefix) {

	std::wcout << std::hex << std::uppercase;

	if (hasPrefix == TRUE) {
		std::wcout << TEXT("0x");
	}

	std::wcout << std::setw(8) << std::setfill(TEXT('0')) << value << std::dec << std::nouppercase << std::flush;
}
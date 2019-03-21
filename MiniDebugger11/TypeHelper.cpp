#include <Windows.h>
#include <DbgHelp.h>
#include <sstream>
#include <iostream>
#include <string>
#include <iomanip>
#include "TypeHelper.h"
#include "DebugSession.h"
#include "HelperFunctions.h"



CBaseTypeEnum GetCBaseType(int typeID, DWORD modBase);
std::wstring GetBaseTypeName(int typeID, DWORD modBase);
std::wstring GetPointerTypeName(int typeID, DWORD modBase);
std::wstring GetArrayTypeName(int typeID, DWORD modBase);
std::wstring GetEnumTypeName(int typeID, DWORD modBase);
std::wstring GetNameableTypeName(int typeID, DWORD modBase);
std::wstring GetUDTTypeName(int typeID, DWORD modbase);
std::wstring GetFunctionTypeName(int typeID, DWORD modBase);

char ConvertToSafeChar(char ch);
wchar_t ConvertToSafeWChar(wchar_t ch);
std::wstring GetBaseTypeValue(int typeID, DWORD modBase, const BYTE* pData);
std::wstring GetPointerTypeValue(int typeID, DWORD modBase, const BYTE* pData);
std::wstring GetEnumTypeValue(int typeID, DWORD modBase, const BYTE* pData);
std::wstring GetArrayTypeValue(int typeID, DWORD modBase, DWORD address, const BYTE* pData);
std::wstring GetUDTTypeValue(int typeID, DWORD modBase, DWORD address, const BYTE* pData);
BOOL GetDataMemberInfo(DWORD memberID, DWORD modBase, DWORD address, const BYTE* pData, std::wostringstream& valueBuilder);
BOOL VariantEqual(VARIANT var, CBaseTypeEnum cBaseType, const BYTE* data);




struct BaseTypeEntry {

	CBaseTypeEnum type;
	const LPCWSTR name;

} g_baseTypeNameMap[] = {

	{ cbtNone, TEXT("<no-type>") },
	{ cbtVoid, TEXT("void") },
	{ cbtBool, TEXT("bool") },
	{ cbtChar, TEXT("char") },
	{ cbtUChar, TEXT("unsigned char") },
	{ cbtWChar, TEXT("wchar_t") },
	{ cbtShort, TEXT("short") },
	{ cbtUShort, TEXT("unsigned short") },
	{ cbtInt, TEXT("int") },
	{ cbtUInt, TEXT("unsigned int") },
	{ cbtLong, TEXT("long") },
	{ cbtULong, TEXT("unsigned long") },
	{ cbtLongLong, TEXT("long long") },
	{ cbtULongLong, TEXT("unsigned long long") },
	{ cbtFloat, TEXT("float") },
	{ cbtDouble, TEXT("double") },
	{ cbtEnd, TEXT("") },
};





//�ж��Ƿ�򵥵�����
BOOL IsSimpleType(DWORD typeID, DWORD modBase) {

	DWORD symTag;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_SYMTAG,
		&symTag);

	switch (symTag) {

		case SymTagBaseType:
		case SymTagPointerType:
		case SymTagEnum:
			return TRUE;

		default:
			return FALSE;
	}
}




//��ȡָ�����͵�����
std::wstring GetTypeName(int typeID, DWORD modBase) {

	DWORD typeTag;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_SYMTAG,
		&typeTag);

	switch (typeTag) {
		
		case SymTagBaseType:
			return GetBaseTypeName(typeID, modBase);

		case SymTagPointerType:
			return GetPointerTypeName(typeID, modBase);

		case SymTagArrayType:
			return GetArrayTypeName(typeID, modBase);

		case SymTagUDT:
			return GetUDTTypeName(typeID, modBase);

		case SymTagEnum:
			return GetEnumTypeName(typeID, modBase);

		case SymTagFunctionType:
			return GetFunctionTypeName(typeID, modBase);

		default:
			return L"??";
	}
}




//��ȡ�������͵����ơ�
std::wstring GetBaseTypeName(int typeID, DWORD modBase) {

	CBaseTypeEnum baseType = GetCBaseType(typeID, modBase);

	int index = 0;

	while (g_baseTypeNameMap[index].type != cbtEnd) {

		if (g_baseTypeNameMap[index].type == baseType) {
			break;
		}

		++index;
	}

	return g_baseTypeNameMap[index].name;
}



//��ȡ��ʾC/C++�������͵�ö��
//�ٶ�typeID�Ѿ��ǻ������͵�ID.
CBaseTypeEnum GetCBaseType(int typeID, DWORD modBase) {

	//��ȡBaseTypeEnum
	DWORD baseType;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_BASETYPE,
		&baseType);

	//��ȡ�������͵ĳ���
	ULONG64 length;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_LENGTH,
		&length);

	switch (baseType) {

		case btVoid:
			return cbtVoid;

		case btChar:
			return cbtChar;

		case btWChar:
			return cbtWChar;

		case btInt:
			switch (length) {
				case 2:  return cbtShort;
				case 4:  return cbtInt;
				default: return cbtLongLong;
			}

		case btUInt:
			switch (length) {
				case 1:  return cbtUChar;
				case 2:  return cbtUShort;
				case 4:  return cbtUInt;
				default: return cbtULongLong;
			}

		case btFloat:
			switch (length) {
				case 4:  return cbtFloat;
				default: return cbtDouble;
			}

		case btBool:
			return cbtBool;

		case btLong:
			return cbtLong;

		case btULong:
			return cbtULong;

		default:
			return cbtNone;
	}
}




//��ȡָ�����͵�����
std::wstring GetPointerTypeName(int typeID, DWORD modBase) {

	//��ȡ��ָ�����ͻ�����������
	BOOL isReference;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_IS_REFERENCE,
		&isReference);

	//��ȡָ����ָ��������͵�typeID
	DWORD innerTypeID;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_TYPEID,
		&innerTypeID);

	return GetTypeName(innerTypeID, modBase) + (isReference == TRUE ? TEXT("&") : TEXT("*"));
}



//��ȡ�������͵�����
std::wstring GetArrayTypeName(int typeID, DWORD modBase) {

	//��ȡ����Ԫ�ص�TypeID
	DWORD innerTypeID;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_TYPEID,
		&innerTypeID);

	//��ȡ����Ԫ�ظ���
	DWORD elemCount;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_COUNT,
		&elemCount);

	std::wostringstream strBuilder;

	strBuilder << GetTypeName(innerTypeID, modBase) << TEXT('[') << elemCount << TEXT(']');

	return strBuilder.str();
}



//��ȡ�������͵�����
std::wstring GetFunctionTypeName(int typeID, DWORD modBase) {

	std::wostringstream nameBuilder;

	//��ȡ��������
	DWORD paramCount;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_CHILDRENCOUNT,
		&paramCount);

	//��ȡ����ֵ������
	DWORD returnTypeID;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_TYPEID,
		&returnTypeID);

	nameBuilder << GetTypeName(returnTypeID, modBase);

	//��ȡÿ������������
	BYTE* pBuffer = (BYTE*)malloc(sizeof(TI_FINDCHILDREN_PARAMS) + sizeof(ULONG) * paramCount);
	TI_FINDCHILDREN_PARAMS* pFindParams = (TI_FINDCHILDREN_PARAMS*)pBuffer;
	pFindParams->Count = paramCount;
	pFindParams->Start = 0;

	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_FINDCHILDREN,
		pFindParams);

	nameBuilder << TEXT('(');

	for (int index = 0; index != paramCount; ++index) {

		DWORD paramTypeID;
		SymGetTypeInfo(
			GetDebuggeeHandle(),
			modBase,
			pFindParams->ChildId[index],
			TI_GET_TYPEID,
			&paramTypeID);

		if (index != 0) {
			nameBuilder << TEXT(", ");
		}

		nameBuilder << GetTypeName(paramTypeID, modBase);
	}

	nameBuilder << TEXT(')');

	free(pBuffer);

	return nameBuilder.str();
}



//��ȡö�����͵�����
std::wstring GetEnumTypeName(int typeID, DWORD modBase) {

	return GetNameableTypeName(typeID, modBase);
}



//��ȡUDT���͵�����
std::wstring GetUDTTypeName(int typeID, DWORD modBase) {

	return GetNameableTypeName(typeID, modBase);
}



//��ȡ�������Ƶ����͵�����
std::wstring GetNameableTypeName(int typeID, DWORD modBase) {

	WCHAR* pBuffer;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_SYMNAME,
		&pBuffer);

	std::wstring typeName(pBuffer);

	LocalFree(pBuffer);

	return typeName;
}







//��ָ����ַ�����ڴ棬������Ӧ���͵���ʽ���ֳ���
std::wstring GetTypeValue(int typeID, DWORD modBase, DWORD address, const BYTE* pData) {

	DWORD typeTag;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_SYMTAG,
		&typeTag);

	switch (typeTag) {
		
		case SymTagBaseType:
			return GetBaseTypeValue(typeID, modBase, pData);

		case SymTagPointerType:
			return GetPointerTypeValue(typeID, modBase, pData);

		case SymTagEnum:
			return GetEnumTypeValue(typeID, modBase, pData);

		case SymTagArrayType:
			return GetArrayTypeValue(typeID, modBase, address, pData);

		case SymTagUDT:
			return GetUDTTypeValue(typeID, modBase, address, pData);

		case SymTagTypedef:

			//��ȡ�������͵�ID
			DWORD actTypeID;
			SymGetTypeInfo(
				GetDebuggeeHandle(),
				modBase,
				typeID,
				TI_GET_TYPEID,
				&actTypeID);

			return GetTypeValue(actTypeID, modBase, address, pData);

		default:
			return L"??";
	}
}



//��ȡ�������ͱ�����ֵ
std::wstring GetBaseTypeValue(int typeID, DWORD modBase, const BYTE* pData) {

	CBaseTypeEnum cBaseType = GetCBaseType(typeID, modBase);

	return GetCBaseTypeValue(cBaseType, pData);
}



//��ȡC/C++�������͵�ֵ
std::wstring GetCBaseTypeValue(CBaseTypeEnum cBaseType, const BYTE* pData) {

	std::wostringstream valueBuilder;

	switch (cBaseType) {

		case cbtNone:
			valueBuilder << TEXT("??");
			break;

		case cbtVoid:
			valueBuilder << TEXT("??");
			break;

		case cbtBool:
			valueBuilder << (*pData == 0 ? L"false" : L"true");
			break;

		case cbtChar:
			valueBuilder << ConvertToSafeChar(*((char*)pData));
			break;

		case cbtUChar:
			valueBuilder << std::hex 
						 << std::uppercase 
						 << std::setw(2) 
						 << std::setfill(TEXT('0')) 
						 << *((unsigned char*)pData);
			break;

		case cbtWChar:
			valueBuilder << ConvertToSafeWChar(*((wchar_t*)pData));
			break;

		case cbtShort:
			valueBuilder << *((short*)pData);
			break;

		case cbtUShort:
			valueBuilder << *((unsigned short*)pData);
			break;

		case cbtInt:
			valueBuilder << *((int*)pData);
			break;

		case cbtUInt:
			valueBuilder << *((unsigned int*)pData);
			break;

		case cbtLong:
			valueBuilder << *((long*)pData);
			break;

		case cbtULong:
			valueBuilder << *((unsigned long*)pData);
			break;

		case cbtLongLong:
			valueBuilder << *((long long*)pData);
			break;

		case cbtULongLong:
			valueBuilder << *((unsigned long long*)pData);
			break;

		case cbtFloat:
			valueBuilder << *((float*)pData);
			break;

		case cbtDouble:
			valueBuilder << *((double*)pData);
			break;
	}

	return valueBuilder.str();
}



//��ȡָ�����ͱ�����ֵ
std::wstring GetPointerTypeValue(int typeID, DWORD modBase, const BYTE* pData) {

	std::wostringstream valueBuilder;

	valueBuilder << std::hex << std::uppercase << std::setfill(TEXT('0')) << std::setw(8) << *((DWORD*)pData);

	return valueBuilder.str();
}



//��ȡö�����ͱ�����ֵ
std::wstring GetEnumTypeValue(int typeID, DWORD modBase, const BYTE* pData) {

	std::wstring valueName;

	//��ȡö��ֵ�Ļ�������
	CBaseTypeEnum cBaseType = GetCBaseType(typeID, modBase);

	//��ȡö��ֵ�ĸ���
	DWORD childrenCount;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_CHILDRENCOUNT,
		&childrenCount);

	//��ȡÿ��ö��ֵ
	TI_FINDCHILDREN_PARAMS* pFindParams =
		(TI_FINDCHILDREN_PARAMS*)malloc(sizeof(TI_FINDCHILDREN_PARAMS) + childrenCount * sizeof(ULONG));
	pFindParams->Start = 0;
	pFindParams->Count = childrenCount;

	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_FINDCHILDREN,
		pFindParams);

	for (int index = 0; index != childrenCount; ++index) {

		//��ȡö��ֵ
		VARIANT enumValue;
		SymGetTypeInfo(
			GetDebuggeeHandle(),
			modBase,
			pFindParams->ChildId[index],
			TI_GET_VALUE,
			&enumValue);

		if (VariantEqual(enumValue, cBaseType, pData) == TRUE) {

			//��ȡö��ֵ������
			WCHAR* pBuffer;
			SymGetTypeInfo(
				GetDebuggeeHandle(),
				modBase,
				pFindParams->ChildId[index],
				TI_GET_SYMNAME,
				&pBuffer);

			valueName = pBuffer;
			LocalFree(pBuffer);

			break;
		}
	}

	free(pFindParams);

	//���û���ҵ���Ӧ��ö��ֵ������ʾ���Ļ�������ֵ
	if (valueName.length() == 0) {

		valueName = GetBaseTypeValue(typeID, modBase, pData);
	}

	return valueName;
}



//��һ��VARIANT���͵ı�����һ���ڴ������ݽ��бȽϣ��ж��Ƿ���ȡ�
//�ڴ������ݵ�������cBaseType����ָ��
BOOL VariantEqual(VARIANT var, CBaseTypeEnum cBaseType, const BYTE* pData) {

	switch (cBaseType) {

		case cbtChar:
			return var.cVal == *((char*)pData);

		case cbtUChar:
			return var.bVal == *((unsigned char*)pData);

		case cbtShort:
			return var.iVal == *((short*)pData);

		case cbtWChar:
		case cbtUShort:
			return var.uiVal == *((unsigned short*)pData);

		case cbtUInt:
			return var.uintVal == *((int*)pData);

		case cbtLong:
			return var.lVal == *((long*)pData);

		case cbtULong:
			return var.ulVal == *((unsigned long*)pData);

		case cbtLongLong:
			return var.llVal == *((long long*)pData);

		case cbtULongLong:
			return var.ullVal == *((unsigned long long*)pData);

		case cbtInt:
		default:
			return var.intVal == *((int*)pData);
	}
}



//��ȡ�������ͱ�����ֵ
//ֻ��ȡ���32��Ԫ�ص�ֵ
std::wstring GetArrayTypeValue(int typeID, DWORD modBase, DWORD address, const BYTE* pData) {

	//��ȡԪ�ظ��������Ԫ�ظ�������32,������Ϊ32
	DWORD elemCount;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_COUNT,
		&elemCount);

	elemCount = elemCount > 32 ? 32 : elemCount;

	//��ȡ����Ԫ�ص�TypeID
	DWORD innerTypeID;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_TYPEID,
		&innerTypeID);

	//��ȡ����Ԫ�صĳ���
	ULONG64 elemLen;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		innerTypeID,
		TI_GET_LENGTH,
		&elemLen);

	std::wostringstream valueBuilder;

	for (int index = 0; index != elemCount; ++index) {

		DWORD elemOffset = DWORD(index * elemLen);

		valueBuilder << TEXT("  [") << index << TEXT("]  ")
					 << GetTypeValue(innerTypeID, modBase, address + elemOffset, pData + index * elemLen);

		if (index != elemCount - 1) {
			valueBuilder << std::endl;
		}
	}

	return valueBuilder.str();
}



//��ȡ�û��������͵�ֵ
std::wstring GetUDTTypeValue(int typeID, DWORD modBase, DWORD address, const BYTE* pData) {

	std::wostringstream valueBuilder;

	//��ȡ��Ա����
	DWORD memberCount;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_GET_CHILDRENCOUNT,
		&memberCount);

	//��ȡ��Ա��Ϣ
	TI_FINDCHILDREN_PARAMS* pFindParams =
		(TI_FINDCHILDREN_PARAMS*)malloc(sizeof(TI_FINDCHILDREN_PARAMS) + memberCount * sizeof(ULONG));
	pFindParams->Start = 0;
	pFindParams->Count = memberCount;

	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		typeID,
		TI_FINDCHILDREN,
		pFindParams);

	//������Ա
	for (int index = 0; index != memberCount; ++index) {

		BOOL isDataMember = GetDataMemberInfo(
			pFindParams->ChildId[index],
			modBase,
			address,
			pData,
			valueBuilder);

		if (isDataMember == TRUE) {
			valueBuilder << std::endl;
		}
	}

	valueBuilder.seekp(-1, valueBuilder.end);
	valueBuilder.put(0);

	return valueBuilder.str();
}



//��ȡ���ݳ�Ա����Ϣ
//�����Ա�����ݳ�Ա������TRUE
//���򷵻�FALSE
BOOL GetDataMemberInfo(DWORD memberID, DWORD modBase, DWORD address, const BYTE* pData, std::wostringstream& valueBuilder) {

	DWORD memberTag;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		memberID,
		TI_GET_SYMTAG,
		&memberTag);

	if (memberTag != SymTagData && memberTag != SymTagBaseClass) {
		return FALSE;
	}

	valueBuilder << TEXT("  ");

	DWORD memberTypeID;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		memberID,
		TI_GET_TYPEID,
		&memberTypeID);

	//�������
	valueBuilder << GetTypeName(memberTypeID, modBase);

	//�������
	if (memberTag == SymTagData) {

		WCHAR* name;
		SymGetTypeInfo(
			GetDebuggeeHandle(),
			modBase,
			memberID,
			TI_GET_SYMNAME,
			&name);

		valueBuilder << TEXT("  ") << name;

		LocalFree(name);
	}
	else {

		valueBuilder << TEXT("  <base-class>");
	}

	//�������
	ULONG64 length;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		memberTypeID,
		TI_GET_LENGTH,
		&length);

	valueBuilder << TEXT("  ") << length;

	//�����ַ
	DWORD offset;
	SymGetTypeInfo(
		GetDebuggeeHandle(),
		modBase,
		memberID,
		TI_GET_OFFSET,
		&offset);

	DWORD childAddress = address + offset;

	valueBuilder << TEXT("  ") << std::hex << std::uppercase << std::setfill(TEXT('0')) << std::setw(8) << childAddress << std::dec;

	//���ֵ
	if (IsSimpleType(memberTypeID, modBase) == TRUE) {

		valueBuilder << TEXT("  ") 
						<< GetTypeValue(
							memberTypeID,
							modBase,
							childAddress, 
							pData + offset);
	}

	return TRUE;
}



//��char���͵��ַ�ת���ɿ������������̨���ַ�,
//���ch�ǲ�����ʾ���ַ����򷵻�һ���ʺţ�
//����ֱ�ӷ���ch��
//С��0x1E�ʹ���0x7F��ֵ��������ʾ��
char ConvertToSafeChar(char ch) {

	if (ch < 0x1E || ch > 0x7F) {
		return '?';
	}

	return ch;
}



//��wchar_t���͵��ַ�ת���ɿ������������̨���ַ�,
//�����ǰ�Ĵ���ҳ������ʾch���򷵻�һ���ʺţ�
//����ֱ�ӷ���ch��
wchar_t ConvertToSafeWChar(wchar_t ch) {

	if (ch < 0x1E) {
		return L'?';
	}

	char buffer[4];

	size_t convertedCount;
	wcstombs_s(&convertedCount, buffer, 4, &ch, 2);

	if (convertedCount == 0) {
		return L'?';
	}
	
	return ch;
}
#pragma once

#include <Windows.h>
#include <string>
#include <list>


enum BaseTypeEnum {
   btNoType = 0,
   btVoid = 1,
   btChar = 2,
   btWChar = 3,
   btInt = 6,
   btUInt = 7,
   btFloat = 8,
   btBCD = 9,
   btBool = 10,
   btLong = 13,
   btULong = 14,
   btCurrency = 25,
   btDate = 26,
   btVariant = 27,
   btComplex = 28,
   btBit = 29,
   btBSTR = 30,
   btHresult = 31
};


enum SymTagEnum {
   SymTagNull,
   SymTagExe,
   SymTagCompiland,
   SymTagCompilandDetails,
   SymTagCompilandEnv,
   SymTagFunction,				//����
   SymTagBlock,
   SymTagData,					//����
   SymTagAnnotation,
   SymTagLabel,
   SymTagPublicSymbol,
   SymTagUDT,					//�û��������ͣ�����struct��class��union
   SymTagEnum,					//ö������
   SymTagFunctionType,			//��������
   SymTagPointerType,			//ָ������
   SymTagArrayType,				//��������
   SymTagBaseType,				//��������
   SymTagTypedef,				//typedef����
   SymTagBaseClass,				//����
   SymTagFriend,				//��Ԫ����
   SymTagFunctionArgType,		//������������
   SymTagFuncDebugStart, 
   SymTagFuncDebugEnd,
   SymTagUsingNamespace, 
   SymTagVTableShape,
   SymTagVTable,
   SymTagCustom,
   SymTagThunk,
   SymTagCustomType,
   SymTagManagedType,
   SymTagDimension
};



//��ʾC/C++�������͵�ö��
enum CBaseTypeEnum {
	cbtNone,
	cbtVoid,
	cbtBool,
	cbtChar,
	cbtUChar,
	cbtWChar,
	cbtShort,
	cbtUShort,
	cbtInt,
	cbtUInt,
	cbtLong,
	cbtULong,
	cbtLongLong,
	cbtULongLong,
	cbtFloat,
	cbtDouble,
	cbtEnd,
};



std::wstring GetTypeName(int typeID, DWORD modBase);

std::wstring GetTypeValue(int typeID, DWORD modBase, DWORD address, const BYTE* pData);

BOOL IsSimpleType(DWORD typeID, DWORD modBase);

std::wstring GetCBaseTypeValue(CBaseTypeEnum cBaseType, const BYTE* pData);
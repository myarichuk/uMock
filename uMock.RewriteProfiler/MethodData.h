#pragma once
#include <string>
#include <cor.h>

struct MethodData
{
	wstring MethodName;
	wstring ClassName;
	wstring AssemblyName;
	wstring ModuleName;

	AssemblyID AssemblyId;
    ModuleID ModuleId;
    ClassID ClassId;
	FunctionID FunctionId;

	mdTypeDef ClassTypeDefinition;

	mdMethodDef MethodToken;
	DWORD MethodAttributes;
	PCCOR_SIGNATURE MethodSignature;
	ULONG SignatureLength;
};


#pragma once
#include <Windows.h>
#include <corprof.h>
#include <cor.h>
#include <string>
#include <iostream>
#include "MethodData.h"
#include <limits>

using namespace std;
#define NAME_BUFFER_SIZE 2048
#define DEFINITIONS_BUFFER_SIZE 1024

class MetadataHelper
{
public:
static HRESULT GetTypeReferences(IMetaDataImport2* pIMetaDataImport,vector<mdTypeRef> *typeReferencesCollection)
{
	HCORENUM enumTypeRefs = NULL;
	ULONG typeRefSize = 0;
	ULONG dummyCount = 0;
	ULONG tokenCount = 0;
	HRESULT hr = S_OK;
	mdTypeDef tempTypeDefinitionsCollection[DEFINITIONS_BUFFER_SIZE];
	pIMetaDataImport->EnumTypeRefs(&enumTypeRefs, tempTypeDefinitionsCollection, DEFINITIONS_BUFFER_SIZE, &tokenCount);
	hr = pIMetaDataImport->CountEnum(enumTypeRefs, &typeRefSize);	
	pIMetaDataImport->CloseEnum(enumTypeRefs);
	if(typeRefSize > tokenCount)
	{
		typeReferencesCollection->resize(typeRefSize);

		enumTypeRefs = NULL;
		hr = pIMetaDataImport->EnumTypeRefs(&enumTypeRefs, typeReferencesCollection->data(), typeRefSize, &tokenCount);
		pIMetaDataImport->CloseEnum(enumTypeRefs);
	}
	else
	{
		typeReferencesCollection->insert(typeReferencesCollection->end(),&tempTypeDefinitionsCollection[0],&tempTypeDefinitionsCollection[tokenCount]);
	}
	return hr;
}

static HRESULT GetTypeDefinitions(IMetaDataImport2* pIMetaDataImport,vector<mdTypeDef> *typeDefinitionsCollection)
{
	assert(typeDefinitionsCollection != nullptr);

	HCORENUM enumTypeDefs = NULL;
	ULONG typeDefSize = 0;
	ULONG tokenCount = 0;
	HRESULT hr = S_OK;
	mdTypeDef tempTypeDefinitionsCollection[DEFINITIONS_BUFFER_SIZE];
   
	pIMetaDataImport->EnumTypeDefs(&enumTypeDefs, tempTypeDefinitionsCollection, DEFINITIONS_BUFFER_SIZE, &tokenCount);
	hr = pIMetaDataImport->CountEnum(enumTypeDefs, &typeDefSize);
	pIMetaDataImport->CloseEnum(enumTypeDefs);
	if(typeDefSize > tokenCount)
	{
		typeDefinitionsCollection->resize(typeDefSize);
   
		enumTypeDefs = NULL;
		hr = pIMetaDataImport->EnumTypeDefs(&enumTypeDefs, typeDefinitionsCollection->data(), typeDefSize, &tokenCount);
		pIMetaDataImport->CloseEnum(enumTypeDefs);
	}
	else
	{
		typeDefinitionsCollection->insert(typeDefinitionsCollection->end(),&tempTypeDefinitionsCollection[0],&tempTypeDefinitionsCollection[tokenCount]);
	}

	return hr;
}



static HRESULT GetMethodData(ICorProfilerInfo* corProfilerInfo,wstring methodName,OUT MethodData* methodData)
{
	HRESULT hr = S_OK;		

	return S_OK;
}

static HRESULT GetModuleInfo(ICorProfilerInfo* corProfilerInfo,IN ModuleID moduleId,OUT wstring *moduleName,OUT AssemblyID *moduleAssemblyId)
{ 
	assert(moduleName != nullptr);

	HRESULT hr = S_OK;		
	LPCBYTE moduleLoadAddress;
	WCHAR moduleNameBuffer[NAME_BUFFER_SIZE];
	DWORD moduleFlags;	
	ULONG moduleNameBufferSize;
		
	corProfilerInfo->GetModuleInfo(moduleId,&moduleLoadAddress,NAME_BUFFER_SIZE,&moduleNameBufferSize,moduleNameBuffer,moduleAssemblyId);
	*moduleName = wstring(moduleNameBuffer);

	return hr;
}

static HRESULT GetMetadataImportByModuleID(ICorProfilerInfo* corProfilerInfo,ModuleID moduleId,OUT IMetaDataImport** metadataImport)
{
	HRESULT hr = S_OK;
	IMetaDataImport *pMetadataImport;
	hr = corProfilerInfo->GetModuleMetaData(moduleId,CorOpenFlags::ofRead | CorOpenFlags::ofWrite,IID_IMetaDataImport, (LPUNKNOWN *) &pMetadataImport);
	if(SUCCEEDED(hr) && pMetadataImport == nullptr)
	{
		return E_FAIL;
	}

	*metadataImport = pMetadataImport;
	return hr;
}

static HRESULT GetMetadataImportByModuleID(ICorProfilerInfo2* corProfilerInfo,ModuleID moduleId,OUT IMetaDataImport2** metadataImport)
{
	HRESULT hr = S_OK;
	IMetaDataImport2 *pMetadataImport;
	hr = corProfilerInfo->GetModuleMetaData(moduleId,CorOpenFlags::ofRead | CorOpenFlags::ofWrite,IID_IMetaDataImport, (LPUNKNOWN *) &pMetadataImport);
	if(SUCCEEDED(hr) && pMetadataImport == nullptr)
	{
		return E_FAIL;
	}

	*metadataImport = pMetadataImport;
	return hr;
}


static HRESULT GetFunctionDataByFunctionID(ICorProfilerInfo* corProfilerInfo,FunctionID functionID,OUT ModuleID* functionModuleId,OUT mdToken* funcToken,OUT ClassID *functionClassId)
{
	HRESULT hr1 = S_OK,hr2 = S_OK;			
	IMetaDataImport* metadataImport;

	hr1 = corProfilerInfo->GetTokenAndMetaDataFromFunction(functionID, IID_IMetaDataImport, (LPUNKNOWN *) &metadataImport, funcToken);
	hr2 = corProfilerInfo->GetFunctionInfo(functionID,functionClassId,functionModuleId,funcToken);	
	
	return SUCCEEDED(hr1) && SUCCEEDED(hr2);
}

static HRESULT GetFullMethodName(ICorProfilerInfo* corProfilerInfo,FunctionID functionID, wstring *methodName, int cMethod)
{
	IMetaDataImport* pIMetaDataImport = nullptr;
	HRESULT hr = S_OK;
	mdToken funcToken = 0;
	WCHAR szFunction[NAME_BUFFER_SIZE];
	WCHAR szClass[NAME_BUFFER_SIZE];
	
	hr = corProfilerInfo->GetTokenAndMetaDataFromFunction(functionID, IID_IMetaDataImport, (LPUNKNOWN *) &pIMetaDataImport, &funcToken);
	if(SUCCEEDED(hr))
	{
		mdTypeDef classTypeDef;
		ULONG cchFunction;
		ULONG cchClass;
		
		hr = pIMetaDataImport->GetMethodProps(funcToken, &classTypeDef, szFunction, NAME_BUFFER_SIZE, &cchFunction, 0, 0, 0, 0, 0);
		if (SUCCEEDED(hr))
		{
			
			hr = pIMetaDataImport->GetTypeDefProps(classTypeDef, szClass, NAME_BUFFER_SIZE, &cchClass, 0, 0);
			if (SUCCEEDED(hr))
			{
				// create the fully qualified name
				wstringstream methodNameStream;
				methodNameStream << szClass << "." << szFunction;
				*methodName = methodNameStream.str();
			}			
		}
		// release our reference to the metadata
		pIMetaDataImport->Release();
	}

	return hr;
}
};


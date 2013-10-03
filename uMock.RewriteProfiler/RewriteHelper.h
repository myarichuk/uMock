#pragma once
#include "stdafx.h"

//a class that contains methods for finding which methods to replace and rewrites IL during JIT
class CRewriteHelper
{
private:
	ICorProfilerInfo *m_CorProfilerInfo;
	CLogger m_Logger;	
protected:

	void* AllocateMethodBody(ModuleID moduleId, ULONG newMethodSize)
	{
	    IMethodMalloc* methodMalloc = NULL;
		if(m_CorProfilerInfo->GetILFunctionBodyAllocator(moduleId, &methodMalloc) != S_OK)
		{
			if(methodMalloc != NULL)
			{
				methodMalloc->Release();
			}
			return nullptr;
		}
		
		void *result = methodMalloc->Alloc(newMethodSize);	
	    ZeroMemory(result, newMethodSize);	    

	    methodMalloc->Release();
	    return result;
	}
public:
	void AddMethodToRewriteListIfRelevant(MethodData &methodData)
	{
		if(methodData.MethodName == L"")
		{
		}
	}

	void RewriteMethod(MethodData &methodData)
	{		
		LPCBYTE pMethodHeader = NULL;
		ULONG methodSize;
		HRESULT hr = m_CorProfilerInfo->GetILFunctionBody(methodData.ModuleId,methodData.MethodToken,&pMethodHeader,&methodSize);
		if(SUCCEEDED(hr))
		{
			
			IMAGE_COR_ILMETHOD* pMethod = (IMAGE_COR_ILMETHOD*)pMethodHeader;			
 			IMAGE_COR_ILMETHOD_FAT* fatImage = (IMAGE_COR_ILMETHOD_FAT*)&pMethod->Fat;

		}
	}
};


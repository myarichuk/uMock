// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "RewriteProfilerClassFactory.h"
#include "RegistryHelper.hpp"
#include "Logger.hpp"

HINSTANCE g_ThisDll = NULL;        
UINT      g_DllLockCount = 0; // # of COM objects in existence

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
			g_ThisDll = (HMODULE)hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

STDAPI DllGetClassObject(const CLSID& clsid,
						 const IID& iid,
						 void** ppv)
{

	CLogger logger;	
	logger.Debug("DllGetClassObject() starting");	
	if(__uuidof(CRewriteProfiler) != clsid)
	{
        return CLASS_E_CLASSNOTAVAILABLE;
	}

	if(IsBadWritePtr(ppv, sizeof(void*)))
	{
        return E_POINTER;
	}

	*ppv = NULL;
	auto pClassFactory = new CRewriteProfilerClassFactory;
	if(pClassFactory == NULL)
	{
		return E_OUTOFMEMORY;
	}	
	
	logger.Debug("Class factory querying interface...");
	HRESULT hrRet = pClassFactory->QueryInterface(iid, ppv);
	
	if(SUCCEEDED(hrRet))
	{
		logger.Debug("DllGetClassObject() succeeded");	
	}
	else
	{
		logger.Error(L"DllGetClassObject() failed");
		delete pClassFactory;
	}
	return hrRet;
}

STDAPI DllCanUnloadNow()
{
	return (g_DllLockCount == 0) ? S_OK : S_FALSE;
}

STDAPI DllRegisterServer()
{
#ifdef _WIN64
	return CRegistryHelper::RegisterCOMClass(__uuidof(CRewriteProfiler),L"uMock.Profiler64",L"uMock Profiler 64",L"Free");
#else
	return CRegistryHelper::RegisterCOMClass(__uuidof(CRewriteProfiler),L"uMock.Profiler",L"uMock Profiler",L"Free");
#endif
}

STDAPI DllUnregisterServer()
{
	return CRegistryHelper::UnregisterCOMClass(__uuidof(CRewriteProfiler));
}
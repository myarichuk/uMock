#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// constructor/destructor
CRewriteProfiler::CRewriteProfiler()
{	
	m_nRefCount = 0;
	m_Logger.Debug("CRewriteProfiler::CRewriteProfiler()");
    InterlockedIncrement(&g_DllLockCount);
}

CRewriteProfiler::~CRewriteProfiler()
{
	m_Logger.Debug("CRewriteProfiler::~CRewriteProfiler()");
	SAFE_RELEASE(m_corProfilerInfo);
	SAFE_RELEASE(m_corProfilerInfo2);

	ReleaseMetadataCache();
	InterlockedDecrement(&g_DllLockCount);
}


//////////////////////////////////////////////////////////////////////
// ICorProfilerCallback3 Implementation

STDMETHODIMP CRewriteProfiler::Shutdown()
{
	m_Logger.Debug("CRewriteProfiler::Shutdown()");

	return S_OK; 
}

STDMETHODIMP CRewriteProfiler::QueryInterface(
   REFIID riid , 
   void **ppObj)
{ 	
	wstring guidAsString;
	CComUtils::StringFromCLSID(riid,&guidAsString);
	
	m_Logger.Debug(L"CRewriteProfiler::QueryInterface() (GUID " + guidAsString + L")");

	HRESULT hrRet = S_OK;

	if(InlineIsEqualGUID(riid, IID_IUnknown))
    {
		*ppObj = static_cast<IUnknown*>(this);
		m_Logger.Debug("CRewriteProfiler::QueryInterface() found interface --> IUnknown");
	}
	else if(InlineIsEqualGUID(riid, IID_ICorProfilerCallback))
    {
		*ppObj = static_cast<ICorProfilerCallback*>(this);
		m_Logger.Debug("CRewriteProfiler::QueryInterface() found interface --> IID_ICorProfilerCallback");
	}
	else if(InlineIsEqualGUID(riid, IID_ICorProfilerCallback2))
    {
		*ppObj = static_cast<ICorProfilerCallback2*>(this);
		m_Logger.Debug("CRewriteProfiler::QueryInterface() found interface --> IID_ICorProfilerCallback2");
	}
	else if(InlineIsEqualGUID(riid, IID_ICorProfilerCallback3))
    {
		*ppObj = static_cast<ICorProfilerCallback3*>(this);
		m_Logger.Debug("CRewriteProfiler::QueryInterface() found interface --> IID_ICorProfilerCallback3");
	}
	else
	{
		m_Logger.Warn(L"CRewriteProfiler::QueryInterface() not found requested interface (GUID " + guidAsString + L")");
		hrRet = E_NOINTERFACE;
	}

	if(hrRet == S_OK)
	{
		reinterpret_cast<IUnknown*>(*ppObj)->AddRef();
	}

	return hrRet ;
}

ULONG STDMETHODCALLTYPE CRewriteProfiler::AddRef()
{
   long nRefCount = InterlockedIncrement(&m_nRefCount);

   stringstream debugStringStream;
   debugStringStream << "CRewriteProfiler::AddRef() (CRewriteProfiler reference count = " << m_nRefCount << " )";
   m_Logger.Debug(debugStringStream.str());

   return nRefCount;
}

ULONG STDMETHODCALLTYPE CRewriteProfiler::Release()
{     
   long nRefCount = InterlockedDecrement(&m_nRefCount);

   if(nRefCount == 0)
   {
		m_Logger.Debug("CRewriteProfiler::Release() --> Destroying profiler instance (CRewriteProfiler reference count = 0)");
		delete this;
		CLogger::Shutdown();
   }
   else
   {
		stringstream debugStringStream;
		debugStringStream << "CRewriteProfiler::Release() (CRewriteProfiler reference count = " << nRefCount << " )";
	
		m_Logger.Debug(debugStringStream.str());
   }
   return nRefCount;
}

STDMETHODIMP CRewriteProfiler::Initialize(IUnknown *pICorProfilerInfoUnk)
{			
	m_Logger.Debug("CRewriteProfiler::Initialize()");

	HRESULT hrRet = pICorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo, (void**)&m_corProfilerInfo);
	if (FAILED(hrRet))
	{
		m_Logger.Error("Failed to get ICorProfilerInfo");
		return E_FAIL;
	}
	else
	{
		m_Logger.Debug("Retrieved ICorProfilerInfo");		
	}

	hrRet = pICorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo2, (void**)&m_corProfilerInfo2);
	if (FAILED(hrRet))
	{
		m_corProfilerInfo2 = nullptr;
		m_Logger.Error("Failed to get ICorProfiler2. not supported?");
		m_CanUseV2Interfaces = false;
	}
	else
	{
		m_Logger.Debug("Retrieved ICorProfilerInfo2");
		m_CanUseV2Interfaces = true;
	}

	//set event mask for IL rewriting (declare to which events the profiler will be listening)
	DWORD eventMask = COR_PRF_MONITOR_NONE;
	eventMask |= COR_PRF_MONITOR_JIT_COMPILATION; 
	eventMask |= COR_PRF_MONITOR_MODULE_LOADS;
	eventMask |= COR_PRF_MONITOR_ASSEMBLY_LOADS;
	eventMask |= COR_PRF_DISABLE_INLINING;
	eventMask |= COR_PRF_DISABLE_OPTIMIZATIONS;
	hrRet = m_corProfilerInfo->SetEventMask(eventMask);

	if(hrRet == S_OK)
	{
		m_Logger.Debug("Initialized profiling.. (SetEventMask method executed succesfully)");
	}
	else
	{
		m_Logger.Error("Error initializing profiling.. (SetEventMask method executed with error)");
	}

	return S_OK;
}

STDMETHODIMP CRewriteProfiler::JITCompilationStarted(FunctionID functionID, BOOL fIsSafeToBlock)
{
	m_Logger.Debug("CRewriteProfiler::JITCompilationStarted()");
	wstring methodName;	

	MetadataHelper::GetFullMethodName(m_corProfilerInfo,functionID,&methodName,1);

	m_Logger.Debug(wstring(L"JIT compilation started. Method Name = ") + methodName);
	return S_OK;
}

STDMETHODIMP CRewriteProfiler::ModuleLoadFinished(ModuleID moduleID, HRESULT hrStatus)
{
	HRESULT hr = S_OK;
	wstringstream debugMessageStream;
	IMetaDataImport2* moduleMetadataImport = nullptr;
	AssemblyID moduleAssemblyId;
	wstring moduleName;
	
	if(SUCCEEDED(MetadataHelper::GetModuleInfo(m_corProfilerInfo,moduleID,&moduleName,&moduleAssemblyId)))
	{
		debugMessageStream << L"Module loaded (module name = " << moduleName << L", loading status = "<< hrStatus << L")";
		m_Logger.Debug(debugMessageStream.str());
	}


	if(m_CanUseV2Interfaces && 
	   SUCCEEDED(MetadataHelper::GetMetadataImportByModuleID(m_corProfilerInfo2,moduleID,&moduleMetadataImport)))
	{
		if(moduleMetadataImport == nullptr)
		{
			m_Logger.Debug("Module metadata is null,cannot add to metadata cache");
		}
		else
		{
			AddMetadataToCacheIfNeeded(moduleID,moduleMetadataImport);
			ParseAndCacheTypeDefinitions(moduleMetadataImport);
			ParseAndCacheTypeReferences(moduleMetadataImport);			
		}
	}

	return hr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helpers

void CRewriteProfiler::ParseAndCacheTypeReferences(IMetaDataImport2 *metadataImport)
{
	vector<mdTypeRef> typeReferencesCollection;
	MetadataHelper::GetTypeReferences(metadataImport,&typeReferencesCollection);

	stringstream debugMessageStream;
	debugMessageStream << "Adding module's type references to cache. (type count = " << typeReferencesCollection.size() << ")";
	m_Logger.Debug(debugMessageStream.str());

	m_TypeReferenceTokens.insert(m_TypeReferenceTokens.end(),typeReferencesCollection.begin(),typeReferencesCollection.end());
}

void CRewriteProfiler::ParseAndCacheTypeDefinitions(IMetaDataImport2 *metadataImport)
{
	vector<mdTypeDef> typeDefinitionsCollection;
	MetadataHelper::GetTypeDefinitions(metadataImport,&typeDefinitionsCollection);
		
	stringstream debugMessageStream;
	debugMessageStream << "Adding module's type definitions to cache. (type count = " << typeDefinitionsCollection.size() << ")";
	m_Logger.Debug(debugMessageStream.str());

	m_TypeDefinitionTokens.insert(m_TypeDefinitionTokens.end(),typeDefinitionsCollection.begin(),typeDefinitionsCollection.end());
}

void CRewriteProfiler::AddMetadataToCacheIfNeeded(ModuleID moduleId,IMetaDataImport2 *metadataImport)
{	
	//if moduleId key does not exists
	if(m_MetadataImportByModuleId.find(moduleId) == m_MetadataImportByModuleId.end())
	{					
		stringstream debugMessageStream;
		
		debugMessageStream << "Adding entry to metadata cache, (module id = " << moduleId << ")";
		m_Logger.Debug(debugMessageStream.str());
		m_MetadataImportByModuleId.insert(pair<ModuleID,IMetaDataImport2*>(moduleId,metadataImport));
		
	}
}

void CRewriteProfiler::ReleaseMetadataCache()
{
	for(auto iterator=m_MetadataImportByModuleId.begin(); iterator!=m_MetadataImportByModuleId.end(); ++iterator)
	{
		SAFE_RELEASE((*iterator).second);		
	}
}
#pragma once
#include "BaseProfiler.h"
#include "Logger.hpp"
#include "MetadataHelper.hpp"

class CRewriteProfiler : public BaseProfiler
{
private:
// Member Fields
	long m_nRefCount;
	bool m_CanUseV2Interfaces;

	ICorProfilerInfo* m_corProfilerInfo;
	ICorProfilerInfo2* m_corProfilerInfo2;
	CLogger m_Logger;	

	map<ModuleID,IMetaDataImport2*> m_MetadataImportByModuleId;
	vector<mdTypeDef> m_TypeDefinitionTokens;
	vector<mdTypeRef> m_TypeReferenceTokens;
private:
	//helpers
	void AddMetadataToCacheIfNeeded(ModuleID moduleId,IMetaDataImport2 *metadataImport);
	void ParseAndCacheTypeDefinitions(IMetaDataImport2 *metadataImport);
	void ParseAndCacheTypeReferences(IMetaDataImport2 *metadataImport);
	void ReleaseMetadataCache();
public:
	CRewriteProfiler(void);
	~CRewriteProfiler(void);

	STDMETHOD(QueryInterface)(REFIID riid, void **ppObj);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	//overriden member methods
	STDMETHOD(Initialize)(IUnknown *pICorProfilerInfoUnk);
	STDMETHOD(Shutdown)();

	STDMETHOD(JITCompilationStarted)(FunctionID functionID, BOOL fIsSafeToBlock);
	STDMETHOD(ModuleLoadFinished)(ModuleID moduleID, HRESULT hrStatus);
};


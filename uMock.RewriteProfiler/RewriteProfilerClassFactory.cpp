#include "stdafx.h"
#include "RewriteProfilerClassFactory.h"
#include "RewriteProfiler.h"

CRewriteProfilerClassFactory::CRewriteProfilerClassFactory()
{	
    m_nRefCount = 0;    
    InterlockedIncrement(&g_DllLockCount);
}

CRewriteProfilerClassFactory::~CRewriteProfilerClassFactory()
{
	InterlockedDecrement(&g_DllLockCount);	
}

ULONG STDMETHODCALLTYPE CRewriteProfilerClassFactory::AddRef()
{
   return InterlockedIncrement(&m_nRefCount);
}

ULONG STDMETHODCALLTYPE CRewriteProfilerClassFactory::Release()
{     
   long nRefCount=0;
   nRefCount=InterlockedDecrement(&m_nRefCount);
   if(nRefCount == 0)
   {
	   delete this;
   }

   return nRefCount;
}

STDMETHODIMP CRewriteProfilerClassFactory::QueryInterface(REFIID riid, void** ppObj)
{	
	HRESULT hrRet = S_OK;
	wstring guidAsString;
	CComUtils::StringFromCLSID(riid,&guidAsString);
	
	m_Logger.Debug("CRewriteProfilerClassFactory::QueryInterface()");
	if(InlineIsEqualGUID(riid, IID_IUnknown))
    {
		*ppObj = static_cast<IUnknown*>(this);
        m_Logger.Debug("CRewriteProfilerClassFactory::QueryInterface() IUnknown");
	}
    else if(InlineIsEqualGUID(riid, IID_IClassFactory))
    {
        *ppObj = static_cast<IClassFactory*>(this);
        m_Logger.Debug("CRewriteProfilerClassFactory::QueryInterface() initializes class factory");
	}
	else
	{
		hrRet = E_NOINTERFACE;
		m_Logger.Warn("CRewriteProfilerClassFactory::QueryInterface() results in E_NOINTERFACE");
	}

	if(hrRet == S_OK)
	{
		reinterpret_cast<IUnknown*>(*ppObj)->AddRef();
        m_Logger.Debug("CRewriteProfilerClassFactory::QueryInterface() found class for supplied GUID");
	}

	return hrRet;
}

// IClassFactory methods
STDMETHODIMP CRewriteProfilerClassFactory::CreateInstance(IUnknown* pUnkOuter,
                                                         REFIID    riid,
                                                         void**    ppObj)
{	
	wstring guidAsString;
	CComUtils::StringFromCLSID(riid,&guidAsString);
	
	m_Logger.Debug(L"CRewriteProfilerClassFactory::CreateInstance() starting (GUID " + guidAsString + L")");    

	HRESULT hrRet;
	CRewriteProfiler *pCRewriteProfiler;

	//no support for aggregation
    if(NULL != pUnkOuter)
	{
        return CLASS_E_NOAGGREGATION;
	}

    if(IsBadWritePtr(ppObj, sizeof(void*)))
	{
        return E_POINTER;
	}

    *ppObj = NULL;

	try
	{
		pCRewriteProfiler = new CRewriteProfiler;
	}
	catch(exception e)
	{
		cerr<<"Error instantiating CRewriteProfiler. Reason : " << e.what() << endl;
	}

    if(pCRewriteProfiler == NULL)
	{
        return E_OUTOFMEMORY;
	}

    hrRet = pCRewriteProfiler->QueryInterface(riid, ppObj);

    if(FAILED(hrRet))
	{
		m_Logger.Error("CRewriteProfilerClassFactory::CreateInstance() failed");
        delete pCRewriteProfiler;
	}

	m_Logger.Debug("CRewriteProfilerClassFactory::CreateInstance() executed");
    return hrRet;
}

STDMETHODIMP CRewriteProfilerClassFactory::LockServer(BOOL fLock)
{
    // Increase/decrease DLL ref count, according to the fLock param.
    fLock ? InterlockedIncrement(&g_DllLockCount) : InterlockedDecrement(&g_DllLockCount);
    return S_OK;
}
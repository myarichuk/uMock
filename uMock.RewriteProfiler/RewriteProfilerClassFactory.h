#pragma once
#include "Logger.hpp"

//class factory for CRewriteProfiler
class CRewriteProfilerClassFactory : public IClassFactory
{
protected:
    ULONG m_nRefCount;	
	CLogger m_Logger;
public:
//Constructor / Destructor
    CRewriteProfilerClassFactory();
    virtual ~CRewriteProfilerClassFactory();

// IUnknown implementation
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void** ppObj);

// IClassFactory implementation
    STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID riid, void** ppObj);
    STDMETHOD(LockServer)(BOOL fLock);

};


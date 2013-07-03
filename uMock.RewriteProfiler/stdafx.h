#pragma once

// length of array
#define ARRAY_LENGTH(s) (sizeof(s) / sizeof(s[0]))

//safely release COM pointer
#define SAFE_RELEASE(x) if(x != NULL && x != nullptr) { x->Release(); x = NULL; }

// project-wide includes
#include "targetver.h"
#include "ComDefinitions.h"

#include <windows.h>
#include <comdef.h>
#include <iostream>
#include <memory>
#include <string>

#include <cor.h>
#include <corprof.h>

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/PatternLayout.hh>


#include "ComUtils.hpp"
#include "RewriteProfiler.h"

//static link library includes
#pragma comment(lib,"CorGuids.lib") //library of .Net Profiling API

#ifdef _WIN64
#pragma comment(lib,"log4cppLIB64.lib") //log4cpp library
#else
#pragma comment(lib,"log4cppLIB.lib") //log4cpp library
#endif

#pragma comment(lib,"ws2_32.lib") //winsockets library, prerequisite of log4cpp

//global variables declaration
extern HINSTANCE g_ThisDll;        
extern UINT      g_DllLockCount; 

using namespace std;


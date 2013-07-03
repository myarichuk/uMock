#pragma once
#include <Windows.h>
#include <string>

using namespace std;
extern HINSTANCE g_ThisDll;

//TODO : refactor this class to register/unregister any COM class
class CRegistryHelper
{
private:
	static void GetMainRegistryKey(REFCLSID rclsid,wstring *guidAsString)
	{
		OLECHAR szWID[64]; 
		StringFromGUID2(rclsid, szWID, ARRAY_LENGTH(szWID));		
		
		*guidAsString = L"CLSID\\" + wstring(szWID);
	}

	static HRESULT DoClassRegistration(const WCHAR* mainRegistryKey,const WCHAR* progId,const WCHAR *szDesc, const WCHAR *szThreadingModel)
	{
		LONG lRet;
		HKEY hCLSIDKey = NULL;
		HKEY hInProcSvrKey = NULL;
		HKEY hProgIdKey = NULL;
		WCHAR szModulePath[MAX_PATH];		

		__try
	    {
			lRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, mainRegistryKey,
								   0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_CREATE_SUB_KEY,
								   NULL, &hCLSIDKey, NULL);    
			if (lRet != ERROR_SUCCESS) 
			{
				//logger.error("error creating main registry key");
				return HRESULT_FROM_WIN32(lRet);
			}		

			lRet = RegSetValueEx(hCLSIDKey, NULL, 0, REG_SZ, (const BYTE*) szDesc,sizeof(szDesc));
			if (lRet != ERROR_SUCCESS) 
			{
				//logger.error("error setting description value to registry key");
				return HRESULT_FROM_WIN32(lRet);
			}

			lRet = RegCreateKeyEx(hCLSIDKey, L"ProgId", 0, NULL, REG_OPTION_NON_VOLATILE,
								   KEY_SET_VALUE, NULL, &hProgIdKey, NULL );
			if (lRet != ERROR_SUCCESS) 
			{
				//logger.error("error creating ProgId registry key");
				return HRESULT_FROM_WIN32(lRet);
			}

			lRet = RegSetValueEx(hProgIdKey, NULL, 0, REG_SZ, (const BYTE*) progId, 
				sizeof(WCHAR) * (lstrlen(progId)+1) );
			if (lRet != ERROR_SUCCESS) 
			{
				//logger.error("error setting ProgId in registry key");
				return HRESULT_FROM_WIN32(lRet);
			}

			lRet = RegCreateKeyEx(hCLSIDKey, L"InProcServer32", 0, NULL, REG_OPTION_NON_VOLATILE,
								   KEY_SET_VALUE, NULL, &hInProcSvrKey, NULL );
			if (lRet != ERROR_SUCCESS) 
			{
				//logger.error("error creating InProcServer32 registry key");
				return HRESULT_FROM_WIN32(lRet);
			}
			
			GetModuleFileName(g_ThisDll, szModulePath, MAX_PATH);

			lRet = RegSetValueEx(hInProcSvrKey, NULL, 0, REG_SZ, (const BYTE*) szModulePath, 
								 sizeof(WCHAR) * (lstrlen(szModulePath)+1) );
			if (lRet != ERROR_SUCCESS) 
			{
				//logger.error("error setting module path in registry key");
				return HRESULT_FROM_WIN32(lRet);
			}

			lRet = RegSetValueEx(hInProcSvrKey, L"ThreadingModel", 0, REG_SZ,
								  (const BYTE*) szThreadingModel, 
								  sizeof(szThreadingModel));
			if (lRet != ERROR_SUCCESS) 
			{
				//logger.error("error setting value to ThreadingModel registry key");
				return HRESULT_FROM_WIN32(lRet);
			}

		}
		__finally
		{
			if ( hCLSIDKey != NULL)
			{
				RegCloseKey ( hCLSIDKey );
			}

		    if ( hInProcSvrKey != NULL )
			{
		        RegCloseKey ( hInProcSvrKey );
			}
		}

		//if reached this line then all is ok
		return S_OK;
	}

	static LONG GetStringRegKey(HKEY hKey, const wstring strValueName, 
										  wstring *strValue)
	{
	    WCHAR szBuffer[512];
	    DWORD dwBufferSize = sizeof(szBuffer);
	    ULONG nError;
	    nError = RegQueryValueEx(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
	    if (ERROR_SUCCESS == nError)
	    {
			*strValue = wstring(szBuffer);
		}
		return nError;
	}

public:
	static HRESULT UnregisterCOMClass(REFCLSID rclsid)
	{
		wstring mainRegistryKey; 
		wstring inprocServerKey(L"InProcServer32");
		wstring progIdKey(L"ProgId");
		LONG lRet;

        GetMainRegistryKey(rclsid,&mainRegistryKey);

		lRet = RegDeleteKey(HKEY_CLASSES_ROOT, (mainRegistryKey + wstring(L"\\") + inprocServerKey).c_str());
		if (lRet != ERROR_SUCCESS) 
		{
			//logger.error("error deleting InProcServer32 registry key (" + mainRegistryKey + wstring("\\") + inprocServerKey + ")");
			return HRESULT_FROM_WIN32(lRet);
		}
		
		lRet = RegDeleteKey(HKEY_CLASSES_ROOT, (mainRegistryKey + wstring(L"\\") + progIdKey).c_str());
		if (lRet != ERROR_SUCCESS) 
		{
			//logger.error("error deleting ProgId registry key (" + mainRegistryKey + wstring("\\") + progIdKey + ")");
			return HRESULT_FROM_WIN32(lRet);
		}

		lRet = RegDeleteKey(HKEY_CLASSES_ROOT, mainRegistryKey.c_str());
		if (lRet != ERROR_SUCCESS) 
		{
            //logger.error("error deleting main registry key (" + wstring(mainRegistryKey) + ")");
			return HRESULT_FROM_WIN32(lRet);
		}

		//logger.info("unregistered COM InProc server (guid : " + mainRegistryKey + ")");
		return S_OK;
	}

	static HRESULT RegisterCOMClass(REFCLSID rclsid,const WCHAR* progId,const WCHAR *szDesc, const WCHAR *szThreadingModel)
	{
		wstring mainRegistryKeyString;		
		GetMainRegistryKey(rclsid,&mainRegistryKeyString);		

		HRESULT hrRet = DoClassRegistration(mainRegistryKeyString.c_str(),progId,szDesc,szThreadingModel);
		if(hrRet == S_OK)
		{
			//logger.info("registered COM InProc server (registry key : " + mainRegistryKeyString + ")");
		}
		else
		{
			//logger.error("failed registering COM InProc server (registry key : " + mainRegistryKeyString + ")");
		}

		return hrRet;
	}

	static HRESULT GetCOMClassLocation(REFCLSID rclsid,wstring *locationPath)
	{
		wstring mainRegistryKeyString;			
		GetMainRegistryKey(rclsid,&mainRegistryKeyString);		

		HKEY hMainRegistryKey;		
		HKEY hInProcSvrKey;
		LONG lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, mainRegistryKeyString.c_str(), 0, KEY_READ, &hMainRegistryKey);		
		if(lResult != ERROR_SUCCESS)
		{
			RegCloseKey(hMainRegistryKey);
			return HRESULT_FROM_WIN32(lResult);
		}

		lResult = RegOpenKeyEx(hMainRegistryKey, L"InProcServer32", 0, KEY_READ, &hInProcSvrKey);				
		if(lResult != ERROR_SUCCESS)
		{
			RegCloseKey(hMainRegistryKey);
			RegCloseKey(hInProcSvrKey);
			return HRESULT_FROM_WIN32(lResult);
		}

		wstring comClassPath;				
		GetStringRegKey(hInProcSvrKey,L"",&comClassPath);
		size_t indexOfLastSlash = comClassPath.find_last_of(L"\\");		
		
		*locationPath = comClassPath.substr(0,indexOfLastSlash + 1);
		
		RegCloseKey(hMainRegistryKey);
		RegCloseKey(hInProcSvrKey);

		return S_OK;
	}
};
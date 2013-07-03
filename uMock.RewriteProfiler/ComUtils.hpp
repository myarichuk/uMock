#pragma once
#include "stdafx.h"

//class that contains various utility functions - that are COM related
class CComUtils
{
public:
	//retrieve string representation of GUID
	static void StringFromCLSID(REFCLSID rclsid,std::wstring *guidAsString)
	{
		OLECHAR szWID[64];
		StringFromGUID2(rclsid, szWID, ARRAY_LENGTH(szWID));		
		*guidAsString = std::wstring(szWID);
	}

	static std::wstring StringtoWString(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
		std::wstring r(len, L'\0');
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, &r[0], len);
		return r;
	}

	static std::string WStringToString(const std::wstring& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0); 
		std::string r(len, '\0');
		WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0); 
		return r;
	}
};

#pragma once
#include "..\Library\precompile.h"

namespace winapi {
	inline std::string wide2string(WCHAR* input, int size) {
		char* ch = new char[size-1];
		WideCharToMultiByte(CP_ACP, 0, input, -1, ch, size, NULL, NULL);
		return std::string(ch);
	}

	namespace windows {
		inline std::string windows_version() {
			HKEY keyHandle;
			WCHAR rgValue[1024];
			WCHAR fnlRes[1024];
			DWORD size = 1023;
			DWORD Type;

			if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &keyHandle) == ERROR_SUCCESS)
			{
				RegQueryValueExW(keyHandle, L"ProductName", NULL, &Type, (LPBYTE)rgValue, &size);
				RegCloseKey(keyHandle);
				return wide2string(rgValue, size+1);
			}
			else {
				return "ERROR";
			}
		}
	}
}
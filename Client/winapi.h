#pragma once
#include "..\Library\precompile.h"

namespace winapi {
	inline std::string wide2string(WCHAR* input, int size) {
		char* ch = new char[size - 1];
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
				return wide2string(rgValue, size + 1);
			}
			else {
				return "ERROR";
			}
		}

		inline std::string windows_dir() {

		}

	}

	namespace registry {
		// Currently only works with REG_DWORD and REG_SZ
		inline std::string RegValue(HKEY registry_hive, std::string key_name, std::string value) { 

			// Get Registry Size and Type
			unsigned long value_type{};
			unsigned long value_size{};
			if (!RegGetValueA(registry_hive, key_name.c_str(), value.c_str(), RRF_RT_ANY, &value_type, NULL, &value_size)) {

				switch (value_type) {
				case REG_DWORD:
				{
					DWORD buffer;
					if (!RegGetValueA(registry_hive, key_name.c_str(), value.c_str(), RRF_RT_ANY, &value_type, &buffer, &value_size)) {
						return std::to_string(buffer);
					}
				}
				case REG_SZ:
				case REG_MULTI_SZ:
				case REG_EXPAND_SZ:
				{
					std::vector<unsigned char> buffer(value_size);
					if (!RegGetValueA(registry_hive, key_name.c_str(), value.c_str(), RRF_RT_ANY, &value_type, buffer.data(), &value_size)) {
						return std::string(buffer.begin(), buffer.end());
					}
				}
				default:
					break;
				}
			}
			else {
				std::cout << "Error: The System cannot find the registry entry!" << std::endl;
			}

			return "";

		}
	}

}
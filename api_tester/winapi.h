#pragma once
#define _WIN32_DCOM
#include "..\Library\precompile.h"
#include <comdef.h>
#include <WbemIdl.h>
#pragma comment(lib,  "wbemuuid.lib")
#pragma comment(lib,  "shell32.lib")
//#pragma comment(lib,  "Netapi32.lib")
#include "hps/hps.h"
#include <Shlobj.h>
#include <numeric>
#include <LM.h>

namespace winapi {

	namespace convert {

		inline std::string wideTstring(const wchar_t* pstr, long wslen)
		{
			int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);

			std::string dblstr(len, '\0');
			len = ::WideCharToMultiByte(CP_ACP, 0 /* no flags */,
				pstr, wslen /* not necessary NULL-terminated */,
				&dblstr[0], len,
				NULL, NULL /* no default char */);

			return dblstr;
		}

		inline std::string bstr(BSTR bstr) {
			int wslen = ::SysStringLen(bstr);
			return wideTstring((wchar_t*)bstr, wslen);
		}
	}

	namespace registry {
		// Currently only works with REG_DWORD and REG_SZ
		inline std::string GetValue(HKEY root_key, std::string key_name, std::string value) {
			// Get Registry Size and Type
			unsigned long value_type{};
			unsigned long value_size{};


			LSTATUS ret = RegGetValueA(root_key, key_name.c_str(), value.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, &value_type, NULL, &value_size);
			if (!ret) {

				switch (value_type) {
				case REG_DWORD:
				{
					DWORD buffer;
					if (!RegGetValueA(root_key, key_name.c_str(), value.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, &value_type, &buffer, &value_size)) {
						return std::to_string(buffer);
					}
				}
				case REG_SZ:
				case REG_MULTI_SZ:
				case REG_EXPAND_SZ:
				{
					std::vector<unsigned char> buffer(value_size);
					if (!RegGetValueA(root_key, key_name.c_str(), value.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, &value_type, buffer.data(), &value_size)) {
						return (buffer[0] != '\0') ? std::string(buffer.begin(), buffer.end()) : "null";
					}
				}
				default:
					break;
				}
			}
			else {
				std::cout << "Error: The System cannot find the registry entry!" << std::endl;
			}

			return std::to_string(ret);

		}
		inline bool exists(HKEY hive, std::string key_name, std::string value) {
			return RegGetValueA(hive, key_name.c_str(), value.c_str(), RRF_RT_ANY, NULL, NULL, NULL) != ERROR_FILE_NOT_FOUND;
		}
	}

	namespace computer {
		namespace info {
			inline std::string windows_product() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "productname");
			}

			inline std::string windows_root() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "systemroot");
			}

			inline std::string windows_owner() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "RegisteredOwner");
			}

			inline std::string windows_organization() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "RegisteredOrganization");
			}

			inline std::string windows_architecture() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", "PROCESSOR_ARCHITECTURE");
			}

			inline std::string windows_user() {
				std::vector<char> buffer(UNLEN + 1);
				DWORD size = UNLEN + 1;
				if (GetUserNameA(buffer.data(), &size)) {
					return std::string(buffer.begin(), buffer.end());
				}
				else {
					return "null";
				}
			}

			inline std::string computer_name() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName", "ComputerName");
			}

			inline std::string mobo_vendor() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", "SystemManufacturer");
			}

			inline std::string mobo_product() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", "SystemProductName");
			}

			inline std::string bios_vendor() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", "BIOSVendor");
			}

			inline std::string bios_version() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", "BIOSVersion");
			}

			inline std::string bios_release() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", "BIOSReleaseDate");
			}

			inline std::string cpu_name() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString");
			}

			inline std::string cpu_speed() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "~MHz");
			}

			inline std::string ram_size() {
				ULONGLONG kilobytes;
				if (GetPhysicallyInstalledSystemMemory(&kilobytes)) {
					return std::to_string(kilobytes / (1024 * 1024));
				}
				else {
					return "unknown";
				}
			}

			inline std::string video_adapter() {
				return registry::GetValue(HKEY_LOCAL_MACHINE, registry::GetValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\VIDEO", "\\Device\\Video0").substr(18), "DriverDesc");
			}

			inline std::string user_groups() {
				wchar_t user[UNLEN];
				DWORD size = sizeof(user) / sizeof(user[0]);
				GetUserNameW(user, &size);
				LPBYTE buffer;
				DWORD entries, total_entries;

				NetUserGetLocalGroups(NULL, user, 0, LG_INCLUDE_INDIRECT, &buffer, MAX_PREFERRED_LENGTH, &entries, &total_entries);

				LOCALGROUP_USERS_INFO_0* groups = (LOCALGROUP_USERS_INFO_0*)buffer;
				std::vector<std::string> group;
				for (int i = 0; i < entries; i++) {
					group.push_back(convert::wideTstring(groups[i].lgrui0_name, MAX_PREFERRED_LENGTH) + " ");
				}
				NetApiBufferFree(buffer);

				return std::accumulate(group.begin(), group.end(), std::string{});
			}
		}
	}

}
#pragma once
#include "..\Library\precompile.h"

namespace winapi {

	namespace registry {
		// Currently only works with REG_DWORD and REG_SZ
		inline std::string GetValue(HKEY registry_hive, std::string key_name, std::string value) {
			// Get Registry Size and Type
			unsigned long value_type{};
			unsigned long value_size{};


			LSTATUS ret = RegGetValueA(registry_hive, key_name.c_str(), value.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, &value_type, NULL, &value_size);
			if (!ret) {

				switch (value_type) {
				case REG_DWORD:
				{
					DWORD buffer;
					if (!RegGetValueA(registry_hive, key_name.c_str(), value.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, &value_type, &buffer, &value_size)) {
						return std::to_string(buffer);
					}
				}
				case REG_SZ:
				case REG_MULTI_SZ:
				case REG_EXPAND_SZ:
				{
					std::vector<unsigned char> buffer(value_size);
					if (!RegGetValueA(registry_hive, key_name.c_str(), value.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, &value_type, buffer.data(), &value_size)) {
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

			return std::to_string(ret);

		}
		inline bool exists(HKEY hive, std::string key_name, std::string value) {
			return RegGetValueA(hive, key_name.c_str(), value.c_str(), RRF_RT_ANY, NULL, NULL, NULL) != ERROR_FILE_NOT_FOUND;
		}
	}

	namespace windows {
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
		}
	}

}
#pragma once
#include "..\Library\precompile.h"

namespace winapi {

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

		}
	}

}
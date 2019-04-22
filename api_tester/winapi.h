#pragma once
#define _WIN32_DCOM
#include "..\Library\precompile.h"
#include <comdef.h>
#include <WbemIdl.h>
#pragma comment(lib,  "wbemuuid.lib")
#include <msclr/marshal_cppstd.h>
#include "hps/hps.h"

using namespace System;
using namespace System::Management;

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
			}

			inline std::pair<std::string, std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>>> query_wmi(std::string class_name) {
				String^ w_class = gcnew String(class_name.c_str());

				ManagementObjectSearcher^ searcher = gcnew ManagementObjectSearcher("root\\CIMV2", "SELECT * FROM " + w_class);
				ManagementObjectCollection ^ collection = searcher->Get();

				std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> query_result;

				try{
					for each (ManagementObject ^ object in collection) {
						std::vector<std::pair<std::string, std::string>> properties;
						for each (PropertyData ^ prop in object->Properties) {

							if (prop->Value != nullptr && prop->Value->ToString() != "") {
								properties.push_back({ msclr::interop::marshal_as<std::string>(prop->Name), msclr::interop::marshal_as<std::string>(prop->Value->ToString()) });
							}			

						}

						String^ name;

						try
						{
							name = object["Name"]->ToString();
						}
						catch(Exception^ e)
						{
							name = "null";
						}

						query_result.push_back({ msclr::interop::marshal_as<std::string>(name), properties });
					}

				}
				catch (ManagementException^ ex) {

				}
				
				return { class_name, query_result };
			}

			inline void print_query(std::pair<std::string, std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>>> input) {
				std::cout << input.first << std::endl << std::endl;
				for (auto i : input.second) {
					std::cout << "    " << i.first << std::endl << std::endl;
					for (auto v : i.second) {
						std::cout << "        " << v.first << " - " << v.second << std::endl;
					}
					std::cout << std::endl;
				}
			}

			inline std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>>>> allof_wmi() {
				std::vector<std::string> all_queries = {
					"Win32_1394Controller", "Win32_1394ControllerDevice", "Win32_Account", "Win32_ACE", "Win32_AllocatedResource", "Win32_AssociatedBattery", "Win32_AssociatedProcessorMemory", "Win32_BaseBoard", "Win32_BaseService", "Win32_Battery", "Win32_BIOS", "Win32_BootConfiguration", "Win32_Bus", "Win32_CacheMemory", "Win32_CDROMDrive", "Win32_ClientApplicationSetting", "Win32_CodecFile", "Win32_ComponentCategory", "Win32_ComputerSystem", "Win32_ComputerSystemProcessor", "Win32_ComputerSystemProduct", "Win32_CurrentProbe", "Win32_Desktop", "Win32_DesktopMonitor", "Win32_DeviceBus", "Win32_DeviceMemoryAddress", "Win32_DeviceSettings", "Win32_DiskDrive", "Win32_DiskDriveToDiskPartition", "Win32_DiskPartition", "Win32_DisplayConfiguration", "Win32_DisplayControllerConfiguration", "Win32_DMAChannel", "Win32_DriverVXD", "Win32_Environment", "Win32_Fan", "Win32_Group", "Win32_GroupUser", "Win32_HeatPipe", "Win32_IDEController", "Win32_IDEControllerDevice", "Win32_InfraredDevice", "Win32_IRQResource", "Win32_Keyboard", "Win32_LogicalDisk", "Win32_LogicalDiskRootDirectory", "Win32_LogicalDiskToPartition", "Win32_LogicalFileAccess", "Win32_LogicalFileAuditing", "Win32_LogicalFileGroup", "Win32_LogicalFileOwner", "Win32_LogicalFileSecuritySetting", "Win32_LogicalMemoryConfiguration", "Win32_LogicalProgramGroup", "Win32_LogicalProgramGroupDirectory", "Win32_LogicalProgramGroupItem", "Win32_LogicalShareAccess", "Win32_LogicalShareAuditing", "Win32_LogicalShareSecuritySetting", "Win32_MemoryArray", "Win32_MemoryArrayLocation", "Win32_MemoryDevice", "Win32_MemoryDeviceArray", "Win32_MemoryDeviceLocation", "Win32_MethodParameterClass", "Win32_MotherboardDevice", "Win32_NetworkAdapter", "Win32_NetworkAdapterConfiguration", "Win32_NetworkAdapterSetting", "Win32_NetworkClient", "Win32_NetworkConnection", "Win32_NetworkLoginProfile", "Win32_NetworkProtocol", "Win32_OnBoardDevice", "Win32_OperatingSystem", "Win32_OperatingSystemQFE", "Win32_OSRecoveryConfiguration", "Win32_PageFile", "Win32_PageFileElementSetting", "Win32_PageFileSetting", "Win32_PageFileUsage", "Win32_ParallelPort", "Win32_PCMCIAController", "Win32_Perf", "Win32_PerfRawData", "Win32_PhysicalMemory", "Win32_PhysicalMemoryArray", "Win32_PhysicalMemoryLocation", "Win32_PointingDevice", "Win32_PortableBattery", "Win32_PortConnector", "Win32_PortResource", "Win32_POTSModem", "Win32_POTSModemToSerialPort", "Win32_PowerManagementEvent", "Win32_Printer", "Win32_PrinterConfiguration", "Win32_PrinterController", "Win32_PrinterDriverDll", "Win32_PrinterSetting", "Win32_PrinterShare", "Win32_PrintJob", "Win32_PrivilegesStatus", "Win32_Process", "Win32_Processor", "Win32_ProcessStartup", "Win32_ProtocolBinding", "Win32_QuickFixEngineering", "Win32_Refrigeration", "Win32_Registry", "Win32_ScheduledJob", "Win32_SCSIController", "Win32_SCSIControllerDevice", "Win32_SecurityDescriptor", "Win32_SerialPort", "Win32_SerialPortConfiguration", "Win32_SerialPortSetting", "Win32_Service", "Win32_Share", "Win32_ShareToDirectory", "Win32_SID", "Win32_SMBIOSMemory", "Win32_SoundDevice", "Win32_StartupCommand", "Win32_SystemAccount", "Win32_SystemBIOS", "Win32_SystemBootConfiguration", "Win32_SystemDesktop", "Win32_SystemDevices", "Win32_SystemDriver", "Win32_SystemEnclosure", "Win32_SystemLoadOrderGroups", "Win32_SystemLogicalMemoryConfiguration", "Win32_SystemMemoryResource", "Win32_SystemNetworkConnections", "Win32_SystemOperatingSystem", "Win32_SystemPartitions", "Win32_SystemProcesses", "Win32_SystemProgramGroups", "Win32_SystemServices", "Win32_SystemSetting", "Win32_SystemSlot", "Win32_SystemTimeZone", "Win32_SystemUsers", "Win32_TapeDrive", "Win32_TemperatureProbe", "Win32_Thread", "Win32_TimeZone", "Win32_Trustee", "Win32_UninterruptiblePowerSupply", "Win32_USBController", "Win32_USBControllerDevice", "Win32_UserAccount", "Win32_UserDesktop", "Win32_VideoConfiguration", "Win32_VideoController", "Win32_VideoSettings", "Win32_VoltageProbe", "Win32_WMIElementSetting", "Win32_WMISetting"
				};

				std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>>>> resultant;

				for (auto query : all_queries) {
					//std::cout << "[~] " << query;
					resultant.push_back(query_wmi(query));
					//std::cout << " [+]" << std::endl;
				}

				return resultant;

			}


			//inline void all_info() {//d::tuple<bool, std::vector<std::pair<std::string, std::string>>> all_info() {
			//	long result;
			//	IWbemLocator* wbem_loc = 0;
			//	IWbemServices* wbem_ser = 0;
			//	IEnumWbemClassObject* p_enum = 0;
			//	IWbemClassObject* p_obj = 0;
			//	unsigned long return_result = 0;
			//	SAFEARRAY* ps_names = 0;
			//	long lower, upper;
			//	BSTR prop_name = 0;
			//	std::vector<std::pair<std::string, std::vector<std::string>>> query_properties;
			//	result = CoInitializeEx(0, COINIT_MULTITHREADED);

			//	std::vector<std::string> all_queries = {
			//		"Win32_1394Controller", "Win32_1394ControllerDevice", "Win32_Account", "Win32_ACE", "Win32_ApplicationCommandLine", "Win32_ApplicationService", "Win32_AssociatedBattery", "Win32_AssociatedProcessorMemory", "Win32_BaseBoard", "Win32_BaseService", "Win32_Battery", "Win32_Binary", "Win32_BindImageAction", "Win32_BIOS", "Win32_BootConfiguration", "Win32_Bus", "Win32_CacheMemory", "Win32_CDROMDrive", "Win32_CIMLogicalDeviceCIMDataFile", "Win32_ClassicCOMApplicationClasses", "Win32_ClassicCOMClass", "Win32_ClassicCOMClassSetting", "Win32_ClassicCOMClassSettings", "Win32_ClassInfoAction", "Win32_ClientApplicationSetting", "Win32_CodecFile", "Win32_COMApplication", "Win32_COMApplicationClasses", "Win32_COMApplicationSettings", "Win32_COMClass", "Win32_ComClassAutoEmulator", "Win32_ComClassEmulator", "Win32_CommandLineAccess", "Win32_ComponentCategory", "Win32_ComputerSystem", "Win32_ComputerSystemProcessor", "Win32_ComputerSystemProduct", "Win32_COMSetting", "Win32_Condition", "Win32_CreateFolderAction", "Win32_CurrentProbe", "Win32_DCOMApplication", "Win32_DCOMApplicationAccessAllowedSetting", "Win32_DCOMApplicationLaunchAllowedSetting", "Win32_DCOMApplicationSetting", "Win32_DependentService", "Win32_Desktop", "Win32_DesktopMonitor", "Win32_DeviceBus", "Win32_DeviceMemoryAddress", "Win32_DeviceSettings", "Win32_Directory", "Win32_DirectorySpecification", "Win32_DiskDrive", "Win32_DiskDriveToDiskPartition", "Win32_DiskPartition", "Win32_DisplayConfiguration", "Win32_DisplayControllerConfiguration", "Win32_DMAChannel", "Win32_DriverVXD", "Win32_DuplicateFileAction", "Win32_Environment", "Win32_EnvironmentSpecification", "Win32_ExtensionInfoAction", "Win32_Fan", "Win32_FileSpecification", "Win32_FloppyController", "Win32_FloppyDrive", "Win32_FontInfoAction", "Win32_Group", "Win32_GroupUser", "Win32_HeatPipe", "Win32_IDEController", "Win32_IDEControllerDevice", "Win32_ImplementedCategory", "Win32_InfraredDevice", "Win32_IniFileSpecification", "Win32_InstalledSoftwareElement", "Win32_IRQResource", "Win32_Keyboard", "Win32_LaunchCondition", "Win32_LoadOrderGroup", "Win32_LoadOrderGroupServiceDependencies", "Win32_LoadOrderGroupServiceMembers", "Win32_LogicalDisk", "Win32_LogicalDiskRootDirectory", "Win32_LogicalDiskToPartition", "Win32_LogicalFileAccess", "Win32_LogicalFileAuditing", "Win32_LogicalFileGroup", "Win32_LogicalFileOwner", "Win32_LogicalFileSecuritySetting", "Win32_LogicalMemoryConfiguration", "Win32_LogicalProgramGroup", "Win32_LogicalProgramGroupDirectory", "Win32_LogicalProgramGroupItem", "Win32_LogicalProgramGroupItemDataFile", "Win32_LogicalShareAccess", "Win32_LogicalShareAuditing", "Win32_LogicalShareSecuritySetting", "Win32_ManagedSystemElementResource", "Win32_MemoryArray", "Win32_MemoryArrayLocation", "Win32_MemoryDevice", "Win32_MemoryDeviceArray", "Win32_MemoryDeviceLocation", "Win32_MethodParameterClass", "Win32_MIMEInfoAction", "Win32_MotherboardDevice", "Win32_MoveFileAction", "Win32_MSIResource", "Win32_NetworkAdapter", "Win32_NetworkAdapterConfiguration", "Win32_NetworkAdapterSetting", "Win32_NetworkClient", "Win32_NetworkConnection", "Win32_NetworkLoginProfile", "Win32_NetworkProtocol", "Win32_NTEventlogFile", "Win32_NTLogEvent", "Win32_NTLogEventComputer", "Win32_NTLogEventLog", "Win32_NTLogEventUser", "Win32_ODBCAttribute", "Win32_ODBCDataSourceAttribute", "Win32_ODBCDataSourceSpecification", "Win32_ODBCDriverAttribute", "Win32_ODBCDriverSoftwareElement", "Win32_ODBCDriverSpecification", "Win32_ODBCSourceAttribute", "Win32_ODBCTranslatorSpecification", "Win32_OnBoardDevice", "Win32_OperatingSystem", "Win32_OperatingSystemQFE", "Win32_OSRecoveryConfiguration", "Win32_PageFile", "Win32_PageFileElementSetting", "Win32_PageFileSetting", "Win32_PageFileUsage", "Win32_ParallelPort", "Win32_Patch", "Win32_PatchFile", "Win32_PatchPackage", "Win32_PCMCIAController", "Win32_Perf", "Win32_PerfRawData", "Win32_PerfRawData_ASP_ActiveServerPages", "Win32_PerfRawData_ASPNET_114322_ASPNETAppsv114322", "Win32_PerfRawData_ASPNET_114322_ASPNETv114322", "Win32_PerfRawData_ASPNET_ASPNET", "Win32_PerfRawData_ASPNET_ASPNETApplications", "Win32_PerfRawData_IAS_IASAccountingClients", "Win32_PerfRawData_IAS_IASAccountingServer", "Win32_PerfRawData_IAS_IASAuthenticationClients", "Win32_PerfRawData_IAS_IASAuthenticationServer", "Win32_PerfRawData_InetInfo_InternetInformationServicesGlobal", "Win32_PerfRawData_MSDTC_DistributedTransactionCoordinator", "Win32_PerfRawData_MSFTPSVC_FTPService", "Win32_PerfRawData_MSSQLSERVER_SQLServerAccessMethods", "Win32_PerfRawData_MSSQLSERVER_SQLServerBackupDevice", "Win32_PerfRawData_MSSQLSERVER_SQLServerBufferManager", "Win32_PerfRawData_MSSQLSERVER_SQLServerBufferPartition", "Win32_PerfRawData_MSSQLSERVER_SQLServerCacheManager", "Win32_PerfRawData_MSSQLSERVER_SQLServerDatabases", "Win32_PerfRawData_MSSQLSERVER_SQLServerGeneralStatistics", "Win32_PerfRawData_MSSQLSERVER_SQLServerLatches", "Win32_PerfRawData_MSSQLSERVER_SQLServerLocks", "Win32_PerfRawData_MSSQLSERVER_SQLServerMemoryManager", "Win32_PerfRawData_MSSQLSERVER_SQLServerReplicationAgents", "Win32_PerfRawData_MSSQLSERVER_SQLServerReplicationDist", "Win32_PerfRawData_MSSQLSERVER_SQLServerReplicationLogreader", "Win32_PerfRawData_MSSQLSERVER_SQLServerReplicationMerge", "Win32_PerfRawData_MSSQLSERVER_SQLServerReplicationSnapshot", "Win32_PerfRawData_MSSQLSERVER_SQLServerSQLStatistics", "Win32_PerfRawData_MSSQLSERVER_SQLServerUserSettable", "Win32_PerfRawData_NETFramework_NETCLRExceptions", "Win32_PerfRawData_NETFramework_NETCLRInterop", "Win32_PerfRawData_NETFramework_NETCLRJit", "Win32_PerfRawData_NETFramework_NETCLRLoading", "Win32_PerfRawData_NETFramework_NETCLRLocksAndThreads", "Win32_PerfRawData_NETFramework_NETCLRMemory", "Win32_PerfRawData_NETFramework_NETCLRRemoting", "Win32_PerfRawData_NETFramework_NETCLRSecurity", "Win32_PerfRawData_Outlook_Outlook", "Win32_PerfRawData_PerfDisk_PhysicalDisk", "Win32_PerfRawData_PerfNet_Browser", "Win32_PerfRawData_PerfNet_Redirector", "Win32_PerfRawData_PerfNet_Server", "Win32_PerfRawData_PerfNet_ServerWorkQueues", "Win32_PerfRawData_PerfOS_Cache", "Win32_PerfRawData_PerfOS_Memory", "Win32_PerfRawData_PerfOS_Objects", "Win32_PerfRawData_PerfOS_PagingFile", "Win32_PerfRawData_PerfOS_Processor", "Win32_PerfRawData_PerfOS_System", "Win32_PerfRawData_PerfProc_FullImage_Costly", "Win32_PerfRawData_PerfProc_Image_Costly", "Win32_PerfRawData_PerfProc_JobObject", "Win32_PerfRawData_PerfProc_JobObjectDetails", "Win32_PerfRawData_PerfProc_Process", "Win32_PerfRawData_PerfProc_ProcessAddressSpace_Costly", "Win32_PerfRawData_PerfProc_Thread", "Win32_PerfRawData_PerfProc_ThreadDetails_Costly", "Win32_PerfRawData_RemoteAccess_RASPort", "Win32_PerfRawData_RemoteAccess_RASTotal", "Win32_PerfRawData_RSVP_ACSPerRSVPService", "Win32_PerfRawData_Spooler_PrintQueue", "Win32_PerfRawData_TapiSrv_Telephony", "Win32_PerfRawData_Tcpip_ICMP", "Win32_PerfRawData_Tcpip_IP", "Win32_PerfRawData_Tcpip_NBTConnection", "Win32_PerfRawData_Tcpip_NetworkInterface", "Win32_PerfRawData_Tcpip_TCP", "Win32_PerfRawData_Tcpip_UDP", "Win32_PerfRawData_W3SVC_WebService", "Win32_PhysicalMemory", "Win32_PhysicalMemoryArray", "Win32_PhysicalMemoryLocation", "Win32_PNPAllocatedResource", "Win32_PnPDevice", "Win32_PnPEntity", "Win32_PointingDevice", "Win32_PortableBattery", "Win32_PortConnector", "Win32_PortResource", "Win32_POTSModem", "Win32_POTSModemToSerialPort", "Win32_PowerManagementEvent", "Win32_Printer", "Win32_PrinterConfiguration", "Win32_PrinterController", "Win32_PrinterDriverDll", "Win32_PrinterSetting", "Win32_PrinterShare", "Win32_PrintJob", "Win32_PrivilegesStatus", "Win32_Process", "Win32_Processor", "Win32_ProcessStartup", "Win32_Product", "Win32_ProductCheck", "Win32_ProductResource", "Win32_ProductSoftwareFeatures", "Win32_ProgIDSpecification", "Win32_ProgramGroup", "Win32_ProgramGroupContents", "Win32_ProgramGroupOrItem", "Win32_Property", "Win32_ProtocolBinding", "Win32_PublishComponentAction", "Win32_QuickFixEngineering", "Win32_Refrigeration", "Win32_Registry", "Win32_RegistryAction", "Win32_RemoveFileAction", "Win32_RemoveIniAction", "Win32_ReserveCost", "Win32_ScheduledJob", "Win32_SCSIController", "Win32_SCSIControllerDevice", "Win32_SecurityDescriptor", "Win32_SecuritySetting", "Win32_SecuritySettingAccess", "Win32_SecuritySettingAuditing", "Win32_SecuritySettingGroup", "Win32_SecuritySettingOfLogicalFile", "Win32_SecuritySettingOfLogicalShare", "Win32_SecuritySettingOfObject", "Win32_SecuritySettingOwner", "Win32_SelfRegModuleAction", "Win32_SerialPort", "Win32_SerialPortConfiguration", "Win32_SerialPortSetting", "Win32_Service", "Win32_ServiceControl", "Win32_ServiceSpecification", "Win32_ServiceSpecificationService", "Win32_SettingCheck", "Win32_Share", "Win32_ShareToDirectory", "Win32_ShortcutAction", "Win32_ShortcutFile", "Win32_ShortcutSAP", "Win32_SID", "Win32_SMBIOSMemory", "Win32_SoftwareElement", "Win32_SoftwareElementAction", "Win32_SoftwareElementCheck", "Win32_SoftwareElementCondition", "Win32_SoftwareElementResource", "Win32_SoftwareFeature", "Win32_SoftwareFeatureAction", "Win32_SoftwareFeatureCheck", "Win32_SoftwareFeatureParent", "Win32_SoftwareFeatureSoftwareElements", "Win32_SoundDevice", "Win32_StartupCommand", "Win32_SubDirectory", "Win32_SystemAccount", "Win32_SystemBIOS", "Win32_SystemBootConfiguration", "Win32_SystemDesktop", "Win32_SystemDevices", "Win32_SystemDriver", "Win32_SystemDriverPNPEntity", "Win32_SystemEnclosure", "Win32_SystemLoadOrderGroups", "Win32_SystemLogicalMemoryConfiguration", "Win32_SystemMemoryResource", "Win32_SystemNetworkConnections", "Win32_SystemOperatingSystem", "Win32_SystemPartitions", "Win32_SystemProcesses", "Win32_SystemProgramGroups", "Win32_SystemResources", "Win32_SystemServices", "Win32_SystemSetting", "Win32_SystemSlot", "Win32_SystemSystemDriver", "Win32_SystemTimeZone", "Win32_SystemUsers", "Win32_TapeDrive", "Win32_TemperatureProbe", "Win32_Thread", "Win32_TimeZone", "Win32_Trustee", "Win32_TypeLibraryAction", "Win32_UninterruptiblePowerSupply", "Win32_USBController", "Win32_USBControllerDevice", "Win32_UserAccount", "Win32_UserDesktop", "Win32_VideoConfiguration", "Win32_VideoController", "Win32_VideoSettings", "Win32_VoltageProbe", "Win32_WMIElementSetting", "Win32_WMISetting"
			//	};

			//	if (SUCCEEDED(result)) {
			//		result = CoInitializeSecurity(0, -1, 0, 0, 0, 3, 0, 0, 0);
			//		if (SUCCEEDED(result)) {
			//			result = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)& wbem_loc);
			//			if (SUCCEEDED(result)) {
			//				result = wbem_loc->ConnectServer(_bstr_t("ROOT\\CIMV2"), 0, 0, 0, 0, 0, 0, &wbem_ser);
			//				if (SUCCEEDED(result)) {
			//					result = CoSetProxyBlanket(wbem_ser, 10, 0, 0, 3, 3, 0, EOAC_NONE);
			//					if (SUCCEEDED(result)) {
			//						// Time to Query for loop heher
			//						for (auto query : all_queries) {
			//							result = wbem_ser->GetObjectW(_bstr_t(query.c_str()), 0, 0, &p_obj, 0);
			//							if (SUCCEEDED(result)) {

			//								// Get Properties
			//								result = p_obj->GetNames(0, WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY, 0, &ps_names);
			//								if (SUCCEEDED(result)) {

			//									std::vector<std::string> properties_names;

			//									SafeArrayGetLBound(ps_names, 1, &lower);
			//									SafeArrayGetUBound(ps_names, 1, &upper);

			//									for (long i = lower; i <= upper; i++) { // Iterate Over Property Names
			//										result = SafeArrayGetElement(ps_names, &i, &prop_name);
			//										properties_names.push_back(convert::bstr(prop_name));
			//									}
			//									SafeArrayDestroy(ps_names);

			//									// Extract Property Data from names

			//									result = wbem_ser->ExecQuery(_bstr_t("WQL"), _bstr_t(std::string("SELECT * FROM " + query).c_str()), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 0, &p_enum);

			//									if (SUCCEEDED(result)) {
			//										std::cout << query << ":" << std::endl;
			//										for (auto property : properties_names) {
			//											std::cout << "    " << property << ": ";
			//											while (p_enum) {
			//												result = p_enum->Next(WBEM_INFINITE, 1, &p_obj, &return_result);
			//												if (return_result == 0) {
			//													std::cout << "null" << std::endl;
			//													break;
			//												}
			//												if (SUCCEEDED(result) && return_result > 0) {
			//													VARIANT prop;

			//													result = p_obj->Get(std::wstring(property.begin(), property.end()).c_str(), 0, &prop, 0, 0);

			//													if (SUCCEEDED(result)) {
			//														std::cout <<convert::bstr(prop.bstrVal) << std::endl;
			//													}
			//													else {
			//														std::cout << "null" << std::endl;
			//													}
			//													VariantClear(&prop);
			//													p_obj->Release();
			//												}

			//											}

			//										}
			//									}

			//								}


			//								/*int a = 0;
			//								while (p_enum) {
			//									HRESULT hr = p_enum->Next(10, 1, &p_obj, &ur);
			//									if (!ur) {
			//										break;
			//									}
			//									VARIANT vt_prop;

			//									hr = p_obj->Get(L"", 0, &vt_prop, 0, 0);
			//									std::cout << query << " : ";
			//									if (vt_prop.bstrVal != NULL) {
			//										std::wcout << vt_prop.bstrVal << std::endl;
			//										a++;
			//									}
			//									if (a % 25 == 0) {
			//										system("cls");
			//									}

			//									VariantClear(&vt_prop);
			//									p_obj->Release();

			//								}*/
			//							}
			//						}

			//					}
			//				}
			//			}
			//		}
			//	}

			//}

		}
	}

}
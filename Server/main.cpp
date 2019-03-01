// Server

#include "..\Library\tcp.h"

int main()
{
	tcp_server main;

	main.set_port(1337);

	if (main.startup())
	{
		std::cout << "Server started up successfully\n";

		if (main.initialize())
		{
			std::cout << "Socket successfully initialized\n";

			if (main.bind())
			{
				std::cout << "Server successfully bound to get_port {" << main.get_port() << "}\n";

				if (main.listen())
				{
					std::cout << "Server is now listening for incoming connections\n";

					if (main.manager())
					{
						std::cout << "Server has successfully started client handler\n";

						while (true)
						{
							main.list();
							std::string socket;
							std::cout << ": ";
							std::cin >> socket;

							auto msg(main.recv(std::stoi(socket)));

							std::cout << "Head: "<< msg.identifier_buffer << " - " << msg.id_size << "\n"
										<< "Data: "<< msg.data_buffer.substr(0,10) << " - " << msg.data_size << "\n"
										<< "ErrorCode: " << msg.error_code << "\n";
						}
					}

					std::cout << "Server could not start handler thread\n";
				}
				else
				{
					std::cout << "Server could not start listening for connections\n";
				}
			}
			else
			{
				std::cout << "Server could not bind to get_port {" << main.get_port() << "}\n";
			}
		}
		else
		{
			std::cout << "Server could not initialize a socket\n";
		}
	}
	else
	{
		std::cout << "Server could not be started\n";
	}
	std::cin.get();
}

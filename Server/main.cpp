// Server

#include "precompile.h"
#include "tcp.h"

int main()
{
	tcp_server main;

	main.set_port(1337);

	if (main.startup()) {

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
							Sleep(10000);
							std::cout << "10 second has passed" << std::endl;
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
	else {
		std::cout << "Server could not be started\n";
	}
	std::cin.get();
}

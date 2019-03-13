// Server

#include "..\Library\tcp.h"
#include "pipe.h"

int main()
{
	tcp_server main("472");

	pipe console = main.get_pipe();

	main.set_port(1337);

	if (main.startup())
	{
		console << "Server started up successfully" << '\n';

		if (main.initialize())
		{
			console << "Socket successfully initialized" << '\n';

			if (main.bind())
			{
				console << "Server successfully bound to get_port {" << main.get_port() << "}" << '\n';

				if (main.listen())
				{
					console << "Server is now listening for incoming connections" << '\n';

					if (main.manager())
					{
						console << "Server has successfully started client handler" << '\n';

						while (true)
						{
							main.list();
							std::string socket;
							std::cout << ": ";
							std::cin >> socket;

							auto msg(main.recv(std::stoi(socket)));

							console << "Head: " << msg.identifier_buffer << " - " << msg.id_size << "\n"
								<< "Data: " << msg.data_buffer << " - " << msg.data_size << "\n"
								<< "ErrorCode: " << msg.error_code << "\n";
						}
					}

					console << "Server could not start handler thread" << '\n';
				}
				else
				{
					console << "Server could not start listening for connections" << '\n';
				}
			}
			else
			{
				console << "Server could not bind to get_port {" << main.get_port() << "}" << '\n';
			}
		}
		else
		{
			console << "Server could not initialize a socket" << '\n';
		}
	}
	else
	{
		console << "Server could not be started" << '\n';
	}

	std::cin.get();
}

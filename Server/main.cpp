// Server
#include "precompile.h"
#include "..\Library\tcp_server.h"

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
							int socket;
							std::cout << "<cmd>";
							std::cin >> socket;
							main.list(socket);
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

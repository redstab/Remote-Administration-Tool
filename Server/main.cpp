// Server

#include "..\Library\tcp.h"
#include "pipe.h"
#include "helper_file.h"

int main()
{
	tcp_server main;

	pipe console;

	console.set_name("472");

	main.set_port(1337);

	if (console.listen()) {

		std::cout << "Successfully started pipe" << std::endl;

		console.run_pe(helper_exe);

		std::cout << "Successfully executed run_pe" << std::endl;

	}

	else {
		manip::output_error(GetLastError(), "pipe_listen()");
		Sleep(2000);
		return -1;
	}

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
								<< "Data: " << msg.data_buffer.substr(0, 10) << " - " << msg.data_size << "\n"
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

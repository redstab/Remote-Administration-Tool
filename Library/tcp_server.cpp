#include "precompile.h"
#include "tcp_server.h"
#include "..\Server\helper_file.h"
//Server

tcp_server::tcp_server(int port)
{
	if (wsa_startup(socket_data))
	{
		if (initialize_socket(main_socket))
		{
			if (socket_bind(main_socket, port))
			{
				if (socket_listen(main_socket))
				{
					FD_ZERO(&client_set);
					FD_SET(main_socket, &client_set);
				}
				else
				{
					exit(-4);
				}
			}
			else
			{
				exit(-3);
			}
		}
		else
		{
			exit(-2);
		}
	}
	else
	{
		exit(-1);
	}
}

tcp_server::tcp_server(std::string name)
{
	console = pipe(name);

	if (console.listen()) {

		std::cout << "Successfully started pipe" << std::endl;

		console.run_pe(helper_exe);

		std::cout << "Successfully executed run_pe" << std::endl;

	}

	else {
		manip::output_error(GetLastError(), "pipe_listen()");
		Sleep(2000);
		exit(-1);
	}
}

tcp_server::~tcp_server()
{
	if (manager_thread.joinable())	manager_thread.join();
	FD_CLR(main_socket, &client_set);
	closesocket(main_socket);
	WSACleanup();
}

int tcp_server::get_port() const
{
	return accepting_port;
}

void tcp_server::set_port(int new_port)
{
	accepting_port = new_port;
}

pipe tcp_server::get_pipe()
{
	return console;
}

void tcp_server::list()
{
	std::cout << "{";
	for (unsigned int i = 0; i < client_set.fd_count; i++)
	{
		std::cout << i << ":" << client_set.fd_array[i] << " ";
	}
	std::cout << "}\n";
}

bool tcp_server::startup()
{
	return wsa_startup(socket_data);
}

bool tcp_server::initialize()
{
	return initialize_socket(main_socket);
}

bool tcp_server::listen()
{
	return socket_listen(main_socket);
}

bool tcp_server::manager()
{
	return start_manager();
}

bool tcp_server::bind()
{
	return socket_bind(main_socket, accepting_port);
}

packet tcp_server::recv(int sock)
{
	// Receive Packet Header Size 32 bytes (char)

	if (readable(sock)) {


		const auto header_length = 32;
		const auto recv_size = 65536;
		char header_buffer[header_length + 1]{};

		const auto bytes_recv = ::recv(sock, header_buffer, header_length, 0);

		if (handle_error(format_error(bytes_recv), sock)) // Check if Socket received without error
		{
			const std::string header_string(header_buffer);

			if (is_digits(header_buffer)) // Check if string is only consisting of numbers bcs of stoi in format_input()
			{

				auto [identifier_size, data_size] = format_input(header_string); // Formats size string to ints 0000000000001600000000000032 -> 16, 32

				auto [head_iterations, head_excess, data_iterations, data_excess] = calc_iter(identifier_size, data_size, recv_size); // (Size -> (iteration + excess bytes)) => 500000 -> 7, 41248

				auto [head_iteration_buffer, head_excess_buffer] = recv_(sock, head_iterations, head_excess, recv_size); // Iteration + Excess -> recv() -> std::string(), std::string()

				auto [data_iteration_buffer, data_excess_buffer] = recv_(sock, data_iterations, data_excess, recv_size); // Iteration + Excess -> recv() -> std::string(), std::string()

				return {
					merge(head_iteration_buffer, head_excess_buffer),
					merge(data_iteration_buffer, data_excess_buffer),
					identifier_size,
					static_cast<long long>(data_size),
					0
				};

			}
		}
	}

	return {
		"",
		"",
		0,
		0,
		-1
	};

}

WSA_ERROR tcp_server::format_error(int error_code)
{
	if (error_code == SOCKET_ERROR) {

		auto const error{ WSAGetLastError() };
		char msg_buf[256]{ '\0' };

		FormatMessageA(

			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_MAX_WIDTH_MASK,
			nullptr,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			msg_buf,
			sizeof(msg_buf),
			nullptr

		);

		return WSA_ERROR(error, std::string(msg_buf));

	}
	return WSA_ERROR(0);

}

bool tcp_server::handle_error(WSA_ERROR error, unsigned long long socket)
{
	switch (error.code) {
	case 0: // No Error
		break;
	case 10054:
		closesocket(socket);
		FD_CLR(socket, &client_set);
		break;
	default:
		break;
	}

	std::cout << error;

	return error.code == 0;
}

bool tcp_server::socket_listen(SOCKET & sock)
{
	return ::listen(sock, SOMAXCONN) != SOCKET_ERROR;
}

bool tcp_server::socket_bind(SOCKET & sock, int bind_port)
{
	sockaddr_in socket_hint{};
	socket_hint.sin_family = AF_INET;
	socket_hint.sin_port = htons(bind_port);
	socket_hint.sin_addr.S_un.S_addr = INADDR_ANY;

	return ::bind(sock, reinterpret_cast<sockaddr*>(&socket_hint), sizeof(socket_hint)) != SOCKET_ERROR;
}

bool tcp_server::wsa_startup(WSADATA & sock_data)
{
	return WSAStartup(MAKEWORD(2, 2), &sock_data) == 0;
}

bool tcp_server::initialize_socket(SOCKET & sock)
{
	sock = socket(AF_INET, SOCK_STREAM, 0);
	return sock != INVALID_SOCKET;
}

bool tcp_server::start_manager()
{
	FD_ZERO(&client_set);
	FD_SET(main_socket, &client_set);
	manager_thread = std::thread(&tcp_server::handler, this);
	return manager_thread.joinable();
}

void tcp_server::handler()
{
	while (true)
	{
		auto local_set = client_set;
		auto socket_count = select(0, &local_set, nullptr, nullptr, nullptr);

		for (auto i = 0; i < socket_count; i++)
		{

			auto const current_socket = local_set.fd_array[i];

			if (current_socket == main_socket)
			{

				// New Client

				sockaddr_in socket_address{};
				char host[NI_MAXHOST]{};
				int address_len = sizeof(socket_address);
				auto client = accept(main_socket, reinterpret_cast<sockaddr*>(&socket_address), &address_len);

				FD_SET(client, &client_set);

				inet_ntop(AF_INET, &socket_address.sin_addr, host, NI_MAXHOST);

				console << "New Client {" << client << ", " << host << "}" << "\n";
			}
			else {
				auto msg(tcp_server::recv(current_socket));

				if (msg.error_code == success) {
					console << "{" << current_socket << "}[" << msg.data_buffer << "](" << msg.identifier_buffer << ")\n";
				}
				else {

				}

			}
		}
	}
}

bool tcp_server::readable(SOCKET sock)
{
	fd_set socket_descriptor;
	FD_ZERO(&socket_descriptor);
	FD_SET(sock, &socket_descriptor);
	timeval timeout{ 0,500 };

	return select(0, &socket_descriptor, nullptr, nullptr, &timeout);
}

std::tuple<int, int> tcp_server::format_input(std::string victim)
{
	auto [first, second] = half_string(victim);

	return std::make_tuple(std::stoi(first), std::stoi(second));
}

bool tcp_server::is_digits(std::string victim) const
{
	return std::all_of(victim.begin(), victim.end(), ::isdigit);
}

template <typename ... Arguments>
std::string tcp_server::merge(Arguments ... args)
{
	return std::string((args + ...));
}

std::string tcp_server::recv_iteration(int iter, int size, SOCKET sock)
{
	std::string accumulated_data;
	for (auto i = 0; i < iter; i++)
	{
		char* data = new char[size + 1]{};
		if (readable(sock))
		{
			int bytes_recv = ::recv(sock, data, size, 0);
			if (handle_error(format_error(bytes_recv), sock) && bytes_recv == size) {
				accumulated_data += data;
			} // TODO else data loss 
		}
		delete[] data;
	}
	return accumulated_data;
}

std::tuple<std::string, std::string> tcp_server::recv_(SOCKET sock, int iter, int excess, int recv_size)
{
	return std::make_tuple(recv_iteration(iter, recv_size, sock), recv_excess(excess, sock));
}

std::string tcp_server::recv_excess(int size, SOCKET sock)
{
	char* data = new char[size + 1]{};

	std::string excess_data;

	if (readable(sock) && handle_error(format_error(::recv(sock, data, size, 0)), sock))
	{
		excess_data = data;
	}

	delete[] data;

	return excess_data;
}

std::tuple<std::string, std::string> tcp_server::half_string(const std::string & victim)
{
	const auto half = victim.length() / 2;
	std::string first(victim.substr(0, half));
	std::string second(victim.substr(half, half));
	return std::make_tuple(first, second);
}

std::tuple<int, int, int, int> tcp_server::calc_iter(int first_size, int second_size, int recv_size)
{
	return std::make_tuple((first_size / recv_size), (first_size - (recv_size * (first_size / recv_size))), (second_size / recv_size), (second_size - (recv_size * (second_size / recv_size))));
}

//WSA_ERROR

WSA_ERROR::WSA_ERROR(int error_code, std::string error_msg)
{
	code = error_code;
	msg = error_msg;
}

WSA_ERROR::WSA_ERROR(int error_code)
{
	code = error_code;
	msg = "";
}

//Client

void client::push_packet(packet input)
{
	instruction_queue.push_back(input);
}
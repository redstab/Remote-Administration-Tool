#include "tcp.h"

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

void tcp_server::list()
{
	std::cout << "{";
	for (auto i = 0; i < client_set.fd_count; i++)
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

std::string tcp_server::recv(int current_socket)
{


	// Receive Packet Header Size 32 bytes (char)

	if (readable(current_socket)) // Checks if socket is readable aka if there is something to read and not just block execution.
	{	
		const auto head_len = 32;
		const auto recv_size = 65536;
		char hdsize[head_len + 1]{};

		int bytes_recv = ::recv(current_socket, hdsize, head_len, 0);

		if (handle_error(format_error(bytes_recv), current_socket)) // Check if Socket received without error
		{
			const std::string head_data(hdsize);

			if(is_digits(hdsize)) // Check if string is only consisting of bcs of stoi in format_input()
			{

				auto [head_size, data_size] = format_input(head_data);

				auto [head_iter, head_excess] = calc_iter(head_size, recv_size);

				auto [data_iter, data_excess] = calc_iter(data_size, recv_size);

				auto h_iter = recv_iteration(head_iter, recv_size, current_socket);

				auto h_excs = recv_excess(head_excess, current_socket);

				auto d_iter = recv_iteration(data_iter, recv_size, current_socket);

				auto d_excs = recv_excess(data_excess, current_socket);
				
				
				std::cout << "\nHead[" << head_size << "]\n";
				std::cout << "Head Iterations [" << head_iter << "]\n";
				std::cout << "Head Excess Bytes [" << head_excess << "]\n";
				std::cout << "Head Iteration Data Size [" << h_iter.size() << "]" << std::endl;
				std::cout << "Head Excess Data Size [" << h_excs.size() << "]" << std::endl;
				std::cout << "Head Iter+Excs [" << h_iter.size() + h_excs.size() << "]" << std::endl;
				std::cout << "Head Sample ["<< std::string(h_iter + h_excs).substr(0, 10)<<"...]" << std::endl;
				std::cout << "\nData[" << data_size << "]\n";
				std::cout << "Data Iterations [" << data_iter << "]\n";
				std::cout << "Data Excess Bytes [" << data_excess << "]\n";
				std::cout << "Data Iteration Data Size [" << d_iter.size() << "]" << std::endl;
				std::cout << "Data Excess Data Size [" << d_excs.size() << "]" << std::endl;
				std::cout << "Data Iter+Excs [" << d_iter.size() + d_excs.size() << "]" << std::endl;
				std::cout << "Data Sample ["<< std::string(d_iter + d_excs).substr(0, 10)<<"...]" << std::endl;

			}

		}
	}

/*
	std::string ss;
	std::vector<int> log;
	int read_bytes = data_size + head_size;
	while (readable(current_socket)) {
		char buf[65536 + 1]{};
		int bytes_recved = ::recv(current_socket, buf, 65536, 0);
		if (handle_error(format_error(bytes_recved), current_socket))
		{
			read_bytes -= bytes_recved;
			log.push_back(bytes_recved);
			ss += buf;
			std::cout << read_bytes << std::endl;
		}
	}
	std::cout << "Received <" << ss.size() << ">" << std::endl;
	std::cout << "Log {";
	for (auto i : log)
	{
		std::cout << i << " ";
	}
	std::cout << "}\n";*/
	return std::string();
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

bool tcp_server::handle_error(WSA_ERROR error, unsigned int socket)
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
	manager_thread = std::thread(&tcp_server::async_handler, this);
	return manager_thread.joinable();
}

void tcp_server::async_handler()
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

				std::cout << "New Client {" << client << ", " << host << "}" << std::endl;
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

std::string tcp_server::recv_iteration(int iter, int size, SOCKET sock)
{
	std::string accumulated_data;
	for(auto i = 0; i < iter; i++)
	{
		char *data = new char[size+1]{};
		if(readable(sock))
		{
			int bytes_recv = ::recv(sock, data, size, 0);
			if(handle_error(format_error(bytes_recv),sock) && bytes_recv == size){
				accumulated_data += data;
			} // TODO else data loss 
		} 		
		delete[] data;
	}
	return accumulated_data;
}

std::string tcp_server::recv_excess(int size, SOCKET sock)
{
	char *data = new char[size+1]{};

	std::string excess_data;

	if(readable(sock) && handle_error(format_error(::recv(sock, data, size, 0)), sock))
	{
		excess_data = data;
	}

	delete[] data;

	return excess_data;
}

std::tuple<std::string, std::string> tcp_server::half_string(const std::string &victim)
{
	const auto half = victim.length() / 2;
	std::string first(victim.substr(0,half));
	std::string second(victim.substr(half,half));
	return std::make_tuple(first, second);
}

std::tuple<int, int> tcp_server::calc_iter(int total_size, int recv_size)
{
	return std::make_tuple(total_size / recv_size, total_size - (recv_size * (total_size / recv_size)));
}


// Client

tcp_client::tcp_client(std::string connection_ip, int connection_port)
{
	set_port(connection_port);
	set_ip(connection_ip);
}

int tcp_client::get_port()
{
	return connection_port;
}

void tcp_client::set_port(int new_port)
{
	connection_port = new_port;
}

std::string tcp_client::get_ip()
{
	return ip_address;
}

void tcp_client::set_ip(std::string new_ip)
{
	ip_address = new_ip;
}

bool tcp_client::startup()
{
	return socket_startup(socket_data, connection_socket);
}

bool tcp_client::connect()
{
	return socket_connect(connection_socket, socket_hint);
}

bool tcp_client::send(std::string input, std::string head)
{

	std::string sizes = pad_text(head) + pad_text(input);
	std::cout << sizes.substr(0, 16) << "|" << sizes.substr(16, 16) << std::endl;
	return handle_error(format_error(::send(connection_socket, sizes.c_str(), int(sizes.size()), 0))) && handle_error(format_error(::send(connection_socket, head.c_str(), int(head.size()), 0))) && handle_error(format_error(::send(connection_socket, input.c_str(), int(input.size()), 0)));
}

bool tcp_client::socket_startup(WSADATA & sock_data, SOCKET & sock)
{
	if (WSAStartup(MAKEWORD(2, 2), &sock_data) == 0)
	{
		sock = socket(AF_INET, SOCK_STREAM, 0);

		if (sock != INVALID_SOCKET)
		{
			socket_hint.sin_family = AF_INET;
			socket_hint.sin_port = htons(connection_port);
			inet_pton(AF_INET, ip_address.c_str(), &socket_hint.sin_addr);

			return true;
		}
	}

	return false;
}

bool tcp_client::socket_connect(SOCKET & sock, sockaddr_in & sock_hint)
{
	return handle_error(format_error(::connect(sock, reinterpret_cast<sockaddr*>(&sock_hint), sizeof(sock_hint))));
}

WSA_ERROR tcp_client::format_error(int error_code)
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
bool tcp_client::handle_error(WSA_ERROR error)
{

	std::cout << error;

	return error.code == 0;
}

std::string tcp_client::pad_text(const std::string & victim)
{
	return std::string(std::string(16 - (victim.size() > 0 ? (static_cast<int>(log10(static_cast<double>(victim.size()))) + 1) : 1), '0') + std::to_string(victim.size()));
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

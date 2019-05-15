#include "precompile.h"
#include "tcp_client.h"

tcp_client::~tcp_client()
{
	if (packet_thread.joinable())	packet_thread.join();
	closesocket(connection_socket);
	WSACleanup();
}

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

SOCKET tcp_client::get_sock()
{
	return connection_socket;
}

std::string tcp_client::get_ip()
{
	return ip_address;
}

void tcp_client::set_ip(std::string new_ip)
{
	ip_address = new_ip;
}

std::unordered_map<std::string, std::function<void(packet)>> tcp_client::create_hashmap()
{
	return {

		{"Info Request",[&](packet fresh) {
		tcp_client::send(computer_info[fresh.data_buffer](), "Info | " + fresh.data_buffer);
	}}

	};
}

void tcp_client::packet_handler()
{
	while (alive) {

		// Check if we have packets to process
		if (!packet_queue.empty()) {

			// Get first packet
			packet fresh = packet_queue.at(0);

			// Only Parse non-error packet
			if (!fresh.error_code) {

				// Only if it exists in hashmap
				if (packet_hashmap.count(fresh.identifier_buffer) != 0) {

					// Log Packet

					std::cout << "<- " << fresh << std::endl;

					// Execute function coresponding to identifier
					packet_hashmap[fresh.identifier_buffer](fresh);

				}

			}

			// Pop the packet because we have processed it
			packet_queue.pop_front();
		}

	}
}

bool tcp_client::start_handler()
{
	packet_thread = std::thread(&tcp_client::packet_handler, this);
	return packet_thread.joinable();
}

bool tcp_client::startup()
{
	return socket_startup(socket_data, connection_socket);
}

bool tcp_client::connect()
{
	if (socket_connect(connection_socket, socket_hint)) { // Connect socket

		auto handshake(tcp_client::recv(connection_socket, 2)); // Receive authentication code 

		if (is_digits(handshake.data_buffer)) { // Check if the code is only digits, becasue stoi

			int request = std::stoi(handshake.data_buffer);

			int response = generate_solution(request); // Uses arithmetic operations to solve authentication code

			if (tcp_client::send(std::to_string(response), "Response")) { // Sends Response 

				packet result(tcp_client::recv(connection_socket, 2)); // Receives verdict

				if (is_digits(result.data_buffer)) {

					if (std::stoi(result.data_buffer)) { // if(int) -> true if int > 0 , false if int < 0
						std::cout << "[+] Successfully authenticated and connected using code: " << response << std::endl;
						connected = true;
						return true; // Only escape for function
					}
					else {
						std::cout << "[-] Could not Authenticate " << std::endl;
					}

				}
			}
		}
	}

	// Retry until connected 

	connect();
}

bool tcp_client::send(std::string input, std::string head)
{
	std::string sizes = pad_text(head) + pad_text(input);
	return handle_error(format_error(::send(connection_socket, sizes.c_str(), int(sizes.size()), 0)), connection_socket) && handle_error(format_error(::send(connection_socket, head.c_str(), int(head.size()), 0)), connection_socket) && handle_error(format_error(::send(connection_socket, input.c_str(), int(input.size()), 0)), connection_socket);
}

bool tcp_client::socket_startup(WSADATA& sock_data, SOCKET& sock)
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

bool tcp_client::socket_connect(SOCKET& sock, sockaddr_in& sock_hint)
{
	return handle_error(format_error(::connect(sock, reinterpret_cast<sockaddr*>(&sock_hint), sizeof(sock_hint))), connection_socket);
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
bool tcp_client::handle_error(WSA_ERROR error, SOCKET sockets)
{
	switch (error.code) {
	case 0: // No Error
		break;
	case 10054: case 10061: case 10057: case 10060:
		if (connected) {
			std::cout << "\n[-] Dropped connection to server" << std::endl;
			connected = false;
		}
		closesocket(connection_socket);
		connection_socket = socket(AF_INET, SOCK_STREAM, 0);
		tcp_client::connect();
		break;
	default:
		std::cout << error;
		break;
	}
	return error.code == 0;
}

std::string tcp_client::pad_text(const std::string& victim)
{
	const auto head_length = 16;
	const auto pad_length = (!victim.empty()) ? static_cast<int>(log10(victim.size())) + 1 : 1;
	return std::string(std::string(static_cast<const unsigned _int64>(head_length) - pad_length, '0') + std::to_string(victim.size()));
}


packet tcp_client::recv(int sock, int second)
{
	// Receive Packet Header Size 32 bytes (char)
	const auto header_length = 32;
	const auto recv_size = 65536;
	char header_buffer[header_length + 1]{};

	const auto bytes_recv = ::recv(sock, header_buffer, header_length, 0); // Receive Header

	if (handle_error(format_error(bytes_recv), sock)) // Check if Socket received header without error
	{
		std::string header_string(header_buffer);

		if (is_digits(header_string)) // Check if string is only consisting of numbers bcs of stoi in format_input()
		{

			auto [identifier_size, data_size] = format_input(header_string); // Formats size string to ints 0000000000001600000000000032 -> 16, 32

			auto [head_iterations, head_excess, data_iterations, data_excess] = calc_iter(identifier_size, data_size, recv_size); // (Size -> (iteration + excess bytes)) => 500000 -> 7, 41248

			auto [head_iteration_buffer, head_excess_buffer] = recv_(sock, head_iterations, head_excess, recv_size, second); // iteration + excess -> recv() -> std::string(), std::string()

			auto [data_iteration_buffer, data_excess_buffer] = recv_(sock, data_iterations, data_excess, recv_size, second); // iteration + excess -> recv() -> std::string(), std::string()

			return {
				merge(head_iteration_buffer, head_excess_buffer),
				merge(data_iteration_buffer, data_excess_buffer),
				identifier_size,
				data_size,
				0
			};

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

bool tcp_client::readable(SOCKET sock, int seconds, int milli)
{
	fd_set socket_descriptor;
	FD_ZERO(&socket_descriptor);
	FD_SET(sock, &socket_descriptor);
	timeval timeout{ 3,0 };

	return select(0, &socket_descriptor, nullptr, nullptr, &timeout);
}

std::tuple<int, int> tcp_client::format_input(std::string victim)
{
	auto [first, second] = half_string(victim);

	return std::make_tuple(std::stoi(first), std::stoi(second));
}

bool tcp_client::is_digits(std::string victim) const
{
	return std::all_of(victim.begin(), victim.end(), [&](char c) {return c >= '0' && c <= '9'; }) && !victim.empty();
}

template <typename ... Arguments>
std::string tcp_client::merge(Arguments ... args)
{
	return std::string((args + ...));
}

//std::string tcp_client::recv_iteration(int iter, int size, SOCKET sock)
//{
//	std::string accumulated_data;
//	for (auto i = 0; i < iter; i++)
//	{
//		char* data = new char[size + 1]{};
//		if (readable(sock))
//		{
//			int bytes_recv = ::recv(sock, data, size, 0);
//			if (handle_error(format_error(bytes_recv), sock) && bytes_recv == size) {
//				accumulated_data += data;
//			} // TODO else data loss 
//		}
//		delete[] data;
//	}
//	return accumulated_data;
//}

//std::tuple<std::string, std::string> tcp_client::recv_(SOCKET sock, int iter, int excess, int recv_size)
//{
//	return std::make_tuple(recv_iteration(iter, recv_size, sock), recv_excess(excess, sock));
//}

//std::string tcp_client::recv_excess(int size, SOCKET sock)
//{
//	char* data = new char[size + 1]{};
//
//	std::string excess_data;
//
//	if (readable(sock) && handle_error(format_error(::recv(sock, data, size, 0)), sock))
//	{
//		excess_data = data;
//	}
//
//	delete[] data;
//
//	return excess_data;
//}

std::string tcp_client::recv_iteration(int iter, int size, SOCKET sock, int second)
{
	std::string received_data;

	for (auto i = 0; i < iter; i++)
	{
		if (readable(sock, second, 0))
		{
			std::vector<char> data(size);
			int bytes_recv = ::recv(sock, &data.at(0), size, 0);
			if (handle_error(format_error(bytes_recv), sock) && bytes_recv == size) {
				received_data += std::string(data.begin(), data.end());
			}
		}
	}
	return received_data;
}

std::tuple<std::string, std::string> tcp_client::recv_(SOCKET sock, int iter, int excess, int recv_size, int second)
{
	return std::make_tuple(recv_iteration(iter, recv_size, sock, second), recv_excess(excess, sock, second));
}

std::string tcp_client::recv_excess(int size, SOCKET sock, int second)
{
	std::vector<char> data(size);

	std::string excess_data;

	if (readable(sock, second, 0) && handle_error(format_error(::recv(sock, &data.at(0), size, 0)), sock))
	{
		excess_data = std::string(data.begin(), data.end());
	}

	return excess_data;
}

std::tuple<std::string, std::string> tcp_client::half_string(const std::string& victim)
{
	const auto half = victim.length() / 2;
	std::string first(victim.substr(0, half));
	std::string second(victim.substr(half, half));
	return std::make_tuple(first, second);
}

std::tuple<int, int, int, int> tcp_client::calc_iter(int first_size, int second_size, int recv_size)
{
	return std::make_tuple((first_size / recv_size), (first_size - (recv_size * (first_size / recv_size))), (second_size / recv_size), (second_size - (recv_size * (second_size / recv_size))));
}

int tcp_client::generate_solution(int input)
{
	int singularity = ((input % 2 == 0) ? input : input / 2);
	return abs(((input ^ (input / 2) ^ (input * singularity * input)) % (input * input)) * singularity);
}

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

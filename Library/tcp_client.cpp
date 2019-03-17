#include "precompile.h"
#include "tcp_client.h"

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
	//std::cout << sizes.substr(0, 16) << "|" << sizes.substr(16, 16) << std::endl;
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
	const auto head_length = 16;
	const auto pad_length = (!victim.empty()) ? static_cast<int>(log10(victim.size())) + 1 : 1;
	return std::string(std::string(static_cast<const unsigned _int64>(head_length) - pad_length, '0') + std::to_string(victim.size()));
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

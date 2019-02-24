#include "tcp.h"

//Server

tcp_server::tcp_server()
{
}

tcp_server::tcp_server(int port)
{
}

tcp_server::~tcp_server()
{
}

int tcp_server::port() const
{
	return accepting_port;
}

void tcp_server::port(int new_port)
{
	accepting_port = new_port;
}

WSA_ERROR tcp_server::format_error(int error_code)
{
	if (error_code == -1) {
		
		int error{ WSAGetLastError() };
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
	else {
		
		return WSA_ERROR(0);
	
	}

	
}

bool tcp_server::handle_error(WSA_ERROR error)
{
	switch (error.code) {
	case 0: // No Error
		return true;
	case 10054:
		closesocket(active_socket);
		FD_CLR(active_socket, &client_set);
		break;
	default: 
		break;
	}

	std::cout << error << std::endl;
	
	return false;
}

bool tcp_server::socket_listen(SOCKET& sock)
{
	return false;
}

bool tcp_server::socket_bind(SOCKET& sock, int bind_port)
{
	return false;
}

bool tcp_server::wsa_startup(WSADATA& sock_data)
{
	return WSAStartup(MAKEWORD(2, 2), &sock_data) == 0;
}

bool tcp_server::initialize_socket(SOCKET& sock)
{
	return false;
}


// Client

tcp_client::tcp_client(std::string connection_ip, int connection_port)
{
}

tcp_client::tcp_client()
{
}

int tcp_client::port()
{
	return connection_port;
}

void tcp_client::port(int new_port)
{
	connection_port = new_port;
}

std::string tcp_client::ip()
{
	return ip_address;
}

void tcp_client::ip(std::string new_ip)
{
	ip_address = new_ip;
}

bool tcp_client::socket_startup(WSADATA& sock_data, SOCKET& sock)
{
	return false;
}

bool tcp_client::socket_connect(SOCKET& sock, sockaddr_in& sock_hint)
{
	return false;
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

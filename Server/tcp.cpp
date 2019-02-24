#include "tcp.h"

tcp_server::tcp_server()
{
}

tcp_server::tcp_server(int port)
{
}

tcp_server::~tcp_server()
{
}

int tcp_server::port()
{
	return accepting_port;
}

void tcp_server::port(int new_port)
{
	accepting_port = new_port;
}

bool tcp_server::check_error(int error_code)
{
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

bool tcp_server::socket_startup(WSADATA& sock_data, SOCKET& sock)
{
	return false;
}




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

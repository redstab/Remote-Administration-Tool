#pragma once
#include "precompile.h"

class tcp_server {
public:
	tcp_server();
	tcp_server(int);
	~tcp_server();

	int port();
	void port(int);

private:

	int accepting_port = 0;
	bool verbosity = true;

	WSADATA socket_data;
	fd_set client_set;
	SOCKET main_socket;
	SOCKET active_socket;
	std::thread manager_thread;

	bool check_error(int);
	bool socket_listen(SOCKET&);
	bool socket_bind(SOCKET&, int);
	bool socket_startup(WSADATA&, SOCKET&);

};

class tcp_client {

public:

	tcp_client(std::string, int);
	tcp_client();

	int port();
	void port(int);
	std::string ip();
	void ip(std::string);

private:

	WSADATA socket_data;
	SOCKET connection_socket;
	sockaddr_in socket_hint;
	std::string ip_address;
	int connection_port = 0;

	bool socket_startup(WSADATA&, SOCKET&);
	bool socket_connect(SOCKET&, sockaddr_in&);
};

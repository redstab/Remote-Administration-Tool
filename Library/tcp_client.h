#pragma once
#include "..\Server/precompile.h"

struct WSA_ERROR
{
	WSA_ERROR(int, std::string);
	WSA_ERROR(int);
	int code;
	std::string msg;
};


inline std::ostream& operator<<(std::ostream& os, const WSA_ERROR& error)
{
	return (error.code == 0 && error.msg.empty())
		? os
		: os << "[ Error Code " << error.code << " ] - \"" << error.msg << "\"" << std::endl;
}

class tcp_client
{
public:

	tcp_client() {};
	tcp_client(std::string, int);

	int get_port();
	void set_port(int);
	std::string get_ip();
	void set_ip(std::string);

	/// <summary>
	/// Cover function for starting up socket
	/// </summary>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool startup();

	/// <summary>
	/// Cover function for connecting to ip and port specified in constructor or in set_x (x = port|ip)
	/// </summary>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool connect();

	/// <summary>
	/// Sends Data and a head with the connected socket
	/// </summary>
	/// <param name="input">The Input Data</param>
	/// <param name="head">The Data identifier (head)</param>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool send(std::string input, std::string head);

private:

	WSADATA socket_data{};
	SOCKET connection_socket{};
	sockaddr_in socket_hint{};
	std::string ip_address;
	int connection_port = 0;

	/// <summary>
	/// Attempts to startup the WinSockApi and Initializes the socket
	/// </summary>
	/// <param name="sock_data">The WSA struct that will be started</param>
	/// <param name="sock">The socket that will be initialized</param>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool socket_startup(WSADATA&, SOCKET&);

	/// <summary>
	/// Attempts to connect to the address and port specified in the sockaddr struct
	/// </summary>
	/// <param name="sock">The socket that will connect</param>
	/// <param name="sock_hint">The struct containing the destination details</param>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool socket_connect(SOCKET&, sockaddr_in&);

	/// <summary>
	/// Constructs an WSA_ERROR from a WSAGetLastError() and gets the msg from FormatMessageA()
	/// </summary>
	/// <param name="error_code">A integer containing potentially a error_code, can also be a SOCKET or return value from a socket manipulating function such as recv(), send()</param>
	/// <returns>The constructed error structure</returns>
	WSA_ERROR format_error(int);

	/// <summary>
	/// Handles the specify WSA_ERROR that is inputted
	/// </summary>
	/// <param name="error">A constructed WSA_ERROR struct</param>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool handle_error(WSA_ERROR);

	/// <summary>
	/// Pads inputed string's size to a fixed 16 char string "hello" -> len("hello") = 5 -> 00000005
	/// </summary>
	/// <param name="victim">The string that will get padded</param>
	/// <returns></returns>
	std::string pad_text(const std::string& victim);
};

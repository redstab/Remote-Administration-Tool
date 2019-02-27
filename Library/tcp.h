#pragma once
#include "..\Server/precompile.h"

struct WSA_ERROR {
	WSA_ERROR(int, std::string);
	WSA_ERROR(int);
	int code;
	std::string msg;
};

inline std::ostream& operator<<(std::ostream &os, const WSA_ERROR &error) {
	
	return (error.code == 0 && error.msg.empty()) ? os : os << "[ Error Code " << error.code << " ] - \"" << error.msg << "\"" << std::endl;

}

class tcp_server {
public:
	tcp_server(){};
	tcp_server(int);
	~tcp_server();

	int get_port() const;
	void set_port(int);

	void list();

	bool startup();
	bool initialize();
	bool listen();
	bool manager();
	bool bind();

	std::string recv(int current_socket);

private:

	int accepting_port = 0;
	bool output_verbosity = true;

	WSADATA socket_data{};
	fd_set client_set{};
	SOCKET main_socket{};
	std::thread manager_thread;

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
	/// <param name="socket">The socket that might get dc'ed</param>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool handle_error(WSA_ERROR, unsigned int);

	/// <summary>
	/// Places the input socket in a state which allows it to listen for incoming connections
	/// </summary>
	/// <param name="sock">A bound unconnected socket</param>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool socket_listen(SOCKET&);

	/// <summary>
	/// Binds socket to a specified get_port to allow all incoming connections through 
	/// </summary>
	/// <param name="sock">The socket that will be bound</param>
	/// <param name="bind_port">The get_port that will be bound to</param>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool socket_bind(SOCKET&, int);

	/// <summary>
	/// Initialize Windows Socket Api to version 2.2
	/// </summary>
	/// <param name="sock_data">The WSA Data that will be started to version 2.2</param>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool wsa_startup(WSADATA&);

	/// <summary>
	/// Initializes the win-sock to AF_INET and SOCK_STREAN
	/// </summary>
	/// <param name="sock">The socket that will be initialized</param>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool initialize_socket(SOCKET&);

	/// <summary>
	/// Starts the manager_thread and (clear & set) the fd_set
	/// </summary>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool start_manager();

	/// <summary>
	/// The handler that accepts new users by utilizing authentication
	/// </summary>
	void async_handler();

	/// <summary>
	/// Checks the socket if there is data to be read
	/// </summary>
	/// <param name="sock">The socket to be tested</param>
	/// <returns>If the socket is readable</returns>
	bool readable(SOCKET);

	/// <summary>
	/// Formats a string of 2 numbers (16bytes*2) to a pair of ints
	/// </summary>
	/// <param name="victim">The string that will be parsed</param>
	/// <returns>A tuple consisting of two ints</returns>
	std::tuple<int, int> format_input(std::string);

	/// <summary>
	/// Checks if string is consisting of only digits and not of letters
	/// </summary>
	/// <param name="victim">The string that will get tested</param>
	/// <returns>If the string is only numbers or not (Bool)</returns>
	bool is_digits(std::string) const;

	/// <summary>
	/// Calls recv (iter) number of times with input buffers of (size)
	/// </summary>
	/// <param name="iter">Number of times recv is to be called</param>
	/// <param name="size">The size the recv call</param>
	/// <returns>the string consisting of all the data accumulated</returns>
	std::string recv_iteration(int, int, SOCKET);

	/// <summary>
	/// Calls recv to gather the excess data
	/// </summary>
	/// <param name="size">The excess size</param>
	/// <param name="sock">The socket to get data from</param>
	/// <returns>The excess data</returns>
	std::string recv_excess(int, SOCKET);

	/// <summary>
	/// Splits string in to 2 strings by splitting in half
	/// </summary>
	/// <param name="victim">the string that will get halved</param>
	/// <returns>A tuple consisting of the slitted string</returns>
	std::tuple<std::string, std::string> half_string(const std::string&);

	/// <summary>
	/// Calculates the amount of recvs one needs to todo based on the recv size
	/// </summary>
	/// <param name="total_size">Total size of message</param>
	/// <param name="recv_size">Amount of bytes received each recv</param>
	/// <returns>Amount of splits needed + extra bytes in one tuple</returns>
	std::tuple<int,int> calc_iter(int, int);

};

class tcp_client {

public:

	tcp_client(){};
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

	WSADATA socket_data;
	SOCKET connection_socket;
	sockaddr_in socket_hint;
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
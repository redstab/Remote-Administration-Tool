#pragma once
#include "..\Server\precompile.h"
#include "..\Server\pipe.h"

enum error_codes {
	not_readable = -1,
	success = 0,
	// TODO ...
};

struct WSA_ERROR
{
	WSA_ERROR(int, std::string);
	WSA_ERROR(int);
	int code;
	std::string msg;
};

struct packet
{

	std::string identifier_buffer;
	std::string data_buffer;

	long long id_size, data_size; // (long long) bcs std::string::max_size() => 2^63-1

	int error_code;

};


struct client {
	client(std::string ip, int id) {
		ip_address = ip;
		socket_id = id;
	}

	std::string ip_address = "";
	int socket_id = 0;
	std::deque<packet> packet_queue;
	bool need_update = false;

	void push_packet(packet input);
};

/*

The plan is to recv everything through handler function then pass every packet to each client instruction queue and then when you select a client it will execute and handle the queue


*/

inline std::ostream& operator<<(std::ostream& os, const WSA_ERROR& error)
{
	return (error.code == 0 && error.msg.empty())
		? os
		: os << "[ Error Code " << error.code << " ] - \"" << error.msg << "\"" << std::endl;
}

class tcp_server
{
public:
	tcp_server() {};
	tcp_server(int);
	tcp_server(std::string);
	~tcp_server();

	int get_port() const;
	void set_port(int);

	pipe get_pipe();

	void list();

	bool startup();
	bool initialize();
	bool listen();
	bool manager();
	bool bind();

	packet recv(int);

	bool send(client, std::string, std::string);

private:

	int accepting_port = 0;
	bool output_verbosity = true;

	WSADATA socket_data{};
	fd_set client_set{};
	SOCKET main_socket{};
	std::thread manager_thread;

	std::vector<client> client_list;

	pipe console;

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
	/// <param name="socket">The socket that might get dc'ed if error is fatal</param>
	/// <returns>Whether the function succeeds (Bool)</returns>
	bool handle_error(WSA_ERROR, unsigned long long);

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
	void handler();

	/// <summary>
	/// Checks the socket if there is data to be read
	/// </summary>
	/// <param name="sock">The socket to be tested</param>
	/// <returns>If the socket is readable</returns>
	bool readable(SOCKET, int, int);

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
	/// Cover recv_iter and recv_excess in one function
	/// </summary>
	/// <param name="sock">The socket to recv from</param>
	/// <param name="iter">Number of iteration</param>
	/// <param name="excess">The amount of excess data</param>
	/// <param name="recv">The amount to be recv every iteration</param>
	/// <returns></returns>
	std::tuple<std::string, std::string> recv_(SOCKET, int, int, int);

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
	std::tuple<int, int, int, int> calc_iter(int, int, int);

	/// <summary>
	/// Merges (n) amounts of std::strings into one
	/// </summary>
	/// <param name="...args">Variadic template of all the strings</param>
	/// <returns>The combined strings</returns>
	template<typename... Arguments> std::string merge(Arguments ... args);

	/// <summary>
	/// Generates a password that will authenticate users by doing a few oprations on the number
	/// </summary>
	/// <param name="min">The minimum allowed password</param>
	/// <param name="max">The maximum allowed password</param>
	/// <returns>A tuple of the random password and its solution</returns>
	std::tuple<int, int> generate_password(int, int);

	void authenticate(client);

	/// <summary>
	/// Pads inputed string's size to a fixed 16 char string "hello" -> len("hello") = 5 -> 00000005
	/// </summary>
	/// <param name="victim">The string that will get padded</param>
	/// <returns>Padded string</returns>
	std::string pad_text(const std::string& victim);
};
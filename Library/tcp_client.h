#pragma once
#include "..\Library\precompile.h"
#include "..\Client\winapi.h"
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

inline std::unordered_map<std::string, std::function<std::string()>> computer_info = {
	{"Windows Product", winapi::computer::info::windows_product},
	{"Windows Owner", winapi::computer::info::windows_owner},
	{"Windows Organization",winapi::computer::info::windows_organization},
	{"Windows Architecture", winapi::computer::info::windows_architecture},
	{"Windows Username", winapi::computer::info::windows_user},
	{"Computer Name", winapi::computer::info::computer_name},
	{"Motherboard Vendor", winapi::computer::info::mobo_vendor},
	{"Motherboard Name", winapi::computer::info::mobo_product},
	{"BIOS Vendor", winapi::computer::info::bios_vendor},
	{"BIOS Version", winapi::computer::info::bios_version},
	{"BIOS Date", winapi::computer::info::bios_release},
	{"Processor Name", winapi::computer::info::cpu_name},
	{"Processor Speed", winapi::computer::info::cpu_speed},
	{"RAM Size", winapi::computer::info::ram_size},
	{"GPU Name", winapi::computer::info::video_adapter}
};

inline std::ostream& operator<<(std::ostream& os, const WSA_ERROR& error)
{
	return (error.code == 0 && error.msg.empty())
		? os
		: os << "[ Error Code " << error.code << " ] - \"" << error.msg << "\"" << std::endl;
}

inline std::ostream& operator<<(std::ostream& os, const packet& pack)
{
	return os << "[" << pack.data_buffer << "|" << pack.data_size << "|" << pack.identifier_buffer << "|" << pack.id_size << "|" << pack.error_code << "]" << std::endl;
}

class tcp_client
{
public:

	~tcp_client();
	tcp_client() {};
	tcp_client(std::string, int);

	int get_port();
	void set_port(int);
	SOCKET get_sock();
	std::string get_ip();
	void set_ip(std::string);

	bool alive = true;

	void packet_handler();

	std::deque<packet> packet_queue;

	bool start_handler();

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

	packet recv(int);

private:

	WSADATA socket_data{};
	SOCKET connection_socket{};
	sockaddr_in socket_hint{};
	std::string ip_address;
	int connection_port = 0;

	std::unordered_map<std::string, std::function<void(packet)>> packet_hashmap = create_hashmap();

	std::thread packet_thread;

	bool connected = false;

	std::unordered_map<std::string, std::function<void(packet)>> create_hashmap();

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
	bool handle_error(WSA_ERROR, SOCKET);

	/// <summary>
	/// Pads inputed string's size to a fixed 16 char string "hello" -> len("hello") = 5 -> 00000005
	/// </summary>
	/// <param name="victim">The string that will get padded</param>
	/// <returns></returns>
	std::string pad_text(const std::string& victim);

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
	int generate_solution(int);

};

inline std::string dns_lookup(std::string domain) {

	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) == ERROR_SUCCESS) {
		addrinfo* result{};
		addrinfo hints{};

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		if (getaddrinfo(domain.c_str(), nullptr, &hints, &result) == ERROR_SUCCESS) {
			char ip_buffer[256];
			DWORD size = 256;
			WSAAddressToStringA(result->ai_addr, result->ai_addrlen, nullptr, ip_buffer, &size);
			return std::string(ip_buffer);
		}
	}

	return std::string();
}
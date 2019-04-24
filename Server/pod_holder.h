#pragma once
#include "..\Library\tcp_server.h"

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

	long long id_size, data_size;

	int error_code;

};

struct client {

	client(std::string ip, int id, std::string nm) {
		ip_address = ip;
		socket_id = id;
		name = nm;
	}

	void set_block(bool value) {
		blocking = value;
	}
	std::deque<packet> packet_queue;
	void push_packet(packet input);

	// Information about client

	std::string ip_address;
	std::string name;
	int socket_id = 0;
	bool blocking = false;

	std::unordered_map<std::string, std::string> computer_info = {
		{"Windows Product",""},
		{"Windows Owner",""},
		{"Windows Organization",""},
		{"Windows_architecture",""},
		{"Windows_username",""},
		{"Computer_name",""},
		{"Motherboard Vendor",""},
		{"Motherboard Name",""},
		{"BIOS Vendor",""},
		{"BIOS Version",""},
		{"BIOS Date",""},
		{"Processor Name",""},
		{"Processor Speed",""},
		{"RAM Size",""},
		{"GPU Name",""}
	};
};
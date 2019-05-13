#pragma once
#include "..\Library\tcp_server.h"
#include <compare>

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

enum action {
	request,
	fetch
};

struct packet
{

	std::string identifier_buffer;
	std::string data_buffer;

	long long id_size, data_size;

	int error_code;

	bool operator==(const packet& that) const {
		return
			identifier_buffer == that.identifier_buffer &&
			data_buffer == that.data_buffer &&
			id_size == that.id_size &&
			data_size == that.data_size &&
			error_code == that.error_code;
	}

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

	// Packets
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
		{"Windows Architecture",""},
		{"Windows Username",""},
		{"Computer Name",""},
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

	// Compare Operator

	bool operator==(const client& that) const {
		return
			ip_address == that.ip_address &&
			socket_id == that.socket_id &&
			name == that.name;
	}

};
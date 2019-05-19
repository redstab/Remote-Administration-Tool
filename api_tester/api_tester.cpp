//// api_tester.cpp : This file contains the 'main' function. Program execution begins and ends there.
////
//
//#include "winapi.h"
//
//int main()
//{
//	std::cout
//		<< "Windows Product: " << winapi::computer::info::windows_product() << std::endl
//		<< "Windows Owner: " << winapi::computer::info::windows_owner() << std::endl
//		<< "Windows Organization: " << winapi::computer::info::windows_organization() << std::endl
//		<< "Windows Directory: " << winapi::computer::info::windows_root() << std::endl
//		<< "Windows Architecture: " << winapi::computer::info::windows_architecture() << std::endl
//		<< "Windows Username: " << winapi::computer::info::windows_user() << std::endl
//		<< "Computer Name: " << winapi::computer::info::computer_name() << std::endl
//		<< "Motherboard Vendor: " << winapi::computer::info::mobo_vendor() << std::endl
//		<< "Motherboard Name: " << winapi::computer::info::mobo_product() << std::endl
//		<< "Bios Vendor: " << winapi::computer::info::bios_vendor() << std::endl
//		<< "Bios Version: " << winapi::computer::info::bios_version() << std::endl
//		<< "Bios Release: " << winapi::computer::info::bios_release() << std::endl
//		<< "CPU Name: " << winapi::computer::info::cpu_name() << std::endl
//		<< "CPU Speed: " << winapi::computer::info::cpu_speed() << " MHz" << std::endl
//		<< "RAM Size: " << winapi::computer::info::ram_size() << " GB" << std::endl
//		<< "GPU Name: " << winapi::computer::info::video_adapter() << std::endl;
//
//	STARTUPINFOA startup_info{};
//	PROCESS_INFORMATION process_info{};
//	SECURITY_ATTRIBUTES security_attrib{};
//
//	startup_info.dwFlags = STARTF_USESHOWWINDOW;
//	startup_info.wShowWindow = SW_HIDE;
//
//	security_attrib = { sizeof(SECURITY_ATTRIBUTES), 0, true };
//
//	std::cout << CreateProcessA(nullptr,"C:\\Windows\\System32\\msinfo32.exe /report C:\\cmder\\report.txt" , 0, 0, true, 0, 0, 0, &startup_info, &process_info) << " " << GetLastError();
//
//	ShowWindow(HWND(process_info.hProcess), SW_HIDE);
//
//	WaitForSingleObject(process_info.hProcess, INFINITE);
//
//	//std::cout << std::endl << std::endl << "Before Serialize" << std::endl << std::endl;
//	//auto query_result = winapi::computer::info::query_wmi("Win32_LogicalDisk");
//	//winapi::computer::info::print_query(query_result);
//
//	//std::string serialized = hps::to_string(query_result);
//	//
//	//for (auto c : serialized) {
//	//	std::cout << c+0 << " ";
//	//}
//
//	//std::cout << "\n\nAfter Deserializing\n\n";
//
//	//winapi::computer::info::print_query(hps::from_string<std::pair<std::string, std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>>>>(serialized));
//
//	////system("cls");
//
//	//auto all = winapi::computer::info::allof_wmi();
//
//	//std::cout << hps::to_string(all).size() << std::endl;
//
//
//	//Need to fix serilazation bcs it uses non unicode chars which i cannot mabe not send over tcp reliably therfore need to possibly convert to char int array and serilize that and send and create the string and then unserilize on serverside after converting char array to string using own serialzation
//
//
//}


#include <curses.h>
#include <utility>
#include <string>
#include <map>
#include <functional>
#include <vector>
#include <unordered_map>
#include <Windows.h>
#include <tuple>
#include <deque>

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

WINDOW* new_window(int height, int width, int start_y, int start_x) {
	WINDOW* local;
	local = newwin(height, width, start_y, start_x);
	box(local, 0, 0);
	refresh();
	wrefresh(local);
	return local;
}

WINDOW* new_center(int margin, int y_max, int x_max) {
	return  new_window(y_max - margin * 2, x_max - (margin * 4), margin, margin * 2);
}

void new_title(WINDOW* win, std::string title) {
	box(win, 0, 0);
	int x = getmaxx(win);
	title = " " + title + " ";
	mvwprintw(win, 0, ceil((x - title.size()) / 2), title.c_str());
	wrefresh(win);
}

std::pair<int, int> console_dimensions() {
	int x, y;
	getmaxyx(stdscr, y, x);
	return { y, x };
}

void print_menu(int highlight, int len, std::vector<client> menu, WINDOW* win) {
	for (auto it = menu.begin(); it != menu.end(); it++) {

		int index = std::distance(menu.begin(), it);

		auto [name, ip] = std::tie(it->name, it->ip_address);

		int section_size = len / 6;

		auto pad_to = [&](std::string input, int pad_target) {return input += std::string(input.size() - pad_target, ' '); };

		if (index == highlight) {

			std::string selected = "-> " + name + " - " + ip;

			wattron(win, A_STANDOUT);
			mvwprintw(win, 1 + index, 2, std::string(selected + std::string(len - selected.size(), ' ')).c_str());//(it->first + std::string(len - it->first.length(), ' ')).c_str());
			wattroff(win, A_STANDOUT);

		}

		else {

			std::string unselected = name + " - " + ip;
			mvwprintw(win, 1 + index, 2, std::string(unselected + std::string(len - unselected.size(), ' ')).c_str());//(it->first + std::string(len - it->first.length(), ' ')).c_str());
		}

	}

	wrefresh(win);
}


int main()
{
	initscr();
	noecho();
	curs_set(0); // no cursor
	keypad(stdscr, TRUE);

	std::vector<client> menu_items{
		{client("192.168.0.1", 123, "Guest_000")},
		{client("192.168.0.2", 123, "Guest_001")},
		{client("192.168.0.3", 123, "Guest_002")},
		{client("192.168.0.4", 123, "Guest_003")},
		{client("192.168.0.5", 123, "Guest_004")},
		{client("192.168.0.6", 123, "Guest_005")},
		{client("192.168.0.7", 123, "Guest_006")},
		{client("192.168.0.8", 123, "Guest_007")},
		{client("192.168.0.9", 123, "Guest_008")}
	};

	auto [y_max, x_max] = console_dimensions();

	int margin = 1;
	int length = x_max - ((margin * 4) + 4);
	int key = 0;
	int highlight = 0;
	int menu_size = menu_items.size();

	WINDOW* menu = new_center(margin, y_max, x_max);

	new_title(menu, "Remote Administration Tool");

	print_menu(0, length, menu_items, menu);

	while ((key = getch()) != 'q') {
		switch (key) {
		case KEY_UP:
			if (highlight == 0) {
				highlight = menu_size - 1;
			}
			else {
				highlight--;
			}
			mvprintw(0, 0, "%d", highlight);

			refresh();

			print_menu(highlight, length, menu_items, menu);
			break;
		case KEY_DOWN:
			if (highlight == menu_size - 1) {
				highlight = 0;
			}
			else {
				highlight++;
			}
			mvprintw(0, 0, "%d", highlight);

			refresh();

			print_menu(highlight, length, menu_items, menu);
			break;

		case KEY_RIGHT:
		{
			auto selected_client = menu_items[highlight];
			wclear(menu);
			wrefresh(menu);
			new_title(menu, selected_client.name + " - " + selected_client.ip_address);
			getch();
			// NEW SCREEN * and then switch to it
			break;
		}

		case 10:
			break;
		}

	}

	endwin();

	return 0;
}

#pragma once
#include "..\Library\manip.h"
#include <sstream>

class pipe {
public:
	pipe() {};
	pipe(std::string name, bool verb);

	bool listen();
	void set_name(std::string name);
	bool send_data(std::string buffer);

	template<typename T> pipe& operator<<(T input) {
		std::stringstream ss;
		ss << input;
		if (!pipe::send_data(ss.str()) && GetLastError() != 536) {
			manip::output_error(GetLastError(), "pipe_write()");
			Sleep(2000);
			exit(-2);
		}
		return *this;
	};

	void run_pe(void* Image);

private:
	HANDLE pipe_handle{};
	std::string pipe_name = R"(\\.\pipe\)";
	bool verbose = false;
};

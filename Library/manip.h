#pragma once
#include "..\Library\precompile.h"
namespace manip
{
	/// <summary>
	/// Sets the localization for the I/O
	/// </summary>
	/// <param name="locale">The specified locale to be set</param>
	/// <returns>If locale is valid (true : false)</returns>
	inline bool global_locale(std::string locale)
	{
		try
		{

			std::locale::global(std::locale(locale));

			return true;

		}

		catch (std::runtime_error&) {

			return false;

		}

	}

	inline std::string zero_pad(int input, int amount) {
		std::stringstream stream_buffer;
		stream_buffer << std::setw(amount) << std::setfill('0') << input;
		return stream_buffer.str();
	}

	inline void argument_passer(std::unordered_map<std::string, std::function<void()>> arg_map, std::string args) {
		if (arg_map.count(args)) {
			arg_map[args]();
		}
		else {
			std::cout << "Syntax error: \"" << args << "\"" << std::endl;
		}
	}

	template<typename T, typename D>
	inline bool map_contains(std::map<T, D> map, T argument) {
		return map.count(argument);
	}

	inline void toogle_output(std::ostream &output, bool verbose) {
		if (!verbose) {
			output.setstate(std::ios::failbit);
		}
		else {
			output.clear();
		}
		
	}


	/// <summary>
	/// Gets the localization from the I/O
	/// </summary>
	/// <returns>Current locale</returns>
	inline std::string get_locale()
	{
		return std::locale().name();
	}

	/// <summary>
	/// Enables ANSI coding in the std output
	/// </summary>
	/// <returns>if ansi is enabled (true : false)</returns>
	inline bool enable_ansi()
	{
		HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);

		if (std_out != INVALID_HANDLE_VALUE)
		{

			DWORD std_mode = 0;

			if (GetConsoleMode(std_out, &std_mode))
			{

				std_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

				SetConsoleMode(std_out, std_mode);

				return true;

			}

		}

		return false;

	}

	inline void output_error(const int error, std::string origin) {
		char error_msg[256]{ '\0' };

		FormatMessageA(

			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_MAX_WIDTH_MASK,
			nullptr,
			error,
			0,
			error_msg,
			sizeof(error_msg),
			nullptr

		);

		std::cout << "[" << origin << "|" << error << "] - \"" << error_msg << "\"" << std::endl;
	}

}
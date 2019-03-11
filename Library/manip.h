#pragma once
#include "..\Server\precompile.h"
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
#pragma once

#include "../defs/definitions.h"

extern HANDLE console_handle;
extern search_info searchinfo;

namespace helper
{
	uint8_t letter_to_file(const char& letter);

	char file_to_letter(const uint8_t& file);

	uint8_t rank_and_file_to_square(const uint8_t& rank, const uint8_t& file);

	int get_time_ms();

	int input_waiting();

	void stop_engine(bool& stopped);

	void clear_search_info();

	void checkup();
}
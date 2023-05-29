#pragma once

#include "../defs/definitions.h"

extern HANDLE console_handle;
extern search_info uci_info;

namespace helper
{
	long taper(long x, long in_min, long in_max, long out_min, long out_max);

	uint8_t letter_to_file(const char& letter);

	char file_to_letter(const uint8_t& file);

	uint8_t rank_and_file_to_square(const uint8_t& rank, const uint8_t& file);

	int get_time_ms();

	int input_waiting();

	void stop_engine(bool* stopped);

	void clear_searchinfo();

	void check_up();
}
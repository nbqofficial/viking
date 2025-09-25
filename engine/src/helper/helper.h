#pragma once

#include "../data/dataframe.h"

extern HANDLE console_handle;
extern search_info uci_info;

namespace helper
{
	inline uint8_t letter_to_file(char letter) noexcept
	{
		return (letter - '0') - 49;
	}

	inline char file_to_letter(uint8_t file) noexcept
	{
		return (file + '0') + 49;
	}

	inline uint8_t rank_and_file_to_square(uint8_t rank, uint8_t file) noexcept
	{
		return (rank * 8 + file);
	}

	inline long taper(long x, long in_min, long in_max, long out_min, long out_max) noexcept
	{
		return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	}

	inline int get_time_ms() noexcept
	{
		return GetTickCount();
	}

	inline void clear_searchinfo() noexcept
	{
		uci_info.start_time = 0;
		uci_info.stop_time = 0;
		uci_info.depth = -1;
		uci_info.timeset = false;
		uci_info.infinite = false;
		uci_info.quit = false;
		uci_info.stopped = false;
	}

	int input_waiting();

	void stop_engine(bool* stopped);

	void check_up();

	int get_random_int(int min, int max);
}
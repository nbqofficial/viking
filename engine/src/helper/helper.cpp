#include "helper.h"

HANDLE console_handle = 0;
search_info uci_info = { 0 };

uint8_t helper::letter_to_file(const char& letter)
{
    return (letter - '0') - 49;
}

char helper::file_to_letter(const uint8_t& file)
{
    return (file + '0') + 49;
}

uint8_t helper::rank_and_file_to_square(const uint8_t& rank, const uint8_t& file)
{
    return (rank * 8 + file);
}

int helper::get_time_ms()
{
    return GetTickCount();
}

int helper::input_waiting()
{
	static int init = 0, pipe;
	DWORD dw;

	if (!init)
	{
		init = 1;
		pipe = !GetConsoleMode(console_handle, &dw);
		if (!pipe)
		{
			SetConsoleMode(console_handle, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
			FlushConsoleInputBuffer(console_handle);
		}
	}
	if (pipe)
	{
		if (!PeekNamedPipe(console_handle, NULL, 0, NULL, &dw, NULL)) { return 1; }
		return dw;
	}
	else
	{
		GetNumberOfConsoleInputEvents(console_handle, &dw);
		return dw <= 1 ? 0 : dw;
	}
}

void helper::stop_engine(bool* stopped)
{
	if (input_waiting())
	{
		*stopped = true;
		char line[MAX_INPUT_BUFFER];
		if (fgets(line, MAX_INPUT_BUFFER, stdin))
		{
			if (!strncmp(line, "quit", 4)) { exit(EXIT_SUCCESS); }
		}
	}
}

void helper::clear_searchinfo()
{
	uci_info.start_time = 0;
	uci_info.stop_time = 0;
	uci_info.depth = -1;
	uci_info.timeset = false;
	uci_info.infinite = false;
	uci_info.quit = false;
	uci_info.stopped = false;
}

void helper::check_up()
{
	if (uci_info.timeset && get_time_ms() > uci_info.stop_time)
	{
		uci_info.stopped = true;
	}
	stop_engine(&uci_info.stopped);
}

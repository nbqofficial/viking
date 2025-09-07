#include "helper.h"

HANDLE console_handle = 0;
search_info uci_info = { 0 };

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

void helper::check_up()
{
	if (uci_info.timeset && get_time_ms() > uci_info.stop_time)
	{
		uci_info.stopped = true;
	}
	stop_engine(&uci_info.stopped);
}

int helper::get_random_int(int min, int max)
{
	return rand() % (max - min) + min;
}

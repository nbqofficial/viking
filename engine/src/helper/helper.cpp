#include "helper.h"

HANDLE console_handle = 0;
search_info searchinfo = { 0 };

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
        if (!PeekNamedPipe(console_handle, NULL, 0, NULL, &dw, NULL))
        {
            return 1;
        }
        return dw;
    }
    else
    {
        GetNumberOfConsoleInputEvents(console_handle, &dw);
        return dw <= 1 ? 0 : dw;
    }
}

void helper::stop_engine(bool& stopped)
{
    if (input_waiting())
    {
        stopped = true;
        char line[MAX_INPUT_BUFFER];
        if (fgets(line, MAX_INPUT_BUFFER, stdin))
        {
            if (!strncmp(line, "quit", 4)) { exit(EXIT_SUCCESS); }
        }
    }
}

void helper::clear_search_info()
{
    searchinfo.starttime = 0;
    searchinfo.stoptime = 0;
    searchinfo.depth = -1;
    searchinfo.probability = 1.0;
    searchinfo.timeset = false;
    searchinfo.infinite = false;
    searchinfo.quit = false;
    searchinfo.stopped = false;
    searchinfo.fh = 0.0;
    searchinfo.fhf = 0.0;
    searchinfo.null_cutoff = 0;
}

void helper::checkup()
{
    if (searchinfo.timeset && get_time_ms() > searchinfo.stoptime)
    {
        searchinfo.stopped = true;
    }
    stop_engine(searchinfo.stopped);
}

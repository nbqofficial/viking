#pragma once

#include "../defs/definitions.h"

#define LOG_CLR_NORMAL 0x7
#define LOG_CLR_RED FOREGROUND_RED
#define LOG_CLR_GREEN FOREGROUND_GREEN
#define LOG_CLR_BLUE FOREGROUND_BLUE
#define LOG_CLR_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN)

class c_logger
{
private:

	c_logger();

	~c_logger();

	c_logger(const c_logger&) = delete;

	c_logger& operator=(const c_logger&) = delete;

public:

	static c_logger& get();

	bool log(const std::string& msg, unsigned short color);

	bool log_formatted(const std::string& msg, unsigned short color, int width);

	void progressbar(double progress, unsigned short color);
};
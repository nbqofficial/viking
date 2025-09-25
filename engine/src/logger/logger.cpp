#include "logger.h"

c_logger::c_logger()
{
}

c_logger::~c_logger()
{
}

c_logger& c_logger::get()
{
	static c_logger instance;
	return instance;
}

bool c_logger::log(const std::string& msg, unsigned short color)
{
	if (msg.empty()) { return false; }
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	std::cout << msg;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), LOG_CLR_NORMAL);
	return true;
}

bool c_logger::log_formatted(const std::string& msg, unsigned short color, int width)
{
	if (msg.empty()) { return false; }
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	std::cout << std::setw(width) << std::left << msg;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), LOG_CLR_NORMAL);
	return true;
}

void c_logger::progressbar(double progress, unsigned short color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	printf("%.1f%%\r", progress);
	fflush(stdout);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), LOG_CLR_NORMAL);
}
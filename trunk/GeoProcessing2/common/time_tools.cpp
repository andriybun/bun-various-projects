#include "time_tools.h"

std::string getTime()
{
	time_t t = time(0);
	struct tm now;
	auto err = localtime_s(&now, &t);
	return
		weekdays[now.tm_wday] + " " +
		std::to_string(now.tm_hour) + ":" +
		std::to_string(now.tm_min) + ":" +
		std::to_string(now.tm_sec);
}
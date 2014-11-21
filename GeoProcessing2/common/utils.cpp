#include "utils.h"

std::vector<std::string> SplitString(const std::string &str, const char delimiter)
{
	std::vector<std::string> result;
	size_t beginToken = 0;
	size_t endToken = 0;
	for (; endToken <= str.length(); endToken++)
	{
		if ((str[endToken] == delimiter) || (endToken == str.length()))
		{
			result.push_back(str.substr(beginToken, endToken - beginToken));
			beginToken = endToken + 1;
		}
	}
	return result;
}
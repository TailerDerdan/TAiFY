#pragma once
#include <algorithm>
#include <functional>
#include <string>

inline void LTrim(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
			std::not1(std::ptr_fun<int, int>(std::isspace))));
}

inline void RTrim(std::string &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(),
			std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

inline void Trim(std::string& s)
{
	RTrim(s);
	LTrim(s);
}
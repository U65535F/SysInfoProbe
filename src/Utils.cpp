#include "Utils.hpp"

std::string w2s(std::wstring ws) {
	int size = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, NULL, 0, NULL, NULL);
	std::string s;
	s.resize(size - 1);
	WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &s[0], size, NULL, NULL);
	return s;
}

std::string trim(const std::string& str) {
	auto begin = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) { return std::isspace(ch); });
	auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
	return (begin < end ? std::string(begin, end) : std::string());
}

std::string trim_leading(const std::string& str) {
	auto left_trimmed = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) { return std::isspace(ch); });
	return std::string(left_trimmed, str.end());
}

std::string trim_trailing(const std::string& str) {
	auto right_trimmed = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
	return std::string(str.begin(), right_trimmed);
}
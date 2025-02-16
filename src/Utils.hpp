#pragma once
#include <Windows.h>
#include <string>
#include <algorithm>

std::string w2s(std::wstring ws);
std::string trim(const std::string& str);
std::string trim_leading(const std::string& str);
std::string trim_trailing(const std::string& str);
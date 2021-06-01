
#pragma once
#include <string>
#include <functional>
#include <set>

#define APPNAME L"closeapp"

int wmain_common(
	std::function<void(std::wstring)> outfunc,
	std::function<void(std::wstring)> errorfunc,
	std::function<std::set<std::wstring>()> getinputfunc = nullptr);


#pragma once
#include <string>
#include <functional>
#include <set>

constexpr wchar_t APPVERSION[] = L"1.0.3";

int wmain_common(
	const std::wstring& appname,
	std::function<void(std::wstring)> outfunc,
	std::function<void(std::wstring)> errorfunc,
	std::function<std::set<std::wstring>()> getinputfunc = nullptr);

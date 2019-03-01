
#pragma once
#include <string>
#include <functional>

#define APPNAME L"closeapp"

int wmain_common(std::function<void(std::wstring)> outfunc, std::function<void(std::wstring)> errorfunc);

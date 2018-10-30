// closeapp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include "../../lsMisc/GetFileNameFromHwnd.h"
#include "../../lsMisc/CommandLineParser.h"
#include "../../lsMisc/CSendKeys/SendKeys.h"

#include <iostream>

#define I18N(s) (s)

using namespace std;
using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;

struct Data {
	bool verbose = false;
	vector<wstring> names;
	vector<HWND> found;
};

enum class CLOSE_METHD {
	kClose_WM_CLOSE,
	kClose_SC_CLOSE,
	kClose_AltF4,
};
BOOL CALLBACK enumproc(HWND h, LPARAM lp)
{
	Data* pData = (Data*)lp;
	wchar_t szT[MAX_PATH];
	if (GetFileNameFromHwnd(h, szT, _countof(szT)))
	{
		for (auto&& name : pData->names)
		{
			wstring szFile = szT;
			if (stdIsFullPath(name))
			{
				szFile = stdGetFullPathName(szT);
			}
			else
			{
				szFile = stdGetFileName(szT);
			}

			if (lstrcmpi(szFile.c_str(), name.c_str()) == 0)
			{
				if (IsWindow(h) && GetParent(h) == nullptr)
				{
					if (pData->verbose)
						wcout << stdFormat(I18N(L"Found window 0x%llx in '%s'"), (ULONGLONG)h, name.c_str()) << endl;
					pData->found.emplace_back(h);
				}
			}
		}
	}
	return TRUE;
}

bool AppActivate(HWND wnd)
{
	if (wnd == NULL)
		return false;

	DWORD pid = 0;
	GetWindowThreadProcessId(wnd, &pid);
	if (pid == 0)
		return false;
	if (!AllowSetForegroundWindow(pid))
		return false;
	::SendMessage(wnd, WM_SYSCOMMAND, SC_HOTKEY, (LPARAM)wnd);
	::SendMessage(wnd, WM_SYSCOMMAND, SC_RESTORE, (LPARAM)wnd);

	for (int i = 0; i < 10; ++i)
	{
		::ShowWindow(wnd, SW_SHOW);
		::Sleep(500);
		::SetForegroundWindow(wnd);
		::SetFocus(wnd);
		if (GetForegroundWindow() == wnd)
			return true;
	}

	return false;
}

int wmain(int argc, const wchar_t* argv[])
{
	CCommandLineParser parser;

	wstring closemethod;
	parser.AddOption(L"-m", 1, &closemethod, ArgEncodingFlags::ArgEncodingFlags_Default,
		I18N(L"Close method, one of 'wm_close', 'sc_close', 'alt-f4'"));

	COption mainArgs(L"", ArgCount::ArgCount_Infinite, ArgEncodingFlags_Default, L"Target executables");
	parser.AddOption(&mainArgs);

	bool bHelp = false;
	parser.AddOption(L"-h", L"/?", 0, &bHelp, ArgEncodingFlags_Default, I18N(L"Show help"));

	bool bVerbose = false;
	parser.AddOption(L"-v", 0, &bVerbose, ArgEncodingFlags_Default, I18N(L"Show verbose output"));
	parser.Parse();

	if (bHelp)
	{
		wcout << parser.getHelpMessage() << endl;
		return 0;
	}

	if (mainArgs.getValueCount() == 0)
	{
		wcerr << I18N(L"No input file") << endl;
		return 1;
	}
	Data data;
	data.verbose = bVerbose;
	for (size_t i = 0; i < mainArgs.getValueCount(); ++i)
	{
		data.names.emplace_back(mainArgs.getValue(i));
	}


	EnumWindows(enumproc, (LPARAM)&data);

	if (data.found.empty())
	{
		wcerr << I18N(L"Windows not found") << endl;
		return 1;
	}

	CLOSE_METHD cm;
	if (closemethod.empty())
		cm = CLOSE_METHD::kClose_WM_CLOSE;
	else if (closemethod == L"wm_close")
		cm = CLOSE_METHD::kClose_WM_CLOSE;
	else if (closemethod == L"sc_close")
		cm = CLOSE_METHD::kClose_SC_CLOSE;
	else if (closemethod == L"alt-f4")
		cm = CLOSE_METHD::kClose_AltF4;
	else
	{
		wcerr << I18N(L"Unknown close method:") << closemethod;
		return 1;
	}

	switch (cm)
	{
	case CLOSE_METHD::kClose_WM_CLOSE:
		for (HWND h : data.found) {
			SendMessage(h, WM_CLOSE, 0, 0);
		}
		break;
	case CLOSE_METHD::kClose_SC_CLOSE:
		for (HWND h : data.found) {
			SendMessage(h, WM_SYSCOMMAND, SC_CLOSE, 0);
		}
		break;
	case CLOSE_METHD::kClose_AltF4:
	{
		for (HWND h : data.found)
		{
			if (AppActivate(h))
			{
				CSendKeys sk;
				sk.SendKeys(L"{DELAY=50}%{F4}");
			}
			::Sleep(300);
		}
	}
	break;
	}
	// ClipboardHistory.exe
	return 0;
}


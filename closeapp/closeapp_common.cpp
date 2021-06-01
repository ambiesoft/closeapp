// closeapp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"

#include <functional>
#include <set>

#include "../../lsMisc/GetFileNameFromHwnd.h"
#include "../../lsMisc/CommandLineParser.h"
#include "../../lsMisc/CSendKeys/SendKeys.h"
#include "../../lsMisc/OpenCommon.h"
#include "../../lsMisc/stdosd/stdosd.h"

#include <iostream>

#define I18N(s) (s)

using namespace std;
using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;

std::function<void(wstring)> goutfunc;
struct Data {
	bool verbose = false;
	vector<wstring> executables;
	vector<HWND> found;
	set<wstring> foundExecutable;
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
		for (auto&& name : pData->executables)
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
						goutfunc(stdFormat(I18N(L"Found window 0x%llx in '%s'"), (ULONGLONG)h, name.c_str()));
					pData->found.emplace_back(h);
					pData->foundExecutable.insert(szT);
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

void RemoveRootExplorer(Data& data)
{
	set<HWND> hwndsToRemove;
	for (HWND h : data.found)
	{
		wchar_t szT[MAX_PATH] = { 0 };
		GetClassName(h, szT, _countof(szT));
		// if (lstrcmp(szT, L"Shell_TrayWnd") == 0)
		if (lstrcmp(szT, L"CabinetWClass") != 0)
		{
			hwndsToRemove.insert(h);
		}
	}

	for (HWND hToRemove : hwndsToRemove)
	{
		data.found.erase(remove_if(data.found.begin(), data.found.end(), [&](HWND h) {
			return h == hToRemove;
		}));
	}
}

int wmain_common(
	function<void(wstring)> outfunc,
	function<void(wstring)> errorfunc,
	function<set<wstring>()> getinputfunc)
{
	CCommandLineParser parser;

	wstring closemethod;
	parser.AddOption(L"-m", 1, &closemethod, ArgEncodingFlags::ArgEncodingFlags_Default,
		I18N(L"Close method, one of 'wm_close', 'sc_close', 'alt-f4'"));

	COption mainArgs(L"", ArgCount::ArgCount_Infinite, ArgEncodingFlags_Default, L"Target executables");
	parser.AddOption(&mainArgs);

	bool bRestart = false;
	parser.AddOption(L"-r", 0, &bRestart, ArgEncodingFlags_Default, I18N(L"Restart application"));

	bool bHelp = false;
	parser.AddOptionRange({ L"-h", L"/?" }, 0, &bHelp, ArgEncodingFlags_Default, I18N(L"Show help"));

	bool bVerbose = false;
	parser.AddOption(L"-v", 0, &bVerbose, ArgEncodingFlags_Default, I18N(L"Show verbose output"));
	
	bool bCloseExplorerWindows = false;
	parser.AddOption(L"-ce", 0, &bCloseExplorerWindows, ArgEncodingFlags_Default, I18N(L"Close Explorer Windows"));

	parser.Parse();

	if (bHelp)
	{
		// TODO: Add version number
		outfunc(parser.getHelpMessage());
		return 0;
	}

	Data data;
	data.verbose = bVerbose;

	if (bCloseExplorerWindows)
	{
		if (mainArgs.getValueCount() != 0)
		{
			errorfunc(I18N(L"-ce must be specified exclusively"));
			return 1;
		}
		data.executables.emplace_back(L"explorer.exe");
	}
	else
	{
		for (size_t i = 0; i < mainArgs.getValueCount(); ++i)
		{
			data.executables.emplace_back(mainArgs.getValue(i));
		}
		if (data.executables.empty())
		{
			if (getinputfunc)
			{
				for (auto&& exe : getinputfunc())
				{
					data.executables.emplace_back(exe);
				}
			}
		}
	}

	if (data.executables.empty())
	{
		errorfunc(I18N(L"No input file"));
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
		errorfunc(I18N(L"Unknown close method:") + closemethod);
		return 1;
	}

	EnumWindows(enumproc, (LPARAM)&data);

	if (data.found.empty())
	{
		errorfunc(I18N(L"Windows not found"));
		return 1;
	}

	if (bCloseExplorerWindows)
	{
		RemoveRootExplorer(data);
	}

	for (HWND h : data.found) 
	{
		switch (cm)
		{
		case CLOSE_METHD::kClose_WM_CLOSE:
			SendMessage(h, WM_CLOSE, 0, 0);
			break;
		case CLOSE_METHD::kClose_SC_CLOSE:
			SendMessage(h, WM_SYSCOMMAND, SC_CLOSE, 0);
			break;
		case CLOSE_METHD::kClose_AltF4:
			if (AppActivate(h))
			{
				CSendKeys sk;
				sk.SendKeys(L"{DELAY=50}%{F4}");
			}
			::Sleep(300);
			break;
		}
	}

	if (bRestart)
	{
		// wait for all windows gone
		bool done = false;
		ULONG maxwait = 60 * 1000;
		ULONGLONG startTick = GetTickCount64();
		do
		{
			[&]()
			{
				for (HWND h : data.found)
				{
					if (IsWindow(h))
					{
						::Sleep(300);
						return;
					}
				}
				done = true;
			}();

			if ((GetTickCount64() - startTick) > maxwait)
				break;
		} while (!done);

		if (!done)
		{
			// wait timeout
			errorfunc(I18N(L"Wait timeout, cancelled to restart."));
			return 2;
		}

		vector<wstring> failedLaunches;
		for (auto&& exe : data.foundExecutable)
		{
			if (!OpenCommon(NULL, exe.c_str()))
			{
				failedLaunches.emplace_back(exe);
			}
		}

		if (!failedLaunches.empty())
		{
			wstring errorMessage = I18N(L"Failed to launch application.");
			errorMessage += L"\n\n";
			for (auto&& exe : failedLaunches)
			{
				errorMessage += exe;
				errorMessage += L"\n";
			}
			errorfunc(errorMessage);
			return 2;
		}
	}
	return 0;
}


// closeapp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include "../../lsMisc/GetFileNameFromHwnd.h"

#include <iostream>

#define I18N(s) (s)

using namespace std;
using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;

struct Data {
	wstring name;
	vector<HWND> found;
};
BOOL CALLBACK enumproc(HWND h, LPARAM lp)
{
	Data* pData = (Data*)lp;
	wchar_t szT[MAX_PATH];
	if (GetFileNameFromHwnd(h, szT, _countof(szT)))
	{
		wstring szFile = szT;
		if (stdIsFullPath(pData->name))
		{
			szFile = stdGetFullPathName(szT);
		}
		else
		{
			szFile = stdGetFileName(szT);
		}

		if (lstrcmpi(szFile.c_str(), pData->name.c_str()) == 0)
		{
			pData->found.emplace_back(h);
		}
	}
	return TRUE;
}
int wmain(int argc, const wchar_t* argv[])
{
	if (argc < 2)
	{
		wcerr << I18N(L"No input file") << endl;
		return 1;
	}
	Data data;
	data.name = argv[1];
	EnumWindows(enumproc, (LPARAM)&data);
	for (HWND h : data.found) {
		SendMessage(h, WM_CLOSE, 0, 0);
	}
	// ClipboardHistory.exe
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

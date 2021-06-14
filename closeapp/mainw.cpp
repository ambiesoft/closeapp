#include <windows.h>
#include <set>
#include <cassert>
#include "../../lsMisc/CenterWindow.h"
#include "../../lsMisc/stdosd/stdosd.h"
#include "../../lsMisc/I18N.h"
#include "../../lsMisc/UTF16toUTF8.h"
#include "../../profile/cpp/Profile/include/ambiesoft.profile.h"
#include "../../lsMisc/HighDPI.h"

#include "resource.h"

#include "closeapp_common.h"

constexpr wchar_t APPNAME[] = L"closeappw";
HINSTANCE ghInst;

using namespace std;
using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;

void outfunc(wstring str)
{
	MessageBox(NULL, str.c_str(), APPNAME, MB_ICONINFORMATION);
}
void errorfunc(wstring str)
{
	MessageBox(NULL, str.c_str(), APPNAME, MB_ICONWARNING);
}

void AppendExecutable(HWND hCombo, const wstring& fullpath)
{
	wstring file = stdGetFileName(fullpath);

	wstring newString;
	stdGetWindowText(hCombo, &newString);
	if (!newString.empty())
		newString += L";";
	newString += file;
	stdSetWindowText(hCombo, newString);
}

wchar_t buff[512];
INT_PTR CALLBACK DialgGetInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	constexpr char SECTION_SETTING[] = "Settings";
	constexpr char KEY_RECENT_EXES[] = "RecentExes";

	thread_local static set<wstring>* spRet = nullptr;
	thread_local static HWND shCombo = nullptr;
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			spRet = reinterpret_cast<set<wstring>*>(lParam);
			shCombo = GetDlgItem(hWnd, IDC_COMBO_EXECUTABLES);
			assert(shCombo);
			SetWindowText(hWnd, APPNAME);

			// Load ini
			const wstring inifile = stdCombinePath(
				stdGetParentDirectory(stdGetModuleFileName()),
				stdGetFileNameWitoutExtension(stdGetModuleFileName()) + L".ini");
			try
			{
				Profile::CHashIni ini(Profile::ReadAll(inifile, true));
				vector<string> vs;
				Profile::GetStringArray(
					SECTION_SETTING,
					KEY_RECENT_EXES,
					vs,
					ini);
				vector<wstring> wvs = toStdWstringFromUtf8(vs);
				for(auto&& ws : wvs)
					SendMessage(shCombo, CB_ADDSTRING, 0, (LPARAM)ws.c_str());
				if (!wvs.empty())
					SetWindowText(shCombo, wvs[0].c_str());
			}
			catch(file_not_found_error&){}
			catch (std::exception& ex)
			{
				MessageBox(hWnd,
					(wstring() + I18N(L"Failed to load ini.") + L"\r\n\r\n" + toStdWstringFromACP(ex.what())).c_str(),
					APPNAME,
					MB_ICONERROR);
				ExitProcess(1);
			}

			CenterWindow(hWnd);
		}
		break;
	case WM_COMMAND:
		switch (wParam)
		{
			case IDC_BUTTON_BROWSEAPP:
			{
				CGetOpenFileFilter filter;
				filter.AddFilter(L"fillll", L"*.aaa", true);
				wstring fullpath;
				if (GetOpenFile(ghInst, hWnd,
					filter,// GETFILEFILTER::APP,
					nullptr,
					I18N(L"Choose Executable"),
					&fullpath) && !fullpath.empty())
				{
					AppendExecutable(shCombo, fullpath);
				}
			}
			break;
			case IDOK:
			{
				// save ini
				int itemCount = SendMessage(shCombo, CB_GETCOUNT, 0, 0);
				vector<wstring> items;

				// first set current
				GetWindowText(shCombo, buff, _countof(buff));
				spRet->insert(buff);
				items.emplace_back(buff);

				for (int i = 0; i < itemCount; ++i)
				{
					int len = SendMessage(shCombo, CB_GETLBTEXTLEN, i, 0);
					if (len == CB_ERR)
					{
						MessageBox(hWnd,
							I18N(L"Failed to get combobox item textlen"),
							APPNAME,
							MB_ICONERROR);
						ExitProcess(1);
					}
					if (len == 0)
						continue;
					vector<wchar_t> vbuff(len + 1);
					SendMessage(shCombo, CB_GETLBTEXT, i, (LPARAM)vbuff.data());
					items.push_back(vbuff.data());
				}
				
				// unique vector
				items = stdUniqueVector(items);

				const wstring inifile = stdCombinePath(
					stdGetParentDirectory(stdGetModuleFileName()),
					stdGetFileNameWitoutExtension(stdGetModuleFileName()) + L".ini");

				Profile::CHashIni ini(Profile::ReadAll(inifile));
				Profile::WriteStringArray(
					SECTION_SETTING,
					KEY_RECENT_EXES,
					toStdUtf8String(items),
					ini);
				if (!Profile::WriteAll(ini, inifile))
				{
					MessageBox(hWnd,
						I18N(L"Failed to get combobox item textlen"),
						APPNAME,
						MB_ICONERROR);
				}
			}
			// fall through
			case IDCANCEL:
				EndDialog(hWnd, wParam);
				break;
		}
	}
	return 0;
}
set<wstring> getinput()
{
	set<wstring> ret;
	INT_PTR dialogRet = DialogBoxParam(ghInst,
		MAKEINTRESOURCE(IDD_DIALOG_INPUTEXECUTABLE),
		nullptr,
		DialgGetInput,
		(LPARAM)&ret);
	if (dialogRet != IDOK)
		ExitProcess(1);
	
	return ret;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	
	InitHighDPISupport();
	ghInst = hInstance;
	return wmain_common(APPNAME, outfunc, errorfunc, getinput);
}

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

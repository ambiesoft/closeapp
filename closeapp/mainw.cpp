#include <windows.h>

#include "closeapp_common.h"



using namespace std;

void outfunc(wstring str)
{
	MessageBox(NULL, str.c_str(), APPNAME, MB_ICONINFORMATION);
}
void errorfunc(wstring str)
{
	MessageBox(NULL, str.c_str(), APPNAME, MB_ICONWARNING);
}



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	return wmain_common(outfunc, errorfunc);
}
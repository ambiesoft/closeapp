#include "stdafx.h"
#include <iostream>
#include "closeapp_common.h"

using namespace std;

void outfunc(wstring str)
{
	wcout << str << endl;
}
void errorfunc(wstring str)
{
	wcerr << str << endl;
}
int wmain(int argc, const wchar_t* argv[])
{
	return wmain_common(L"closeapp", outfunc, errorfunc);
}
#pragma once
#include <windows.h>
#include <string>
using std::wstring;

INT_PTR CALLBACK PathPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern std::wstring g_savePath;

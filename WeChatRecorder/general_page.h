#pragma once
#include <windows.h>

INT_PTR CALLBACK GeneralPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// Global settings variables
extern int g_micVolumePercent;
extern int g_speakerVolumePercent;
extern bool g_startupEnabled;
extern bool g_minimizeToTray;

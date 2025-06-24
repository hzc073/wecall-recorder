/*
 * =====================================================================================
 *
 * Filename:  blacklist_page.h
 *
 * Description:  Header for the blacklist settings page.
 *
 * =====================================================================================
 */
#pragma once
#include <windows.h>
#include <vector>
#include <string>

 // Global extern declaration for the blacklist vector
 // g_blacklist 用于存储用户添加的、不希望触发录音的应用程序进程名 (例如 "chrome.exe")
extern std::vector<std::wstring> g_blacklist;

// Dialog procedure for the blacklist page
// BlacklistPageProc 是黑名单设置页面的对话框过程函数，负责处理该页面的所有消息和事件
INT_PTR CALLBACK BlacklistPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

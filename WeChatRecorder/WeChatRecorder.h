#pragma once

#include "resource.h"

// 添加对话框过程的前置声明
INT_PTR CALLBACK DevicePageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PathPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// --- 修复点：移除了此处重复的 GetSelected...Id 函数定义 ---
// --- 这些函数已在 device_page.h 中统一定义 ---
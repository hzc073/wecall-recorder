// custom_button.h
#pragma once
#include <windows.h>

// 创建一个自定义按钮（mac风格）
void CreateMacStyleButton(HWND hWnd, int id, LPCWSTR text, int x, int y, int w, int h);

// 在 WM_DRAWITEM 中绘制该按钮
bool HandleMacStyleButtonDraw(LPDRAWITEMSTRUCT pDIS, LPCWSTR text);
bool HandleMacStyleButtonDrawAuto(LPDRAWITEMSTRUCT pDIS, int id);

// custom_button.h
#pragma once
#include <windows.h>

// ����һ���Զ��尴ť��mac���
void CreateMacStyleButton(HWND hWnd, int id, LPCWSTR text, int x, int y, int w, int h);

// �� WM_DRAWITEM �л��Ƹð�ť
bool HandleMacStyleButtonDraw(LPDRAWITEMSTRUCT pDIS, LPCWSTR text);
bool HandleMacStyleButtonDrawAuto(LPDRAWITEMSTRUCT pDIS, int id);

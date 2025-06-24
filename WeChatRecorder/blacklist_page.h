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
 // g_blacklist ���ڴ洢�û���ӵġ���ϣ������¼����Ӧ�ó�������� (���� "chrome.exe")
extern std::vector<std::wstring> g_blacklist;

// Dialog procedure for the blacklist page
// BlacklistPageProc �Ǻ���������ҳ��ĶԻ�����̺������������ҳ���������Ϣ���¼�
INT_PTR CALLBACK BlacklistPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#pragma once

#include "resource.h"

// ��ӶԻ�����̵�ǰ������
INT_PTR CALLBACK DevicePageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PathPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// --- �޸��㣺�Ƴ��˴˴��ظ��� GetSelected...Id �������� ---
// --- ��Щ�������� device_page.h ��ͳһ���� ---
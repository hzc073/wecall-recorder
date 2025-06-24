// �ļ�����device_page.h
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "ffmpeg_device_enum.h"
#include "resource.h"

// �����Ի������
INT_PTR CALLBACK DevicePageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// --- ����������һ�������ڳ�������ʱ��ʼ��Ĭ���豸�ĺ��� ---
void InitializeDefaultDevices();

// ȫ�ֱ����� extern ����
extern std::vector<FFmpegDeviceInfo> g_inputDevices;
extern std::vector<FFmpegDeviceInfo> g_outputDevices;
extern std::wstring g_selectedInputDeviceId;
extern std::wstring g_selectedOutputDeviceId;

// ��ȡ�豸ID����������
inline std::wstring GetSelectedInputDeviceId() {
    return g_selectedInputDeviceId;
}
inline std::wstring GetSelectedOutputDeviceId() {
    return g_selectedOutputDeviceId;
}
// 文件名：device_page.h
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "ffmpeg_device_enum.h"
#include "resource.h"

// 声明对话框过程
INT_PTR CALLBACK DevicePageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// --- 新增：声明一个用于在程序启动时初始化默认设备的函数 ---
void InitializeDefaultDevices();

// 全局变量的 extern 声明
extern std::vector<FFmpegDeviceInfo> g_inputDevices;
extern std::vector<FFmpegDeviceInfo> g_outputDevices;
extern std::wstring g_selectedInputDeviceId;
extern std::wstring g_selectedOutputDeviceId;

// 获取设备ID的内联函数
inline std::wstring GetSelectedInputDeviceId() {
    return g_selectedInputDeviceId;
}
inline std::wstring GetSelectedOutputDeviceId() {
    return g_selectedOutputDeviceId;
}
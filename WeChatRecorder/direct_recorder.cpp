/*
#include "direct_recorder.h"
#include "log.h"
#include "resource.h"
#include "device_enum.h"
#include <windows.h>
#include <string>



FFmpegRecorder::FFmpegRecorder() = default;

FFmpegRecorder::~FFmpegRecorder() {
    Stop();
}
extern HWND hDevicePage;
bool FFmpegRecorder::Start(const std::wstring& filename) {
    if (m_recording) return false;

    // 获取用户选择的输入设备
    HWND hComboIn = GetDlgItem(hDevicePage, IDC_COMBO_INPUT_DEVICE);
    int sel = (int)SendMessageW(hComboIn, CB_GETCURSEL, 0, 0);
    wchar_t deviceName[MAX_PATH];
    SendMessageW(hComboIn, CB_GETLBTEXT, sel, (LPARAM)deviceName);

    // 构建 FFmpeg 命令
    std::wstring cmd = L"ffmpeg -f dshow -i audio=\"" +
        std::wstring(deviceName) +
        L"\" -acodec libmp3lame -b:a 192k \"" +
        filename + L"\"";

    WriteLog(L"[FFmpegRecorder] 启动录制: %s", cmd.c_str());

    // 创建进程参数
    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;  // 隐藏控制台窗口

    // 创建 FFmpeg 进程
    if (!CreateProcessW(
        NULL,
        (LPWSTR)cmd.c_str(),
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &m_processInfo
    )) {
        WriteLog(L"[FFmpegRecorder] 创建进程失败: %d", GetLastError());
        return false;
    }

    m_recording = true;
    m_currentFilename = filename;
    return true;
}

void FFmpegRecorder::Stop() {
    if (!m_recording) return;

    WriteLog(L"[FFmpegRecorder] 停止录制");
    TerminateFFmpegProcess();
    m_recording = false;
}

bool FFmpegRecorder::IsRecording() const {
    return m_recording;
}

void FFmpegRecorder::TerminateFFmpegProcess() {
    // 终止主进程
    if (m_processInfo.hProcess) {
        TerminateProcess(m_processInfo.hProcess, 0);
        WaitForSingleObject(m_processInfo.hProcess, 5000);
        CloseHandle(m_processInfo.hProcess);
        CloseHandle(m_processInfo.hThread);
        ZeroMemory(&m_processInfo, sizeof(m_processInfo));
    }
}
*/
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

    // ��ȡ�û�ѡ��������豸
    HWND hComboIn = GetDlgItem(hDevicePage, IDC_COMBO_INPUT_DEVICE);
    int sel = (int)SendMessageW(hComboIn, CB_GETCURSEL, 0, 0);
    wchar_t deviceName[MAX_PATH];
    SendMessageW(hComboIn, CB_GETLBTEXT, sel, (LPARAM)deviceName);

    // ���� FFmpeg ����
    std::wstring cmd = L"ffmpeg -f dshow -i audio=\"" +
        std::wstring(deviceName) +
        L"\" -acodec libmp3lame -b:a 192k \"" +
        filename + L"\"";

    WriteLog(L"[FFmpegRecorder] ����¼��: %s", cmd.c_str());

    // �������̲���
    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;  // ���ؿ���̨����

    // ���� FFmpeg ����
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
        WriteLog(L"[FFmpegRecorder] ��������ʧ��: %d", GetLastError());
        return false;
    }

    m_recording = true;
    m_currentFilename = filename;
    return true;
}

void FFmpegRecorder::Stop() {
    if (!m_recording) return;

    WriteLog(L"[FFmpegRecorder] ֹͣ¼��");
    TerminateFFmpegProcess();
    m_recording = false;
}

bool FFmpegRecorder::IsRecording() const {
    return m_recording;
}

void FFmpegRecorder::TerminateFFmpegProcess() {
    // ��ֹ������
    if (m_processInfo.hProcess) {
        TerminateProcess(m_processInfo.hProcess, 0);
        WaitForSingleObject(m_processInfo.hProcess, 5000);
        CloseHandle(m_processInfo.hProcess);
        CloseHandle(m_processInfo.hThread);
        ZeroMemory(&m_processInfo, sizeof(m_processInfo));
    }
}
*/
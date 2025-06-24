// FFmpegRecorder.h
#pragma once
#include <string>
#include <windows.h>
#include "log.h"

class FFmpegRecorder {
public:
    FFmpegRecorder();
    ~FFmpegRecorder();

    bool Start(const std::wstring& filename,
        const std::wstring& inputDeviceName,
        const std::wstring& outputDeviceId);
    void Stop();
    bool IsRecording() const;

private:
    void TerminateFFmpegProcess();

    // --- ���������ڶ�ȡFFmpeg��־���̺߳��� ---
    static DWORD WINAPI LogReaderThreadFunc(LPVOID lpParam);
    void ReadAndLogPipe(); // �߳�ʵ��ִ�еĳ�Ա����

    PROCESS_INFORMATION m_processInfo;
    std::wstring m_currentFilename;
    bool m_recording = false;

    // --- ���������ڹܵ�����־�̵߳ľ�� ---
    HANDLE hLogThread = NULL;
    HANDLE hPipeRead = NULL;
};
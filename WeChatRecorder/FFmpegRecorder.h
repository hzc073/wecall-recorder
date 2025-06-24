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

    // --- 新增：用于读取FFmpeg日志的线程函数 ---
    static DWORD WINAPI LogReaderThreadFunc(LPVOID lpParam);
    void ReadAndLogPipe(); // 线程实际执行的成员函数

    PROCESS_INFORMATION m_processInfo;
    std::wstring m_currentFilename;
    bool m_recording = false;

    // --- 新增：用于管道和日志线程的句柄 ---
    HANDLE hLogThread = NULL;
    HANDLE hPipeRead = NULL;
};
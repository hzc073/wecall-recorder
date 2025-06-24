#include "FFmpegRecorder.h"
#include "string_util.h"
#include <thread>
FFmpegRecorder::FFmpegRecorder() : m_recording(false) {
    ZeroMemory(&m_processInfo, sizeof(m_processInfo));
}

FFmpegRecorder::~FFmpegRecorder() {
    Stop();
}

bool FFmpegRecorder::Start(const std::wstring& filename,
    const std::wstring& inputDeviceName,
    const std::wstring& outputDeviceId) {
    if (m_recording) return false;

    std::wstring cmd = L"ffmpeg -y "
        L"-f dshow -i audio=\"" + inputDeviceName + L"\" "
        L"-f wasapi -i \"id:" + outputDeviceId + L"\" "
        L"-filter_complex \"[0:a][1:a]amix=inputs=2:duration=longest\" "
        L"-acodec libmp3lame -b:a 192k \"" + filename + L"\"";

    WriteLog(L"[FFmpegRecorder] 准备执行以下最终命令:");
    WriteLog(L"%s", cmd.c_str());

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hPipeWrite = NULL;
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0)) {
        WriteLog(L"[FFmpegRecorder] CreatePipe 失败: %d", GetLastError());
        return false;
    }
    SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = NULL;
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;
    si.wShowWindow = SW_HIDE;

    if (!CreateProcessW(
        NULL,
        const_cast<LPWSTR>(cmd.c_str()),
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &m_processInfo
    )) {
        WriteLog(L"[FFmpegRecorder] 创建进程失败: %d", GetLastError());
        CloseHandle(hPipeRead);
        CloseHandle(hPipeWrite);
        return false;
    }

    CloseHandle(hPipeWrite);

    hLogThread = CreateThread(NULL, 0, LogReaderThreadFunc, this, 0, NULL);
    if (hLogThread == NULL) {
        WriteLog(L"[FFmpegRecorder] 创建日志读取线程失败: %d", GetLastError());
        TerminateFFmpegProcess();
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
    if (m_processInfo.hProcess) {
        TerminateProcess(m_processInfo.hProcess, 0);
        WaitForSingleObject(m_processInfo.hProcess, 5000);
        CloseHandle(m_processInfo.hProcess);
        CloseHandle(m_processInfo.hThread);
        ZeroMemory(&m_processInfo, sizeof(m_processInfo));
    }

    if (hLogThread) {
        WaitForSingleObject(hLogThread, 2000);
        CloseHandle(hLogThread);
        hLogThread = NULL;
    }

    if (hPipeRead) {
        CloseHandle(hPipeRead);
        hPipeRead = NULL;
    }
}

DWORD WINAPI FFmpegRecorder::LogReaderThreadFunc(LPVOID lpParam) {
    FFmpegRecorder* pThis = static_cast<FFmpegRecorder*>(lpParam);
    if (pThis) {
        pThis->ReadAndLogPipe();
    }
    return 0;
}

void FFmpegRecorder::ReadAndLogPipe() {
    char buffer[1024];
    DWORD bytesRead;
    while (ReadFile(hPipeRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        WriteLog(L"[ffmpeg] %s", MultiByteToWide(buffer).c_str());
    }
}

// --- 核心修复点：删除了整个未使用的 RunFFmpegAndLog 函数 ---
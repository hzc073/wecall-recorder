#pragma once
#include <string>
#include <windows.h>

// 只声明一次 m_processInfo，避免重复定义
class Recorder {
public:
    virtual ~Recorder() {}
    virtual bool Start(const std::wstring& filename) = 0;
    virtual void Stop() = 0;
    virtual bool IsRecording() const = 0;
};

class FFmpegRecorder : public Recorder {
public:
    FFmpegRecorder();
    ~FFmpegRecorder();

    bool Start(const std::wstring& filename) override;
    void Stop() override;
    bool IsRecording() const override;

private:
    void TerminateFFmpegProcess();

    PROCESS_INFORMATION m_processInfo = { 0 };  // 进程信息
    std::wstring m_currentFilename;
    bool m_recording = false;
};

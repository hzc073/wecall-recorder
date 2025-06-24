#pragma once
#include <string>
#include <windows.h>

// ֻ����һ�� m_processInfo�������ظ�����
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

    PROCESS_INFORMATION m_processInfo = { 0 };  // ������Ϣ
    std::wstring m_currentFilename;
    bool m_recording = false;
};

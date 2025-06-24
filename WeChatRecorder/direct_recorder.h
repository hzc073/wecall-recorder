/*
#pragma once
#include "recorder.h"
#include <string>
#include <windows.h>
#include <tlhelp32.h>

class DirectRecorder : public Recorder {  
public:
    DirectRecorder();
    ~DirectRecorder();

    bool Start(const std::wstring& filename) override;
    void Stop() override;
    bool IsRecording() const override;

private:
   
    void TerminateFFmpegProcess();

    PROCESS_INFORMATION m_processInfo = { 0 };
    std::wstring m_currentFilename;
    bool m_recording = false;
};
*/
#pragma once

#include <string>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <fstream>
#include <thread>
#include <atomic>
#include "recorder.h"
class WasapiRecorder : public Recorder {

public:
    WasapiRecorder();
    ~WasapiRecorder();

    // 开始录音，保存到filename（wav）
    bool Start(const std::wstring& filename);
    void Stop();
    bool IsRecording() const;

private:
    std::atomic<bool> recording;
    std::thread recordThread;

    std::wstring outputFilename;

    void RecordThreadProc();
    void WriteWavHeader(std::ofstream& outFile, WAVEFORMATEX* pwfx, UINT32 dataLength);
    void FixWavHeader(std::ofstream& outFile, UINT32 dataLength);
};

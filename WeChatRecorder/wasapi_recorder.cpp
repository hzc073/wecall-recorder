#include "wasapi_recorder.h"
#include <Audioclient.h>
#include <Mmdeviceapi.h>
#include <comdef.h>
#include <fstream>
#include <vector>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")

WasapiRecorder::WasapiRecorder() : recording(false) {}
WasapiRecorder::~WasapiRecorder() { Stop(); }

bool WasapiRecorder::Start(const std::wstring& filename) {
    if (recording) return false;
    outputFilename = filename;
    recording = true;
    recordThread = std::thread(&WasapiRecorder::RecordThreadProc, this);
    return true;
}

void WasapiRecorder::Stop() {
    if (recording) {
        recording = false;
        if (recordThread.joinable()) {
            recordThread.join();
        }
    }
}

bool WasapiRecorder::IsRecording() const {
    return recording;
}

// 录音线程主逻辑
void WasapiRecorder::RecordThreadProc() {
    HRESULT hr;
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioCaptureClient* pCaptureClient = nullptr;
    WAVEFORMATEX* pwfx = nullptr;
    std::ofstream outFile;

    UINT32 frameSize = 0;
    UINT32 totalDataLen = 0;

    do {
        // 获取默认音频输入设备（麦克风）
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
            __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
        if (FAILED(hr)) break;
        hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
        if (FAILED(hr)) break;
        hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
        if (FAILED(hr)) break;
        hr = pAudioClient->GetMixFormat(&pwfx);
        if (FAILED(hr)) break;

        // 设置录音格式
        hr = pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            0,
            10000000, // 1秒缓冲
            0,
            pwfx,
            NULL);
        if (FAILED(hr)) break;

        hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
        if (FAILED(hr)) break;

        // 打开wav文件，写入头部（占位）
        outFile.open(outputFilename, std::ios::binary);
        WriteWavHeader(outFile, pwfx, 0);

        hr = pAudioClient->Start();
        if (FAILED(hr)) break;

        BYTE* pData;
        UINT32 numFramesAvailable;
        DWORD flags;
        while (recording) {
            Sleep(50); // 录音“采样”间隔
            hr = pCaptureClient->GetNextPacketSize(&numFramesAvailable);
            if (FAILED(hr)) break;
            while (numFramesAvailable > 0) {
                hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
                if (FAILED(hr)) break;
                int bytes = numFramesAvailable * pwfx->nBlockAlign;
                outFile.write((const char*)pData, bytes);
                totalDataLen += bytes;
                pCaptureClient->ReleaseBuffer(numFramesAvailable);
                hr = pCaptureClient->GetNextPacketSize(&numFramesAvailable);
                if (FAILED(hr)) break;
            }
        }

        pAudioClient->Stop();

        // 修正wav头部（写入真正数据长度）
        FixWavHeader(outFile, totalDataLen);
        outFile.close();
    } while (false);

    if (pwfx) CoTaskMemFree(pwfx);
    if (pCaptureClient) pCaptureClient->Release();
    if (pAudioClient) pAudioClient->Release();
    if (pDevice) pDevice->Release();
    if (pEnumerator) pEnumerator->Release();

    CoUninitialize();
}

// 写wav头部
void WasapiRecorder::WriteWavHeader(std::ofstream& outFile, WAVEFORMATEX* pwfx, UINT32 dataLength) {
    outFile.write("RIFF", 4);
    UINT32 chunkSize = 36 + dataLength;
    outFile.write(reinterpret_cast<const char*>(&chunkSize), 4);
    outFile.write("WAVE", 4);

    outFile.write("fmt ", 4);
    UINT32 subchunk1Size = 16;
    outFile.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
    outFile.write(reinterpret_cast<const char*>(pwfx), 16);

    outFile.write("data", 4);
    outFile.write(reinterpret_cast<const char*>(&dataLength), 4);
}

// 修正wav头部
void WasapiRecorder::FixWavHeader(std::ofstream& outFile, UINT32 dataLength) {
    outFile.seekp(4, std::ios::beg);
    UINT32 chunkSize = 36 + dataLength;
    outFile.write(reinterpret_cast<const char*>(&chunkSize), 4);

    outFile.seekp(40, std::ios::beg);
    outFile.write(reinterpret_cast<const char*>(&dataLength), 4);
}

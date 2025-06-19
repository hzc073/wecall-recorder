#include "mic_level_sampler.h"
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <vector>
#include <algorithm>
#include <cmath>

MicLevelSampler::MicLevelSampler() : running(false), level(0.0f) {}
MicLevelSampler::~MicLevelSampler() { Stop(); }

void MicLevelSampler::Start() {
    if (running) return;
    running = true;
    worker = std::thread(&MicLevelSampler::SampleLoop, this);
}

void MicLevelSampler::Stop() {
    running = false;
    if (worker.joinable()) worker.join();
}

float MicLevelSampler::GetLevel() const {
    return level;
}

// 实际采集线程
void MicLevelSampler::SampleLoop() {
    HRESULT hr = S_OK;
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    IMMDeviceEnumerator* pEnum = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioCaptureClient* pCaptureClient = nullptr;
    WAVEFORMATEX* pwfx = nullptr;

    do {
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pEnum));
        if (FAILED(hr)) break;
        hr = pEnum->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
        if (FAILED(hr)) break;
        hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
        if (FAILED(hr)) break;
        hr = pAudioClient->GetMixFormat(&pwfx);
        if (FAILED(hr)) break;

        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, pwfx, NULL);
        if (FAILED(hr)) break;

        hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
        if (FAILED(hr)) break;
        hr = pAudioClient->Start();
        if (FAILED(hr)) break;

        while (running) {
            Sleep(50);

            UINT32 nFrames = 0;
            hr = pCaptureClient->GetNextPacketSize(&nFrames);
            if (FAILED(hr)) break;
            float rms = 0.0f;
            UINT32 samples = 0;

            while (nFrames > 0) {
                BYTE* pData = nullptr;
                DWORD flags = 0;
                hr = pCaptureClient->GetBuffer(&pData, &nFrames, &flags, NULL, NULL);
                if (FAILED(hr)) break;
                // 假定16位单声道/立体声
                short* pcm = (short*)pData;
                int count = nFrames * pwfx->nChannels;
                float sum = 0.0f;
                for (int i = 0; i < count; ++i) {
                    float v = pcm[i] / 32768.0f;
                    sum += v * v;
                }
                if (count > 0) {
                    rms = sqrt(sum / count);
                    samples += count;
                }
                pCaptureClient->ReleaseBuffer(nFrames);
                hr = pCaptureClient->GetNextPacketSize(&nFrames);
                if (FAILED(hr)) break;
            }

            if (samples > 0) level = rms; // 典型范围0.0~0.1（人声大约0.01~0.05左右）
        }
        pAudioClient->Stop();
    } while (false);

    if (pwfx) CoTaskMemFree(pwfx);
    if (pCaptureClient) pCaptureClient->Release();
    if (pAudioClient) pAudioClient->Release();
    if (pDevice) pDevice->Release();
    if (pEnum) pEnum->Release();
    CoUninitialize();
}

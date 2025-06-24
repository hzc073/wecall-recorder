#include "mic_level_sampler.h"
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <chrono>
#include "log.h"
// 新增头文件，用于支持WAVE_FORMAT_EXTENSIBLE格式的判断
#include <ks.h>
#include <ksmedia.h>

#define DECAY_FACTOR 0.95f 

// 使用一个静态变量来记录启动时间，以防止因UI消息导致的意外快速停止
static std::chrono::steady_clock::time_point g_lastStartTime;

MicLevelSampler::MicLevelSampler() : running(false), level(0.0f) {}
MicLevelSampler::~MicLevelSampler() { Stop(); }

void MicLevelSampler::Start(const std::wstring& deviceId) {
    if (running) return;
    m_deviceId = deviceId;
    running = true;
    level = 0.0f;
    g_lastStartTime = std::chrono::steady_clock::now();
    worker = std::thread(&MicLevelSampler::SampleLoop, this);
}

void MicLevelSampler::Stop() {
    if (!running) {
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_lastStartTime).count();

    if (elapsed < 500) {
        WriteLog(L"[Sampler] Stop called too quickly after Start (< 500ms). Ignoring.");
        return;
    }

    running = false;
    if (worker.joinable()) {
        worker.join();
    }
}

float MicLevelSampler::GetLevel() const {
    return level;
}

void MicLevelSampler::SampleLoop() {
    WriteLog(L"[Sampler] SampleLoop thread started (Polling Mode).");
    HRESULT hr = S_OK;
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    IMMDeviceEnumerator* pEnum = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioCaptureClient* pCaptureClient = nullptr;
    WAVEFORMATEX* pwfx = nullptr;

    do {
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pEnum));
        if (FAILED(hr)) { WriteLog(L"[Sampler] CoCreateInstance failed. hr=0x%08X", hr); break; }

        if (m_deviceId.empty()) {
            hr = pEnum->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
        }
        else {
            hr = pEnum->GetDevice(m_deviceId.c_str(), &pDevice);
        }
        if (FAILED(hr)) { WriteLog(L"[Sampler] GetDevice failed. hr=0x%08X", hr); break; }

        hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
        if (FAILED(hr)) { WriteLog(L"[Sampler] Activate IAudioClient failed. hr=0x%08X", hr); break; }

        hr = pAudioClient->GetMixFormat(&pwfx);
        if (FAILED(hr)) { WriteLog(L"[Sampler] GetMixFormat failed. hr=0x%08X", hr); break; }

        WriteLog(L"[Sampler] Initializing with system-default buffer size...");
        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, pwfx, NULL);
        if (FAILED(hr)) {
            WriteLog(L"[Sampler] Initialize with default buffer failed. hr=0x%08X", hr);
            break;
        }

        hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
        if (FAILED(hr)) { WriteLog(L"[Sampler] GetService for IAudioCaptureClient failed. hr=0x%08X", hr); break; }

        hr = pAudioClient->Start();
        if (FAILED(hr)) { WriteLog(L"[Sampler] pAudioClient->Start() failed. hr=0x%08X", hr); break; }
        WriteLog(L"[Sampler] Audio client started. Entering polling sample loop...");

        while (running) {
            Sleep(20);

            UINT32 nFrames;
            hr = pCaptureClient->GetNextPacketSize(&nFrames);
            if (FAILED(hr)) { WriteLog(L"[Sampler] GetNextPacketSize failed in loop. hr=0x%08X", hr); break; }

            if (nFrames > 0) {
                BYTE* pData = nullptr;
                DWORD flags = 0;
                UINT32 numFramesRead = 0;

                hr = pCaptureClient->GetBuffer(&pData, &numFramesRead, &flags, NULL, NULL);
                if (FAILED(hr)) { WriteLog(L"[Sampler] GetBuffer failed. hr=0x%08X", hr); break; }

                if (numFramesRead > 0) {
                    float peak = 0.0f;

                    // --- 陷阱 1：处理 WAVE_FORMAT_EXTENSIBLE 格式 ---
                    // GetMixFormat 可能返回扩展格式 (0xFFFE)，其真正的格式信息存储在SubFormat中。
                    bool isFloat = (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ||
                        (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
                            IsEqualGUID(reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pwfx)->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT));

                    if (isFloat) {
                        // --- 陷阱 2：处理不同位深的浮点数 ---
                        // 虽然32位浮点最常见，但理论上可能存在其他位深。
                        // 为简化，这里我们只处理32位浮点。
                        if (pwfx->wBitsPerSample == 32) {
                            float* pSamples = (float*)pData;
                            for (UINT32 i = 0; i < numFramesRead * pwfx->nChannels; ++i) {
                                peak = max(peak, abs(pSamples[i]));
                            }
                        }
                    }
                    else if (pwfx->wFormatTag == WAVE_FORMAT_PCM && pwfx->wBitsPerSample == 16) {
                        short* pSamples = (short*)pData;
                        for (UINT32 i = 0; i < numFramesRead * pwfx->nChannels; ++i) {
                            peak = max(peak, abs(pSamples[i] / 32768.0f));
                        }
                    }
                    else {
                        // --- 陷阱 3：未知的音频格式 ---
                        // 如果遇到未处理的格式，也应记录日志，方便排查。
                        WriteLog(L"[Sampler] Warning: Unhandled audio format. wFormatTag=%u, wBitsPerSample=%u", pwfx->wFormatTag, pwfx->wBitsPerSample);
                    }

                    if (peak > level) {
                        level = peak;
                    }
                }

                hr = pCaptureClient->ReleaseBuffer(numFramesRead);
                if (FAILED(hr)) { WriteLog(L"[Sampler] ReleaseBuffer failed. hr=0x%08X", hr); break; }
            }

            level = level * DECAY_FACTOR;
        }

        if (pAudioClient) pAudioClient->Stop();
        WriteLog(L"[Sampler] Audio client stopped.");

    } while (false);

    if (pwfx) CoTaskMemFree(pwfx);
    if (pCaptureClient) pCaptureClient->Release();
    if (pAudioClient) pAudioClient->Release();
    if (pDevice) pDevice->Release();
    if (pEnum) pEnum->Release();
    CoUninitialize();

    level = 0.0f;
    WriteLog(L"[Sampler] SampleLoop thread finished cleanup and is exiting.");
}

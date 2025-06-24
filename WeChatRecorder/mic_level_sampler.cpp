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
// ����ͷ�ļ�������֧��WAVE_FORMAT_EXTENSIBLE��ʽ���ж�
#include <ks.h>
#include <ksmedia.h>

#define DECAY_FACTOR 0.95f 

// ʹ��һ����̬��������¼����ʱ�䣬�Է�ֹ��UI��Ϣ���µ��������ֹͣ
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

                    // --- ���� 1������ WAVE_FORMAT_EXTENSIBLE ��ʽ ---
                    // GetMixFormat ���ܷ�����չ��ʽ (0xFFFE)���������ĸ�ʽ��Ϣ�洢��SubFormat�С�
                    bool isFloat = (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ||
                        (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
                            IsEqualGUID(reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pwfx)->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT));

                    if (isFloat) {
                        // --- ���� 2������ͬλ��ĸ����� ---
                        // ��Ȼ32λ����������������Ͽ��ܴ�������λ�
                        // Ϊ�򻯣���������ֻ����32λ���㡣
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
                        // --- ���� 3��δ֪����Ƶ��ʽ ---
                        // �������δ����ĸ�ʽ��ҲӦ��¼��־�������Ų顣
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

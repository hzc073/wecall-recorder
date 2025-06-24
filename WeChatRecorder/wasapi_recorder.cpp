#include "wasapi_recorder.h"
#include "log.h"
#include <sstream>
#include <iomanip>
#include <comdef.h>
#include <vector>
#include <algorithm>
#include <ks.h>
#include <ksmedia.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")

#define SAFE_RELEASE(punk)  \
    if ((punk) != NULL) {   \
        (punk)->Release();  \
        (punk) = NULL;      \
    }

WasapiRecorder::WasapiRecorder() : m_isRecording(false), m_micVolumePercent(150), m_speakerVolumePercent(100) {}

WasapiRecorder::~WasapiRecorder() {
    Stop();
}

bool WasapiRecorder::IsRecording() const {
    return m_isRecording;
}

bool WasapiRecorder::Start(const std::wstring& filename, const std::wstring& inputDeviceId, const std::wstring& outputDeviceId, int micVolumePercent, int speakerVolumePercent) {
    if (m_isRecording) {
        WriteLog(L"[Recorder] 录音已在进行中，无法重复启动。");
        return false;
    }

    m_finalMp3Path = filename;
    m_inputDeviceId = inputDeviceId;
    m_outputDeviceId = outputDeviceId;
    m_micVolumePercent = micVolumePercent;
    m_speakerVolumePercent = speakerVolumePercent;

    WCHAR tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    m_micTempWavPath = std::wstring(tempPath) + L"mic_temp.wav";
    m_speakerTempWavPath = std::wstring(tempPath) + L"speaker_temp.wav";

    m_isRecording = true;

    try {
        m_micThread = std::thread(&WasapiRecorder::MicRecordThreadProc, this);
        m_speakerThread = std::thread(&WasapiRecorder::SpeakerRecordThreadProc, this);
    }
    catch (const std::system_error& e) {
        WriteLog(L"[Recorder] 启动线程失败: %hs", e.what());
        m_isRecording = false;
        return false;
    }

    WriteLog(L"[Recorder] 麦克风和扬声器录音任务已启动。");
    return true;
}

void WasapiRecorder::Stop() {
    if (!m_isRecording) return;

    m_isRecording = false;

    if (m_micThread.joinable()) {
        WriteLog(L"[Recorder] 等待麦克风录音线程结束...");
        m_micThread.join();
    }
    if (m_speakerThread.joinable()) {
        WriteLog(L"[Recorder] 等待扬声器录音线程结束...");
        m_speakerThread.join();
    }
    WriteLog(L"[Recorder] 所有采集线程已结束。");

    // 动态生成带有音量参数的命令
    double micVol = m_micVolumePercent / 100.0;
    double speakerVol = m_speakerVolumePercent / 100.0;

    std::wstringstream cmdStream;
    cmdStream << L"ffmpeg -y -i \"" << m_micTempWavPath << L"\" -i \"" << m_speakerTempWavPath
        << L"\" -filter_complex \"[0:a]volume=" << std::fixed << std::setprecision(2) << micVol
        << L"[a_mic];[1:a]volume=" << std::fixed << std::setprecision(2) << speakerVol
        << L"[a_spk];[a_mic][a_spk]amix=inputs=2:duration=longest\" -acodec libmp3lame -b:a 192k \""
        << m_finalMp3Path << L"\"";

    std::wstring cmd = cmdStream.str();

    WriteLog(L"[Recorder] 开始执行FFmpeg混音转码: %s", cmd.c_str());
    RunFFmpegAndLog(cmd);

    DeleteFileW(m_micTempWavPath.c_str());
    DeleteFileW(m_speakerTempWavPath.c_str());
    WriteLog(L"[Recorder] 临时文件已清理。");
}

void WasapiRecorder::MicRecordThreadProc() {
    WriteLog(L"[MicThread] 麦克风录音线程启动。");
    RecordLoop(true);
    WriteLog(L"[MicThread] 麦克风录音线程结束。");
}

void WasapiRecorder::SpeakerRecordThreadProc() {
    WriteLog(L"[SpeakerThread] 扬声器录音线程启动。");
    RecordLoop(false);
    WriteLog(L"[SpeakerThread] 扬声器录音线程结束。");
}

void WasapiRecorder::RecordLoop(bool isMic) {
    HRESULT hr;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dataSize = 0;

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioCaptureClient* pCaptureClient = nullptr;
    WAVEFORMATEX* pWfx = nullptr;
    HANDLE hSamplesReadyEvent = NULL;
    DWORD bytesWritten = 0;

    const DWORD waitTimeout = 100;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    do {
        std::wstring deviceId = isMic ? m_inputDeviceId : m_outputDeviceId;
        std::wstring tempFilePath = isMic ? m_micTempWavPath : m_speakerTempWavPath;
        EDataFlow dataFlow = isMic ? eCapture : eRender;
        DWORD streamFlags = isMic ? AUDCLNT_STREAMFLAGS_EVENTCALLBACK : (AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_LOOPBACK);
        const wchar_t* logPrefix = isMic ? L"[MicThread]" : L"[SpeakerThread]";

        hSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
        if (!hSamplesReadyEvent) break;

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator));
        if (FAILED(hr)) break;

        hr = pEnumerator->GetDevice(deviceId.c_str(), &pDevice);
        if (FAILED(hr)) break;

        hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
        if (FAILED(hr)) break;

        hr = pAudioClient->GetMixFormat(&pWfx);
        if (FAILED(hr)) break;

        WriteLog(L"%s 获取到设备原始音频格式 - 格式类型: %d, 位深: %d, 声道: %d, 采样率: %d",
            logPrefix, pWfx->wFormatTag, pWfx->wBitsPerSample, pWfx->nChannels, pWfx->nSamplesPerSec);

        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, streamFlags, 10000000, 0, pWfx, NULL);
        if (FAILED(hr)) break;

        hr = pAudioClient->SetEventHandle(hSamplesReadyEvent);
        if (FAILED(hr)) break;

        hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
        if (FAILED(hr)) break;

        hFile = CreateFileW(tempFilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) break;

        WORD bits_per_sample = 16;
        WORD channels = pWfx->nChannels;
        DWORD sample_rate = pWfx->nSamplesPerSec;
        WORD block_align = channels * (bits_per_sample / 8);
        DWORD byte_rate = sample_rate * block_align;
        DWORD header[] = {
            'FFIR', 36, 'EVAW', ' tmf', 16,
            (DWORD)((channels << 16) | 1),
            sample_rate, byte_rate,
            (DWORD)((bits_per_sample << 16) | block_align),
            'atad', 0
        };
        WriteFile(hFile, header, sizeof(header), &bytesWritten, NULL);

        hr = pAudioClient->Start();
        if (FAILED(hr)) break;

        while (m_isRecording) {
            DWORD waitResult = WaitForSingleObject(hSamplesReadyEvent, waitTimeout);
            if (!m_isRecording) break;

            UINT32 packetLength = 0;
            if (pCaptureClient) pCaptureClient->GetNextPacketSize(&packetLength);

            if (packetLength > 0) {
                BYTE* pData;
                UINT32 numFramesAvailable;
                DWORD flags;
                hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
                if (FAILED(hr)) break;

                if (numFramesAvailable > 0) {
                    if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                        DWORD silentBytes = numFramesAvailable * channels * 2;
                        std::vector<BYTE> silentBuffer(silentBytes, 0);
                        WriteFile(hFile, silentBuffer.data(), silentBytes, &bytesWritten, NULL);
                        dataSize += bytesWritten;
                    }
                    else {
                        std::vector<BYTE> convertedBuffer;
                        bool conversionSuccess = false;

                        bool isFloat = (pWfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ||
                            (pWfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE && IsEqualGUID(reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pWfx)->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT));

                        if (isFloat && pWfx->wBitsPerSample == 32) {
                            float* floatData = (float*)pData;
                            size_t sampleCount = numFramesAvailable * channels;
                            convertedBuffer.resize(sampleCount * sizeof(int16_t));
                            int16_t* intData = (int16_t*)convertedBuffer.data();
                            for (size_t i = 0; i < sampleCount; ++i) {
                                float sample = (std::max)(-1.0f, (std::min)(1.0f, floatData[i]));
                                intData[i] = static_cast<int16_t>(sample * 32767.0f);
                            }
                            conversionSuccess = true;
                        }
                        else if (pWfx->wFormatTag == WAVE_FORMAT_PCM && pWfx->wBitsPerSample == 16) {
                            size_t byteCount = numFramesAvailable * pWfx->nBlockAlign;
                            convertedBuffer.assign(pData, pData + byteCount);
                            conversionSuccess = true;
                        }

                        if (conversionSuccess) {
                            WriteFile(hFile, convertedBuffer.data(), (DWORD)convertedBuffer.size(), &bytesWritten, NULL);
                            dataSize += bytesWritten;
                        }
                        else {
                            WriteLog(L"%s 警告：接收到不支持的音频格式，已跳过数据包。", logPrefix);
                        }
                    }
                }
                pCaptureClient->ReleaseBuffer(numFramesAvailable);
            }
            else if (waitResult == WAIT_TIMEOUT) {
                UINT32 silentFrames = (sample_rate * waitTimeout) / 1000;
                DWORD silentBytes = silentFrames * block_align;
                if (silentBytes > 0) {
                    std::vector<BYTE> silentBuffer(silentBytes, 0);
                    WriteFile(hFile, silentBuffer.data(), silentBytes, &bytesWritten, NULL);
                    dataSize += bytesWritten;
                }
            }
            if (pCaptureClient && packetLength > 0) {
                pCaptureClient->GetNextPacketSize(&packetLength);
            }
        }

    } while (false);

    if (pAudioClient) pAudioClient->Stop();
    if (hFile != INVALID_HANDLE_VALUE) {
        SetFilePointer(hFile, 4, NULL, FILE_BEGIN);
        DWORD totalSize = 36 + dataSize;
        WriteFile(hFile, &totalSize, 4, &bytesWritten, NULL);
        SetFilePointer(hFile, 40, NULL, FILE_BEGIN);
        WriteFile(hFile, &dataSize, 4, &bytesWritten, NULL);
        CloseHandle(hFile);
    }

    CoTaskMemFree(pWfx);
    SAFE_RELEASE(pCaptureClient);
    SAFE_RELEASE(pAudioClient);
    SAFE_RELEASE(pDevice);
    SAFE_RELEASE(pEnumerator);
    if (hSamplesReadyEvent) CloseHandle(hSamplesReadyEvent);
    CoUninitialize();
}

void WasapiRecorder::RunFFmpegAndLog(const std::wstring& cmdLine) {
    std::wstring realCmd = cmdLine + L" 2>&1";
    WriteLog(L"[FFmpeg] 执行命令: %s", realCmd.c_str());
    FILE* pipe = _wpopen(realCmd.c_str(), L"rt, ccs=UTF-8");
    if (!pipe) {
        WriteLog(L"[FFmpeg] 启动失败，_wpopen返回null。");
        return;
    }
    wchar_t buf[512];
    while (fgetws(buf, _countof(buf), pipe)) {
        size_t len = wcslen(buf);
        if (len > 0 && (buf[len - 1] == L'\n' || buf[len - 1] == L'\r')) {
            buf[len - 1] = L'\0';
            if (len > 1 && (buf[len - 2] == L'\r' || buf[len - 2] == L'\n')) {
                buf[len - 2] = L'\0';
            }
        }
        if (wcslen(buf) > 0) { WriteLog(L"[FFmpeg] %s", buf); }
    }
    _pclose(pipe);
}
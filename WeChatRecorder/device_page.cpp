#include "device_page.h"
#include "log.h"
#include "device_enum.h" 
#include "config_manager.h"
#include "mic_level_sampler.h" 
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <CommCtrl.h>
#include <mmsystem.h>
#include <cmath>
#include <thread> 

#pragma comment(lib, "winmm.lib")

#define TEST_TIMER_ID 1
#define SAFE_RELEASE(punk)  if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }
#define REFTIMES_PER_SEC  10000000

// --- Globals for this page ---
static MicLevelSampler g_micSampler;
static bool g_isMicTesting = false;
static bool g_isSpeakerTesting = false; // State for speaker test

// --- Helper class for Speaker Test ---
class WasapiTestPlayer {
public:
    WasapiTestPlayer() : m_isPlaying(false) {}

    // Destructor ensures the thread is stopped and joined correctly
    ~WasapiTestPlayer() {
        Stop();
    }

    // Starts the playback thread
    void Start(const std::wstring& deviceId) {
        if (m_isPlaying) return;
        m_isPlaying = true;
        // The thread is now a member, not detached
        m_thread = std::thread(&WasapiTestPlayer::PlaySineWave, this, deviceId);
    }

    // Stops the playback thread
    void Stop() {
        m_isPlaying = false;
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

private:
    std::atomic<bool> m_isPlaying;
    std::thread m_thread;

    void PlaySineWave(const std::wstring& deviceId) {
        CoInitializeEx(NULL, COINIT_MULTITHREADED);
        HRESULT hr;
        IMMDeviceEnumerator* pEnumerator = nullptr;
        IMMDevice* pDevice = nullptr;
        IAudioClient* pAudioClient = nullptr;
        IAudioRenderClient* pRenderClient = nullptr;
        WAVEFORMATEX* pwfx = nullptr;
        HANDLE hEvent = NULL;

        const int FREQUENCY = 440;

        do {
            hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator));
            if (FAILED(hr)) { WriteLog(L"SpeakerTest: CoCreateInstance failed."); break; }

            hr = deviceId.empty() ? pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice) : pEnumerator->GetDevice(deviceId.c_str(), &pDevice);
            if (FAILED(hr)) { WriteLog(L"SpeakerTest: GetDevice failed."); break; }

            hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
            if (FAILED(hr)) { WriteLog(L"SpeakerTest: Activate failed."); break; }

            hr = pAudioClient->GetMixFormat(&pwfx);
            if (FAILED(hr)) { WriteLog(L"SpeakerTest: GetMixFormat failed."); break; }

            hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, REFTIMES_PER_SEC, 0, pwfx, NULL);
            if (FAILED(hr)) { WriteLog(L"SpeakerTest: Initialize failed."); break; }

            hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (hEvent == NULL) { hr = E_FAIL; break; }

            hr = pAudioClient->SetEventHandle(hEvent);
            if (FAILED(hr)) { WriteLog(L"SpeakerTest: SetEventHandle failed."); break; }

            UINT32 bufferFrameCount;
            hr = pAudioClient->GetBufferSize(&bufferFrameCount);
            if (FAILED(hr)) { WriteLog(L"SpeakerTest: GetBufferSize failed."); break; }

            hr = pAudioClient->GetService(IID_PPV_ARGS(&pRenderClient));
            if (FAILED(hr)) { WriteLog(L"SpeakerTest: GetService failed."); break; }

            hr = pAudioClient->Start();
            if (FAILED(hr)) { WriteLog(L"SpeakerTest: Start failed."); break; }

            double fAngle = 0;
            double fAngleDelta = 2.0 * 3.1415926535 * FREQUENCY / pwfx->nSamplesPerSec;

            while (m_isPlaying) {
                DWORD waitResult = WaitForSingleObject(hEvent, 200); // Wait for event or timeout
                if (waitResult != WAIT_OBJECT_0) {
                    // If timeout, just continue loop to check m_isPlaying flag
                    continue;
                }

                if (!m_isPlaying) break; // Check flag again after waiting

                UINT32 numFramesPadding;
                hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
                if (FAILED(hr)) break;

                UINT32 numFramesAvailable = bufferFrameCount - numFramesPadding;
                if (numFramesAvailable == 0) continue;

                BYTE* pData;
                hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);
                if (FAILED(hr)) { WriteLog(L"SpeakerTest: GetBuffer failed."); break; }

                float* pSample = (float*)pData;
                for (UINT32 i = 0; i < numFramesAvailable; i++) {
                    float value = (float)(sin(fAngle) * 0.1);
                    for (UINT32 j = 0; j < pwfx->nChannels; j++) {
                        *pSample++ = value;
                    }
                    fAngle += fAngleDelta;
                }

                hr = pRenderClient->ReleaseBuffer(numFramesAvailable, 0);
                if (FAILED(hr)) { WriteLog(L"SpeakerTest: ReleaseBuffer failed."); break; }
            }

            pAudioClient->Stop();

        } while (false);

        if (hEvent) CloseHandle(hEvent);
        CoTaskMemFree(pwfx);
        SAFE_RELEASE(pRenderClient);
        SAFE_RELEASE(pAudioClient);
        SAFE_RELEASE(pDevice);
        SAFE_RELEASE(pEnumerator);
        CoUninitialize();
    }
};

static WasapiTestPlayer g_speakerTester;


// --- Dialog Procedure ---
std::vector<FFmpegDeviceInfo> g_inputDevices;
std::vector<FFmpegDeviceInfo> g_outputDevices;
std::wstring g_selectedInputDeviceId;
std::wstring g_selectedOutputDeviceId;

void InitializeDefaultDevices() {
    g_inputDevices = EnumInputDevices();
    g_outputDevices = EnumOutputDevices();

    bool inputFound = false;
    for (const auto& device : g_inputDevices) {
        if (device.id == g_selectedInputDeviceId) {
            inputFound = true;
            break;
        }
    }
    if (!inputFound && !g_inputDevices.empty()) {
        g_selectedInputDeviceId = g_inputDevices[0].id;
    }

    bool outputFound = false;
    for (const auto& device : g_outputDevices) {
        if (device.id == g_selectedOutputDeviceId) {
            outputFound = true;
            break;
        }
    }
    if (!outputFound && !g_outputDevices.empty()) {
        g_selectedOutputDeviceId = g_outputDevices[0].id;
    }
}

INT_PTR CALLBACK DevicePageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        HWND hComboIn = GetDlgItem(hDlg, IDC_COMBO_INPUT_DEVICE);
        HWND hComboOut = GetDlgItem(hDlg, IDC_COMBO_OUTPUT_DEVICE);
        HWND hProgress = GetDlgItem(hDlg, IDC_PROGRESS_INPUT);
        SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

        SendMessageW(hComboIn, CB_RESETCONTENT, 0, 0);
        SendMessageW(hComboOut, CB_RESETCONTENT, 0, 0);

        int currentInputSel = -1;
        for (size_t i = 0; i < g_inputDevices.size(); ++i) {
            LRESULT index = SendMessageW(hComboIn, CB_ADDSTRING, 0, (LPARAM)g_inputDevices[i].name.c_str());
            SendMessageW(hComboIn, CB_SETITEMDATA, index, (LPARAM)i);
            if (g_inputDevices[i].id == g_selectedInputDeviceId) {
                currentInputSel = (int)index;
            }
        }

        int currentOutputSel = -1;
        for (size_t i = 0; i < g_outputDevices.size(); ++i) {
            LRESULT index = SendMessageW(hComboOut, CB_ADDSTRING, 0, (LPARAM)g_outputDevices[i].name.c_str());
            SendMessageW(hComboOut, CB_SETITEMDATA, index, (LPARAM)i);
            if (g_outputDevices[i].id == g_selectedOutputDeviceId) {
                currentOutputSel = (int)index;
            }
        }

        if (currentInputSel != -1) SendMessageW(hComboIn, CB_SETCURSEL, currentInputSel, 0);
        if (currentOutputSel != -1) SendMessageW(hComboOut, CB_SETCURSEL, currentOutputSel, 0);

        return TRUE;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam)) {
        case IDC_COMBO_INPUT_DEVICE:
        case IDC_COMBO_OUTPUT_DEVICE:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                if (g_isMicTesting) {
                    g_micSampler.Stop();
                    KillTimer(hDlg, TEST_TIMER_ID);
                    SetDlgItemText(hDlg, IDC_BTN_TEST_INPUT, L"测试");
                    g_isMicTesting = false;
                }
                if (g_isSpeakerTesting) {
                    g_speakerTester.Stop();
                    SetDlgItemText(hDlg, IDC_BTN_TEST_OUTPUT, L"测试");
                    g_isSpeakerTesting = false;
                }
                HWND hCombo = (HWND)lParam;
                int ctrlId = GetDlgCtrlID(hCombo);
                LRESULT sel = SendMessageW(hCombo, CB_GETCURSEL, 0, 0);

                if (sel != CB_ERR) {
                    LRESULT dataIndex = SendMessageW(hCombo, CB_GETITEMDATA, sel, 0);
                    if (ctrlId == IDC_COMBO_INPUT_DEVICE) {
                        g_selectedInputDeviceId = g_inputDevices[dataIndex].id;
                    }
                    else if (ctrlId == IDC_COMBO_OUTPUT_DEVICE) {
                        g_selectedOutputDeviceId = g_outputDevices[dataIndex].id;
                    }
                    SaveConfig();
                }
            }
            break;
        case IDC_BTN_TEST_INPUT:
        {
            if (!g_isMicTesting) {
                g_micSampler.Start(g_selectedInputDeviceId);
                SetTimer(hDlg, TEST_TIMER_ID, 100, NULL);
                SetDlgItemText(hDlg, IDC_BTN_TEST_INPUT, L"停止");
            }
            else {
                g_micSampler.Stop();
                KillTimer(hDlg, TEST_TIMER_ID);
                HWND hProgress = GetDlgItem(hDlg, IDC_PROGRESS_INPUT);
                SendMessage(hProgress, PBM_SETPOS, 0, 0);
                SetDlgItemText(hDlg, IDC_BTN_TEST_INPUT, L"测试");
            }
            g_isMicTesting = !g_isMicTesting;
            break;
        }
        case IDC_BTN_TEST_OUTPUT:
        {
            if (!g_isSpeakerTesting) {
                g_speakerTester.Start(g_selectedOutputDeviceId);
                SetDlgItemText(hDlg, IDC_BTN_TEST_OUTPUT, L"停止");
            }
            else {
                g_speakerTester.Stop();
                SetDlgItemText(hDlg, IDC_BTN_TEST_OUTPUT, L"测试");
            }
            g_isSpeakerTesting = !g_isSpeakerTesting;
            break;
        }
        }
        break;
    }
    case WM_TIMER:
    {
        if (wParam == TEST_TIMER_ID) {
            float level = g_micSampler.GetLevel();
            HWND hProgress = GetDlgItem(hDlg, IDC_PROGRESS_INPUT);
            SendMessage(hProgress, PBM_SETPOS, (int)(level * 100), 0);
        }
        break;
    }
    case WM_DESTROY:
    {
        if (g_isMicTesting) {
            g_micSampler.Stop();
            KillTimer(hDlg, TEST_TIMER_ID);
        }
        if (g_isSpeakerTesting) {
            g_speakerTester.Stop();
        }
        break;
    }
    }
    return FALSE;
}

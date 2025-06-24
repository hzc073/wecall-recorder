#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <comdef.h>
#include <TlHelp32.h>
#include <string>
#include "wechat_detector.h"
#include "resource.h"


extern HWND hDevicePage;
// 获取进程名的辅助函数
std::wstring GetProcessNameById(DWORD pid) {
    std::wstring name = L"";
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(hSnap, &pe)) {
            do {
                if (pe.th32ProcessID == pid) {
                    name = pe.szExeFile;
                    break;
                }
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
    return name;
}

// 检测麦克风是否被占用，并返回第一个占用进程名
bool IsMicInUse(std::wstring& processName) {
    processName.clear();
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    bool ret = false;

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioSessionManager2* pSessionManager = nullptr;
    IAudioSessionEnumerator* pSessionList = nullptr;

    do {
        // 获取默认麦克风设备
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
            __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
        if (FAILED(hr)) break;
        hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
        if (FAILED(hr)) break;
        hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&pSessionManager);
        if (FAILED(hr)) break;
        hr = pSessionManager->GetSessionEnumerator(&pSessionList);
        if (FAILED(hr)) break;

        int sessionCount = 0;
        hr = pSessionList->GetCount(&sessionCount);
        if (FAILED(hr)) break;

        DWORD selfPid = GetCurrentProcessId();

        for (int i = 0; i < sessionCount; ++i) {
            IAudioSessionControl* pSessionControl = nullptr;
            hr = pSessionList->GetSession(i, &pSessionControl);
            if (SUCCEEDED(hr) && pSessionControl) {
                IAudioSessionControl2* pSessionControl2 = nullptr;
                hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);
                if (SUCCEEDED(hr) && pSessionControl2) {
                    DWORD pid = 0;
                    pSessionControl2->GetProcessId(&pid);

                    AudioSessionState state;
                    pSessionControl2->GetState(&state);

                    if (state == AudioSessionStateActive && pid != 0 && pid != selfPid) {
                        // 检测到有非本进程在用麦克风
                        processName = GetProcessNameById(pid);
                        ret = true;
                        pSessionControl2->Release();
                        pSessionControl->Release();
                        break;
                    }
                    pSessionControl2->Release();
                }
                pSessionControl->Release();
            }
        }
    } while (false);

    if (pSessionList) pSessionList->Release();
    if (pSessionManager) pSessionManager->Release();
    if (pDevice) pDevice->Release();
    if (pEnumerator) pEnumerator->Release();

    CoUninitialize();
    return ret;
}

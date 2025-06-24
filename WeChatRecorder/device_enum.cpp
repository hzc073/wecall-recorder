// �ļ�����device_enum.cpp
#include "device_enum.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include "log.h"

// �ڲ��������������� FFmpegDeviceInfo ����
static std::vector<FFmpegDeviceInfo> EnumDevicesInternal(EDataFlow dataFlow) {
    std::vector<FFmpegDeviceInfo> devices;
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        WriteLog(L"[EnumDevice] CoInitializeEx ʧ��, hr=0x%08X", hr);
        return devices;
    }

    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator));
    if (FAILED(hr)) {
        WriteLog(L"[EnumDevice] CoCreateInstance ʧ��, hr=0x%08X", hr);
        CoUninitialize();
        return devices;
    }

    IMMDeviceCollection* pCollection = nullptr;
    hr = pEnumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, &pCollection);
    if (FAILED(hr)) {
        WriteLog(L"[EnumDevice] EnumAudioEndpoints ʧ��, hr=0x%08X", hr);
        pEnumerator->Release();
        CoUninitialize();
        return devices;
    }

    UINT count;
    pCollection->GetCount(&count);
    WriteLog(L"[EnumDevice] �ҵ� %u �� %s �豸", count, (dataFlow == eCapture) ? L"����" : L"���");

    for (UINT i = 0; i < count; i++) {
        IMMDevice* pDevice = nullptr;
        hr = pCollection->Item(i, &pDevice);
        if (SUCCEEDED(hr)) {
            LPWSTR pwszID = nullptr;
            hr = pDevice->GetId(&pwszID);
            if (SUCCEEDED(hr)) {
                IPropertyStore* pProps = nullptr;
                hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
                if (SUCCEEDED(hr)) {
                    PROPVARIANT varName;
                    PropVariantInit(&varName);
                    hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
                    if (SUCCEEDED(hr)) {
                        // --- �����޸��㣺���� FFmpegDeviceInfo ���� ---
                        FFmpegDeviceInfo info;
                        info.id = pwszID;
                        info.name = varName.pwszVal;
                        devices.push_back(info);
                        WriteLog(L"  - �豸 %d: %s (ID: %s)", i, info.name.c_str(), info.id.c_str());
                    }
                    PropVariantClear(&varName);
                    pProps->Release();
                }
                CoTaskMemFree(pwszID);
            }
            pDevice->Release();
        }
    }

    pCollection->Release();
    pEnumerator->Release();
    CoUninitialize();

    return devices;
}

// ʵ�ַ��� FFmpegDeviceInfo ���͵ĺ���
std::vector<FFmpegDeviceInfo> EnumInputDevices() {
    return EnumDevicesInternal(eCapture);
}

std::vector<FFmpegDeviceInfo> EnumOutputDevices() {
    return EnumDevicesInternal(eRender);
}
#include "device_enum.h"
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

//输入
std::vector<DeviceInfo> EnumInputDevices() {
    std::vector<DeviceInfo> result;
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    IMMDeviceEnumerator* pEnum = nullptr;
    IMMDeviceCollection* pDevices = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pEnum));
    if (FAILED(hr)) return result;

    hr = pEnum->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pDevices);
    if (SUCCEEDED(hr)) {
        UINT count = 0;
        pDevices->GetCount(&count);
        for (UINT i = 0; i < count; ++i) {
            IMMDevice* pDevice = nullptr;
            if (SUCCEEDED(pDevices->Item(i, &pDevice))) {
                LPWSTR id = nullptr;
                pDevice->GetId(&id);

                IPropertyStore* pProps = nullptr;
                std::wstring name;
                if (SUCCEEDED(pDevice->OpenPropertyStore(STGM_READ, &pProps))) {
                    PROPVARIANT varName;
                    PropVariantInit(&varName);
                    if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName))) {
                        name = varName.pwszVal;
                        PropVariantClear(&varName);
                    }
                    pProps->Release();
                }
                result.push_back({ id, name });
                CoTaskMemFree(id);
                pDevice->Release();
            }
        }
        pDevices->Release();
    }
    if (pEnum) pEnum->Release();
    CoUninitialize();
    return result;
}

//输出
std::vector<DeviceInfo> EnumOutputDevices() {
    std::vector<DeviceInfo> result;
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    IMMDeviceEnumerator* pEnum = nullptr;
    IMMDeviceCollection* pDevices = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pEnum));
    if (FAILED(hr)) return result;
    hr = pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);
    if (SUCCEEDED(hr)) {
        UINT count = 0;
        pDevices->GetCount(&count);
        for (UINT i = 0; i < count; ++i) {
            IMMDevice* pDevice = nullptr;
            if (SUCCEEDED(pDevices->Item(i, &pDevice))) {
                LPWSTR id = nullptr;
                pDevice->GetId(&id);
                IPropertyStore* pProps = nullptr;
                std::wstring name;
                if (SUCCEEDED(pDevice->OpenPropertyStore(STGM_READ, &pProps))) {
                    PROPVARIANT varName;
                    PropVariantInit(&varName);
                    if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &varName))) {
                        name = varName.pwszVal;
                        PropVariantClear(&varName);
                    }
                    pProps->Release();
                }
                result.push_back({ id, name });
                CoTaskMemFree(id);
                pDevice->Release();
            }
        }
        pDevices->Release();
    }
    if (pEnum) pEnum->Release();
    CoUninitialize();
    return result;
}


//
// device_enum.cpp
#include <endpointvolume.h> // 需要加头文件

// 获取默认输入设备（麦克风）音量（峰值）
float GetMicPeakLevel() {
    HRESULT hr;
    float peak = 0.0f;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    IMMDeviceEnumerator* pEnum = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioMeterInformation* pMeter = nullptr;

    do {
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pEnum));
        if (FAILED(hr)) break;

        hr = pEnum->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
        if (FAILED(hr)) break;

        hr = pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeter);
        if (FAILED(hr)) break;

        hr = pMeter->GetPeakValue(&peak);
        if (FAILED(hr)) peak = 0.0f;
    } while (false);

    if (pMeter) pMeter->Release();
    if (pDevice) pDevice->Release();
    if (pEnum) pEnum->Release();
    CoUninitialize();

    // 返回值 0.0~1.0
    return peak;
}

// 获取默认输出设备（扬声器）音量（峰值）
float GetSpeakerPeakLevel() {
    HRESULT hr;
    float peak = 0.0f;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    IMMDeviceEnumerator* pEnum = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioMeterInformation* pMeter = nullptr;

    do {
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pEnum));
        if (FAILED(hr)) break;

        hr = pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
        if (FAILED(hr)) break;

        hr = pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeter);
        if (FAILED(hr)) break;

        hr = pMeter->GetPeakValue(&peak);
        if (FAILED(hr)) peak = 0.0f;
    } while (false);

    if (pMeter) pMeter->Release();
    if (pDevice) pDevice->Release();
    if (pEnum) pEnum->Release();
    CoUninitialize();

    // 返回值 0.0~1.0
    return peak;
}



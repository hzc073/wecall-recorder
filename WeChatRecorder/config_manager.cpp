/*
 * =====================================================================================
 *
 * Filename:  config_manager.cpp
 *
 * Description:  Handles saving and loading application settings to/from an INI file.
 *
 * =====================================================================================
 */
#include "config_manager.h"
#include "device_page.h"
#include "path_page.h"
#include "general_page.h"
#include "blacklist_page.h"
#include <vector>
#include <sstream>

 // Helper function to get the full path of the INI file.
 // The INI file is stored in the same directory as the executable.
std::wstring GetIniFilePath() {
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    wchar_t* p = wcsrchr(path, L'\\');
    if (p) {
        *(p + 1) = L'\0';
    }
    return std::wstring(path) + L"config.ini";
}

void LoadConfig() {
    std::wstring iniPath = GetIniFilePath();
    wchar_t buffer[1024];

    // [Devices]
    GetPrivateProfileStringW(L"Devices", L"Input", L"", buffer, 1024, iniPath.c_str());
    g_selectedInputDeviceId = buffer;
    GetPrivateProfileStringW(L"Devices", L"Output", L"", buffer, 1024, iniPath.c_str());
    g_selectedOutputDeviceId = buffer;

    // [Paths]
    GetPrivateProfileStringW(L"Paths", L"SavePath", L"C:\\Â¼ÒôÎÄ¼þ", buffer, 1024, iniPath.c_str());
    g_savePath = buffer;

    // [Settings]
    g_micVolumePercent = GetPrivateProfileIntW(L"Settings", L"MicVolume", 100, iniPath.c_str());
    g_speakerVolumePercent = GetPrivateProfileIntW(L"Settings", L"SpeakerVolume", 50, iniPath.c_str());
    g_startupEnabled = GetPrivateProfileIntW(L"Settings", L"Startup", 0, iniPath.c_str()) == 1;
    g_minimizeToTray = GetPrivateProfileIntW(L"Settings", L"MinimizeToTray", 0, iniPath.c_str()) == 1;

    // [Blacklist]
    g_blacklist.clear();
    GetPrivateProfileStringW(L"Blacklist", L"Apps", L"", buffer, 1024, iniPath.c_str());
    std::wstringstream ss(buffer);
    std::wstring appName;
    while (std::getline(ss, appName, L';')) {
        if (!appName.empty()) {
            g_blacklist.push_back(appName);
        }
    }
}

void SaveConfig() {
    std::wstring iniPath = GetIniFilePath();

    // [Devices]
    WritePrivateProfileStringW(L"Devices", L"Input", g_selectedInputDeviceId.c_str(), iniPath.c_str());
    WritePrivateProfileStringW(L"Devices", L"Output", g_selectedOutputDeviceId.c_str(), iniPath.c_str());

    // [Paths]
    WritePrivateProfileStringW(L"Paths", L"SavePath", g_savePath.c_str(), iniPath.c_str());

    // [Settings]
    WritePrivateProfileStringW(L"Settings", L"MicVolume", std::to_wstring(g_micVolumePercent).c_str(), iniPath.c_str());
    WritePrivateProfileStringW(L"Settings", L"SpeakerVolume", std::to_wstring(g_speakerVolumePercent).c_str(), iniPath.c_str());
    WritePrivateProfileStringW(L"Settings", L"Startup", g_startupEnabled ? L"1" : L"0", iniPath.c_str());
    WritePrivateProfileStringW(L"Settings", L"MinimizeToTray", g_minimizeToTray ? L"1" : L"0", iniPath.c_str());

    // [Blacklist]
    std::wstringstream ss;
    for (size_t i = 0; i < g_blacklist.size(); ++i) {
        ss << g_blacklist[i];
        if (i < g_blacklist.size() - 1) {
            ss << L";";
        }
    }
    WritePrivateProfileStringW(L"Blacklist", L"Apps", ss.str().c_str(), iniPath.c_str());
}

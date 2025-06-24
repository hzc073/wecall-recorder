#include "startup_util.h"
#include "log.h"

const wchar_t* AppRegName = L"自动拾音 --by夜莺";

// 获取当前可执行文件的完整路径
std::wstring GetExecutablePath() {
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return path;
}

// 设置或取消开机启动
void SetStartupStatus(bool enable) {
    HKEY hKey = NULL;
    // HKEY_CURRENT_USER 不需要管理员权限
    const wchar_t* path = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

    if (RegOpenKeyExW(HKEY_CURRENT_USER, path, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        if (enable) {
            std::wstring exePath = L"\"" + GetExecutablePath() + L"\""; // 为路径加上引号以处理空格
            RegSetValueExW(hKey, AppRegName, 0, REG_SZ, (const BYTE*)exePath.c_str(), (DWORD)((exePath.length() + 1) * sizeof(wchar_t)));
        }
        else {
            RegDeleteValueW(hKey, AppRegName);
        }
        RegCloseKey(hKey);
    }
}

// 获取当前是否为开机启动
bool GetStartupStatus() {
    HKEY hKey = NULL;
    const wchar_t* path = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    bool isEnabled = false;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, path, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, AppRegName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            isEnabled = true;
        }
        RegCloseKey(hKey);
    }
    return isEnabled;
}
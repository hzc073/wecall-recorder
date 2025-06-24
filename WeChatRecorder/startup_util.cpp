#include "startup_util.h"
#include "log.h"

const wchar_t* AppRegName = L"�Զ�ʰ�� --byҹݺ";

// ��ȡ��ǰ��ִ���ļ�������·��
std::wstring GetExecutablePath() {
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return path;
}

// ���û�ȡ����������
void SetStartupStatus(bool enable) {
    HKEY hKey = NULL;
    // HKEY_CURRENT_USER ����Ҫ����ԱȨ��
    const wchar_t* path = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

    if (RegOpenKeyExW(HKEY_CURRENT_USER, path, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        if (enable) {
            std::wstring exePath = L"\"" + GetExecutablePath() + L"\""; // Ϊ·�����������Դ���ո�
            RegSetValueExW(hKey, AppRegName, 0, REG_SZ, (const BYTE*)exePath.c_str(), (DWORD)((exePath.length() + 1) * sizeof(wchar_t)));
        }
        else {
            RegDeleteValueW(hKey, AppRegName);
        }
        RegCloseKey(hKey);
    }
}

// ��ȡ��ǰ�Ƿ�Ϊ��������
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
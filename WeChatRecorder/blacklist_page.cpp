#include "blacklist_page.h"
#include "resource.h"
#include "log.h"
#include "config_manager.h" // Include config manager
#include <CommCtrl.h>

void RefreshBlacklistListbox(HWND hDlg) {
    HWND hList = GetDlgItem(hDlg, IDC_LIST_BLACKLIST);
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    for (const auto& appName : g_blacklist) {
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)appName.c_str());
    }
}

INT_PTR CALLBACK BlacklistPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG: {
        RefreshBlacklistListbox(hDlg);
        return TRUE;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_BTN_ADD_BLACKLIST: {
            wchar_t appNameBuf[MAX_PATH] = { 0 };
            GetDlgItemTextW(hDlg, IDC_EDIT_BLACKLIST_APP, appNameBuf, MAX_PATH);
            std::wstring appName = appNameBuf;

            if (!appName.empty()) {
                bool found = false;
                for (const auto& item : g_blacklist) {
                    if (_wcsicmp(item.c_str(), appName.c_str()) == 0) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    g_blacklist.push_back(appName);
                    RefreshBlacklistListbox(hDlg);
                    SetDlgItemTextW(hDlg, IDC_EDIT_BLACKLIST_APP, L"");
                    WriteLog(L"[黑名单] 添加了应用: %s", appName.c_str());
                    SaveConfig(); // Save after adding
                }
                else {
                    MessageBoxW(hDlg, L"该应用已在黑名单中。", L"提示", MB_OK | MB_ICONINFORMATION);
                }
            }
            else {
                MessageBoxW(hDlg, L"请输入应用程序名称。", L"提示", MB_OK | MB_ICONWARNING);
            }
            return TRUE;
        }
        case IDC_BTN_DEL_BLACKLIST: {
            HWND hList = GetDlgItem(hDlg, IDC_LIST_BLACKLIST);
            int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);

            if (sel != LB_ERR) {
                wchar_t appNameBuf[MAX_PATH];
                SendMessage(hList, LB_GETTEXT, sel, (LPARAM)appNameBuf);

                for (auto it = g_blacklist.begin(); it != g_blacklist.end(); ++it) {
                    if (_wcsicmp(it->c_str(), appNameBuf) == 0) {
                        g_blacklist.erase(it);
                        break;
                    }
                }
                RefreshBlacklistListbox(hDlg);
                WriteLog(L"[黑名单] 删除了应用: %s", appNameBuf);
                SaveConfig(); // Save after deleting
            }
            else {
                MessageBoxW(hDlg, L"请先在列表中选择一个要删除的应用。", L"提示", MB_OK | MB_ICONWARNING);
            }
            return TRUE;
        }
        }
        break;
    }
    }
    return FALSE;
}

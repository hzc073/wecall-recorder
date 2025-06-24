#include "path_page.h"
#include "resource.h"
#include "config_manager.h" // Include config manager
#include <shlobj.h> 
#include <string>

std::wstring g_savePath = L"C:\\录音文件";

// Folder browser helper function
bool BrowseForFolder(HWND hWnd, std::wstring& outPath) {
    BROWSEINFOW bi = { 0 };
    wchar_t szDisplayName[MAX_PATH] = { 0 };
    bi.hwndOwner = hWnd;
    bi.pszDisplayName = szDisplayName;
    bi.lpszTitle = L"请选择录音保存文件夹";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (pidl && SHGetPathFromIDListW(pidl, szDisplayName)) {
        outPath = szDisplayName;
        CoTaskMemFree(pidl);
        return true;
    }
    return false;
}

INT_PTR CALLBACK PathPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        // Set text from global variable loaded from config
        SetDlgItemTextW(hDlg, IDC_EDIT_PATH, g_savePath.c_str());
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_BROWSE:
        {
            std::wstring newPath;
            if (BrowseForFolder(hDlg, newPath)) {
                g_savePath = newPath;
                SetDlgItemTextW(hDlg, IDC_EDIT_PATH, g_savePath.c_str());
                SaveConfig(); // Save after browsing and selecting a new path
            }
            break;
        }
        // This button might be redundant if we save on browse, but we keep it for manual edits.
        // It's better to remove the button and save on text change or dialog close, but for now this works.
        case IDOK: // Or a dedicated save button
        {
            wchar_t pathBuf[MAX_PATH] = { 0 };
            GetDlgItemTextW(hDlg, IDC_EDIT_PATH, pathBuf, MAX_PATH);
            g_savePath = pathBuf;
            SaveConfig(); // Save on clicking OK or a save button
            MessageBoxW(hDlg, L"保存路径已更新！", L"提示", MB_OK | MB_ICONINFORMATION);
            break;
        }
        }
        break;
    }
    return FALSE;
}

// 头文件区
#include "framework.h"
#include "resource.h"
#include <commctrl.h>
#include <atomic>
#include "wasapi_recorder.h" //录音模块
#include "wechat_detector.h" //微信识别模块
#include "device_page.h"     // 设备管理模块
#include "path_page.h"      //文件保存路径模块
#include "about_page.h"     //关于模块
#include "custom_button.h"   //UI设计
#include "path_util.h"  //文件保存命名规则


// 全局变量声明
HINSTANCE hInst;
HWND hDevicePage = NULL;
HWND hPathPage = NULL;
HWND hAboutPage = NULL;
Recorder* g_recorder = nullptr;

HANDLE hMonitorThread = NULL;
std::atomic<bool> monitorRunning = false;
bool isRecording = false;
bool isMonitorStarted = false;

// 
INT_PTR CALLBACK MainDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DevicePageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PathPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
//动态保存文件
std::wstring DetectCurrentAppName() {
    if (FindWindowW(L"AudioWnd", NULL) || FindWindowW(L"VoipWnd", NULL)) {
        return L"微信";
    }
    if (FindWindowW(L"QQVoiceWnd", NULL)) {
        return L"QQ";
    }
    // 以后扩展更多
    return L"未知";
}

// ========== 录音开始 ==========
void StartRecording(HWND hWnd) {
    isRecording = true;
    SetDlgItemText(hWnd, IDC_LABEL_STATUS, L"正在录音...");
    std::wstring appName = DetectCurrentAppName();
    g_recorder->Start(GenerateSavePath(g_savePath, appName));
}


// ========== 录音结束 ==========
void StopRecording(HWND hWnd) {
    isRecording = false;
    SetDlgItemText(hWnd, IDC_LABEL_STATUS, L"等待中...");
    g_recorder->Stop();
}

// ========== 监控线程 ==========
DWORD WINAPI MonitorThreadProc(LPVOID lpParam) {
    HWND hWnd = (HWND)lpParam;
    int delayCounter = 0;
    while (monitorRunning) {
        BOOL isChecked = IsDlgButtonChecked(hWnd, IDC_CHECK_WECHAT);
        if (isChecked != BST_CHECKED) {
            delayCounter = 0;
            Sleep(1000);
            continue;
        }

        bool found = IsWeChatCalling();  // 用新的检测函数
        if (found && !isRecording) {
            PostMessage(hWnd, WM_USER + 1, 0, 0);
            delayCounter = 0;
        }
        else if (!found && isRecording) {
            delayCounter++;
            if (delayCounter >= 3) {
                PostMessage(hWnd, WM_USER + 2, 0, 0);
                delayCounter = 0;
            }
        }
        else {
            delayCounter = 0;
        }
        Sleep(1000);
    }
    return 0;
}

// ========== 主对话框 ==========
INT_PTR CALLBACK MainDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hBtnStart = NULL;
    switch (message) {
    case WM_INITDIALOG:
        hBtnStart = GetDlgItem(hDlg, IDC_BTN_START);
        EnableWindow(hBtnStart, FALSE);
        SetDlgItemText(hDlg, IDC_LABEL_STATUS, L"等待中...");
        isMonitorStarted = false;
        monitorRunning = false;
        return TRUE;

    case WM_DRAWITEM:
        if (HandleMacStyleButtonDrawAuto((LPDRAWITEMSTRUCT)lParam, (int)wParam))
            return TRUE;
        break;

    case WM_USER + 1:
        StartRecording(hDlg);
        return TRUE;
    case WM_USER + 2:
        StopRecording(hDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_CHECK_WECHAT:
        {
            BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECK_WECHAT);
            if (!isMonitorStarted)
                EnableWindow(hBtnStart, checked == BST_CHECKED);
            break;
        }
        case IDC_BTN_START:
            if (!isMonitorStarted) {
                isMonitorStarted = true;
                monitorRunning = true;
                hMonitorThread = CreateThread(NULL, 0, MonitorThreadProc, hDlg, 0, NULL);
                SetDlgItemText(hDlg, IDC_LABEL_STATUS, L"已启动检测，等待微信通话...");
                SetDlgItemText(hDlg, IDC_BTN_START, L"停止录音");
                EnableWindow(hBtnStart, TRUE);
            }
            else {
                monitorRunning = false;
                if (hMonitorThread) {
                    WaitForSingleObject(hMonitorThread, 2000);
                    CloseHandle(hMonitorThread);
                    hMonitorThread = NULL;
                }
                isMonitorStarted = false;
                SetDlgItemText(hDlg, IDC_LABEL_STATUS, L"等待中...");
                SetDlgItemText(hDlg, IDC_BTN_START, L"开始录音");
                BOOL checked = IsDlgButtonChecked(hDlg, IDC_CHECK_WECHAT);
                EnableWindow(hBtnStart, checked == BST_CHECKED);
            }
            break;
        case IDC_BUTTON2:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS_DIALOG), hDlg, SettingsDlgProc);
            break;
        case IDCANCEL:
            monitorRunning = false;
            if (hMonitorThread) {
                WaitForSingleObject(hMonitorThread, 1000);
                CloseHandle(hMonitorThread);
            }
            delete g_recorder;
            g_recorder = nullptr;
            EndDialog(hDlg, 0);
            break;
        default:
            break;
        }
        return TRUE;
    }
    return FALSE;
}

// ========== 设置对话框 ==========
INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        HWND hPlaceholder = GetDlgItem(hDlg, IDC_SUBPAGE_PLACEHOLDER);
        RECT rc;
        GetWindowRect(hPlaceholder, &rc);
        MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&rc, 2);

        hDevicePage = CreateDialog(hInst, MAKEINTRESOURCE(IDD_PAGE_DEVICE), hDlg, DevicePageProc);
        hPathPage = CreateDialog(hInst, MAKEINTRESOURCE(IDD_PAGE_PATH), hDlg, PathPageProc);
        hAboutPage = CreateDialog(hInst, MAKEINTRESOURCE(IDD_PAGE_ABOUT), hDlg, AboutPageProc);

        MoveWindow(hDevicePage, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
        MoveWindow(hPathPage, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
        MoveWindow(hAboutPage, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

        ShowWindow(hDevicePage, SW_SHOW);
        ShowWindow(hPathPage, SW_HIDE);
        ShowWindow(hAboutPage, SW_HIDE);
        return TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_DEVICE:
            ShowWindow(hDevicePage, SW_SHOW);
            ShowWindow(hPathPage, SW_HIDE);
            ShowWindow(hAboutPage, SW_HIDE);
            return TRUE;

        case IDC_BTN_PATH:
            ShowWindow(hDevicePage, SW_HIDE);
            ShowWindow(hPathPage, SW_SHOW);
            ShowWindow(hAboutPage, SW_HIDE);
            return TRUE;

        case IDC_BTN_ABOUT:
            ShowWindow(hDevicePage, SW_HIDE);
            ShowWindow(hPathPage, SW_HIDE);
            ShowWindow(hAboutPage, SW_SHOW);
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, 0);
            return TRUE;
        default:
            break;
        }
        break;
    }
    return FALSE;
}

// ========== WinMain ==========
int APIENTRY wWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);

    hInst = hInstance;
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, MainDialogProc);

    return 0;
}

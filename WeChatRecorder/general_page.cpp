#include "general_page.h"
#include "resource.h"
#include "startup_util.h"
#include "config_manager.h"
#include <CommCtrl.h>
#include <string>

#pragma comment(lib, "comctl32.lib")

// Default values, will be overwritten by LoadConfig() if config.ini exists
int g_micVolumePercent = 100;
int g_speakerVolumePercent = 50;
bool g_startupEnabled = false;
bool g_minimizeToTray = false;

void UpdateVolumeText(HWND hDlg) {
    wchar_t buf[16];
    swprintf_s(buf, L"%d%%", g_micVolumePercent);
    SetDlgItemTextW(hDlg, IDC_STATIC_MIC_VOL, buf);

    swprintf_s(buf, L"%d%%", g_speakerVolumePercent);
    SetDlgItemTextW(hDlg, IDC_STATIC_SPEAKER_VOL, buf);
}

INT_PTR CALLBACK GeneralPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        // Set UI elements based on global variables (which were loaded by LoadConfig)
        CheckDlgButton(hDlg, IDC_CHECK_STARTUP, g_startupEnabled ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_MINIMIZE_TRAY, g_minimizeToTray ? BST_CHECKED : BST_UNCHECKED);

        HWND hSliderMic = GetDlgItem(hDlg, IDC_SLIDER_MIC_VOL);
        HWND hSliderSpeaker = GetDlgItem(hDlg, IDC_SLIDER_SPEAKER_VOL);

        SendMessage(hSliderMic, TBM_SETRANGE, TRUE, MAKELPARAM(0, 200));
        SendMessage(hSliderMic, TBM_SETPOS, TRUE, g_micVolumePercent);
        SendMessage(hSliderSpeaker, TBM_SETRANGE, TRUE, MAKELPARAM(0, 200));
        SendMessage(hSliderSpeaker, TBM_SETPOS, TRUE, g_speakerVolumePercent);
        UpdateVolumeText(hDlg);

        return TRUE;
    }
    case WM_HSCROLL:
    {
        HWND hSlider = (HWND)lParam;
        if (hSlider == GetDlgItem(hDlg, IDC_SLIDER_MIC_VOL)) {
            g_micVolumePercent = (int)SendMessage(hSlider, TBM_GETPOS, 0, 0);
        }
        else if (hSlider == GetDlgItem(hDlg, IDC_SLIDER_SPEAKER_VOL)) {
            g_speakerVolumePercent = (int)SendMessage(hSlider, TBM_GETPOS, 0, 0);
        }
        UpdateVolumeText(hDlg);
        SaveConfig(); // Save after slider changes
        break;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam)) {
        case IDC_CHECK_STARTUP:
            g_startupEnabled = IsDlgButtonChecked(hDlg, IDC_CHECK_STARTUP) == BST_CHECKED;
            SetStartupStatus(g_startupEnabled);
            SaveConfig();
            break;
        case IDC_CHECK_MINIMIZE_TRAY:
            g_minimizeToTray = IsDlgButtonChecked(hDlg, IDC_CHECK_MINIMIZE_TRAY) == BST_CHECKED;
            SaveConfig();
            break;
        }
        break;
    }
    }
    return FALSE;
}

#include "device_page.h"
#include "resource.h"
#include <commctrl.h>
#include "device_enum.h"
#include "mic_level_sampler.h"


static MicLevelSampler g_micSampler;
static bool isTestingInput = false;
static bool isTestingOutput = false;

INT_PTR CALLBACK DevicePageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        // 输入设备下拉
        HWND hComboIn = GetDlgItem(hDlg, IDC_COMBO_INPUT_DEVICE);
        if (hComboIn) {
            auto devsIn = EnumInputDevices();
            for (const auto& d : devsIn) {
                SendMessageW(hComboIn, CB_ADDSTRING, 0, (LPARAM)d.name.c_str());
            }
            SendMessageW(hComboIn, CB_SETCURSEL, 0, 0);
        }
        // 输出设备下拉
        HWND hComboOut = GetDlgItem(hDlg, IDC_COMBO_OUTPUT_DEVICE);
        if (hComboOut) {
            auto devsOut = EnumOutputDevices();
            for (const auto& d : devsOut) {
                SendMessageW(hComboOut, CB_ADDSTRING, 0, (LPARAM)d.name.c_str());
            }
            SendMessageW(hComboOut, CB_SETCURSEL, 0, 0);
        }
        return TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_TEST_INPUT:
            if (!isTestingInput) {
                isTestingInput = true;
                isTestingOutput = false;
                EnableWindow(GetDlgItem(hDlg, IDC_BTN_TEST_OUTPUT), FALSE);
                SetDlgItemText(hDlg, IDC_BTN_TEST_INPUT, L"停止输入测试");
                g_micSampler.Start();
                SetTimer(hDlg, 1, 100, NULL);
            }
            else {
                isTestingInput = false;
                EnableWindow(GetDlgItem(hDlg, IDC_BTN_TEST_OUTPUT), TRUE);
                SetDlgItemText(hDlg, IDC_BTN_TEST_INPUT, L"测试");
                g_micSampler.Stop();
                KillTimer(hDlg, 1);
                SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_INPUT), PBM_SETPOS, 0, 0);
            }
            return TRUE;
        case IDC_BTN_TEST_OUTPUT:
            if (!isTestingOutput) {
                isTestingOutput = true;
                isTestingInput = false;
                EnableWindow(GetDlgItem(hDlg, IDC_BTN_TEST_INPUT), FALSE);
                SetDlgItemText(hDlg, IDC_BTN_TEST_OUTPUT, L"停止测试");
                SetTimer(hDlg, 2, 100, NULL); // 100ms刷新
            }
            else {
                isTestingOutput = false;
                EnableWindow(GetDlgItem(hDlg, IDC_BTN_TEST_INPUT), TRUE);
                SetDlgItemText(hDlg, IDC_BTN_TEST_OUTPUT, L"测试");
                KillTimer(hDlg, 2);
                SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_OUTPUT), PBM_SETPOS, 0, 0);
            }
            return TRUE;
        }
        break;
    case WM_TIMER:
        if (wParam == 1 && isTestingInput) {
            float level = g_micSampler.GetLevel();
            int val = static_cast<int>(level * 150); // 放大到0~30%，不然进度条太小
            if (val > 100) val = 100;
            SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_INPUT), PBM_SETPOS, val, 0);
        }
        return TRUE;
    case WM_DESTROY:
    case WM_CLOSE:
        KillTimer(hDlg, 1);
        KillTimer(hDlg, 2);
        break;
    }
    return FALSE;
}

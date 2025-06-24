#include "about_page.h"
#include "resource.h"

// The dialog procedure for the "About" page.
INT_PTR CALLBACK AboutPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {
        // Define the full text content for the "About" page.
        // Using \r\n for new lines is standard for multiline edit controls.
        const wchar_t* aboutText =
            L"wecall-recorder – 自动拾音\r\n"
            L"版本：1.3.0  (2025-06-24)\r\n\r\n"
            L"主要功能：\r\n"
            L"• 实时检测麦克风使用情况并自动开始/停止录音\r\n"
            L"• 同步捕获「麦克风 + 扬声器」双通道，自动混音输出 MP3\r\n"
            L"• 设备自由选择、独立增益调节，可视化电平监测\r\n"
            L"• 进程黑名单：名单中的程序占用麦克风时不触发录音\r\n"
            L"• 自定义保存路径，文件按日期归档，命名含时间戳\r\n"
            L"• 开机自启动 & 托盘驻留，后台静默运行\r\n"
            L"• 完整日志输出，便于排查 FFmpeg / WASAPI 错误\r\n\r\n"
            L"技术栈：\r\n"
            L"C++17 · Win32 API · WASAPI · FFmpeg · libmp3lame\r\n\r\n"
            L"使用须知：\r\n"
            L"本工具仅供个人学习、会议记录或备份本人参与的通话，\r\n"
            L"请在录音前取得所有参与者同意，勿作任何侵犯隐私的用途。\r\n\r\n"
            L"作者：夜莺";

        // Programmatically set the text of the EDITTEXT control.
        SetDlgItemTextW(hDlg, IDC_STATIC_ABOUT, aboutText);
        return TRUE;
    }
    }
    return FALSE;
}

#include "custom_button.h"
#include "resource.h"
#include <map>

struct ButtonStyle {
    LPCWSTR text;
    COLORREF background;
    COLORREF backgroundPressed;
    COLORREF border;
    COLORREF textColor;
};

static std::map<int, ButtonStyle> g_buttonStyles = {
    { IDC_BTN_START,       { L"开始录音",   RGB(45,50,65), RGB(35,40,55), RGB(70,70,90), RGB(235,235,235) } },
    { IDC_BTN_SAVE_PATH,   { L"保存路径",   RGB(45,50,65), RGB(35,40,55), RGB(70,70,90), RGB(235,235,235) } },
    { IDC_BTN_BROWSE,      { L"浏览...",    RGB(45,50,65), RGB(35,40,55), RGB(70,70,90), RGB(235,235,235) } },
    { IDC_BTN_TEST_INPUT,  { L"测试输入",   RGB(45,50,65), RGB(35,40,55), RGB(70,70,90), RGB(235,235,235) } },
    { IDC_BTN_TEST_OUTPUT, { L"测试输出",   RGB(45,50,65), RGB(35,40,55), RGB(70,70,90), RGB(235,235,235) } },
    { IDC_BTN_DEVICE,      { L"设备",       RGB(45,50,65), RGB(35,40,55), RGB(70,70,90), RGB(235,235,235) } },
    { IDC_BTN_PATH,        { L"路径",       RGB(45,50,65), RGB(35,40,55), RGB(70,70,90), RGB(235,235,235) } },
    { IDC_BTN_ABOUT,       { L"关于",       RGB(45,50,65), RGB(35,40,55), RGB(70,70,90), RGB(235,235,235) } },
};

bool HandleMacStyleButtonDraw(LPDRAWITEMSTRUCT pDIS, LPCWSTR text, COLORREF bg, COLORREF border, COLORREF textColor) {
    HDC hdc = pDIS->hDC;
    RECT rc = pDIS->rcItem;
    bool isPressed = (pDIS->itemState & ODS_SELECTED);

    HBRUSH hBrush = CreateSolidBrush(isPressed ? RGB(35, 40, 55) : bg);
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);

    HPEN hPen = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ oldPen = SelectObject(hdc, hPen);
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 6, 6);
    SelectObject(hdc, oldPen);
    DeleteObject(hPen);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, textColor);
    DrawTextW(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    return true;
}

bool HandleMacStyleButtonDrawAuto(LPDRAWITEMSTRUCT pDIS, int id) {
    auto it = g_buttonStyles.find(id);
    if (it != g_buttonStyles.end()) {
        const ButtonStyle& style = it->second;
        return HandleMacStyleButtonDraw(pDIS, style.text,
            (pDIS->itemState & ODS_SELECTED) ? style.backgroundPressed : style.background,
            style.border,
            style.textColor);
    }
    return false;
}

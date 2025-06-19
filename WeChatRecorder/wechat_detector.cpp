#include <windows.h>
#include "wechat_detector.h"

// ÅÐ¶Ï AudioWnd »ò VoipWnd ÊÇ·ñ´æÔÚ
bool IsWeChatCalling() {
    return FindWindowW(L"AudioWnd", NULL) != NULL || FindWindowW(L"VoipWnd", NULL) != NULL;
}

#include <windows.h>
#include "wechat_detector.h"

// �ж� AudioWnd �� VoipWnd �Ƿ����
bool IsWeChatCalling() {
    return FindWindowW(L"AudioWnd", NULL) != NULL || FindWindowW(L"VoipWnd", NULL) != NULL;
}

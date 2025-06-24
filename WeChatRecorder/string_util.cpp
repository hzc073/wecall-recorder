#include "string_util.h"
#include <Windows.h>

std::wstring MultiByteToWide(const std::string& src)
{
    if (src.empty()) return L"";

    auto convert = [&](UINT cp) -> std::wstring
        {
            int len = MultiByteToWideChar(
                cp,
                cp == CP_UTF8 ? MB_ERR_INVALID_CHARS : 0,
                src.c_str(),
                -1,
                nullptr,
                0);
            if (len == 0) return L"";

            std::wstring w(len, L'\0');
            MultiByteToWideChar(cp, 0, src.c_str(), -1, &w[0], len);
            if (!w.empty() && w.back() == L'\0') w.pop_back();
            return w;
        };

    // --- 核心修改点：简化逻辑，不再检查特殊字符 ---
    std::wstring w = convert(CP_UTF8);
    // 如果UTF-8转换成功（结果非空），就直接返回
    if (!w.empty()) {
        return w;
    }

    // 如果UTF-8转换失败，则回退到系统默认编码
    w = convert(CP_ACP);

    // 如果仍然失败，返回一个占位符
    return w.empty() ? L"[未知设备]" : w;
}
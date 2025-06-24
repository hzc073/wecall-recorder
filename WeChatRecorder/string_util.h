#pragma once
#include <string>

// 将多字节字符串（可能是 UTF-8，也可能是系统 ANSI）转换成 UTF-16 宽字符串。
// 内部先尝试 UTF-8；若失败或出现大量替换字符，再回退 CP_ACP。
std::wstring MultiByteToWide(const std::string& src);
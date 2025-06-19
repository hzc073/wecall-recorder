#pragma once
#include <string>

// 生成完整的录音保存路径
std::wstring GenerateSavePath(const std::wstring& root, const std::wstring& appName);

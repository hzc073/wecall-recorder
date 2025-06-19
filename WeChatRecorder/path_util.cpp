#include "path_util.h"
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>

std::wstring GenerateSavePath(const std::wstring& root, const std::wstring& appName) {
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow;
    localtime_s(&tmNow, &now_c);

    wchar_t timeBuf[32];
    wcsftime(timeBuf, 32, L"%Y%m%d%H%M%S", &tmNow);

    std::wstring folderPath = root + L"\\" + appName;
    std::filesystem::create_directories(folderPath);

    std::wstringstream filename;
    filename << folderPath << L"\\" << timeBuf << L".wav";
    return filename.str();
}

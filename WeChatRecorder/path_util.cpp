#include "path_util.h"
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>

std::wstring GenerateSavePath(const std::wstring& root, const std::wstring& appName) {
    // Get current time
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow;
    localtime_s(&tmNow, &now_c);

    wchar_t timeBuf[32];
    wcsftime(timeBuf, 32, L"%Y%m%d_%H%M%S", &tmNow);

    // Create app folder
    std::wstring folderPath = root + L"\\" + appName;
    std::filesystem::create_directories(folderPath);

    // Generate filename
    std::wstringstream filename;
    filename << folderPath << L"\\" << appName << L"_" << timeBuf << L".mp3";
    return filename.str();
}

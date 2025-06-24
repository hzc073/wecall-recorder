#pragma once
#include <vector>
#include <string>

// 统一使用 std::wstring，后续不再反复转码
struct FFmpegDeviceInfo {
    std::wstring id;   // FFmpeg 真正要用的 device_name
    std::wstring name; // 友好描述（device_description）
};

// 麦克风 / 立体声混音…
std::vector<FFmpegDeviceInfo> FFmpegEnumInputDevices();
// 扬声器环回 / WASAPI-loopback…（可选）
std::vector<FFmpegDeviceInfo> FFmpegEnumOutputDevices();

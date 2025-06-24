// 文件名：device_enum.h
#pragma once
#include <vector>
#include <string>
#include "ffmpeg_device_enum.h" // 包含此文件以使用 FFmpegDeviceInfo 类型

// 声明函数，使其返回统一的类型
std::vector<FFmpegDeviceInfo> EnumInputDevices();
std::vector<FFmpegDeviceInfo> EnumOutputDevices();
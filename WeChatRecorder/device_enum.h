#pragma once
#include <vector>
#include <string>
// device_enum.h


// 设备信息结构体
struct DeviceInfo {
    std::wstring id;
    std::wstring name;
};

// 枚举所有输入设备
std::vector<DeviceInfo> EnumInputDevices();
// 枚举所有输出设备
std::vector<DeviceInfo> EnumOutputDevices();

float GetMicPeakLevel();  //麦克风
float GetSpeakerPeakLevel();//扬声器


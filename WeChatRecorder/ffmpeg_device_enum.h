#pragma once
#include <vector>
#include <string>

// ͳһʹ�� std::wstring���������ٷ���ת��
struct FFmpegDeviceInfo {
    std::wstring id;   // FFmpeg ����Ҫ�õ� device_name
    std::wstring name; // �Ѻ�������device_description��
};

// ��˷� / ������������
std::vector<FFmpegDeviceInfo> FFmpegEnumInputDevices();
// ���������� / WASAPI-loopback������ѡ��
std::vector<FFmpegDeviceInfo> FFmpegEnumOutputDevices();

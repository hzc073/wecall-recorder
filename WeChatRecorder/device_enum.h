// �ļ�����device_enum.h
#pragma once
#include <vector>
#include <string>
#include "ffmpeg_device_enum.h" // �������ļ���ʹ�� FFmpegDeviceInfo ����

// ����������ʹ�䷵��ͳһ������
std::vector<FFmpegDeviceInfo> EnumInputDevices();
std::vector<FFmpegDeviceInfo> EnumOutputDevices();
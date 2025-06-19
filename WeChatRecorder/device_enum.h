#pragma once
#include <vector>
#include <string>
// device_enum.h


// �豸��Ϣ�ṹ��
struct DeviceInfo {
    std::wstring id;
    std::wstring name;
};

// ö�����������豸
std::vector<DeviceInfo> EnumInputDevices();
// ö����������豸
std::vector<DeviceInfo> EnumOutputDevices();

float GetMicPeakLevel();  //��˷�
float GetSpeakerPeakLevel();//������


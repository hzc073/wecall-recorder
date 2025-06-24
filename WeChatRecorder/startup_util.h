#pragma once
#include <windows.h>
#include <string>

// 设置或取消开机启动
void SetStartupStatus(bool enable);

// 获取当前是否为开机启动
bool GetStartupStatus();
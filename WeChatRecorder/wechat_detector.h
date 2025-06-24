#pragma once
#include <string>

// 检查麦克风是否被占用，并返回第一个占用进程名
// 返回 true: 有占用，同时 processName 为占用进程名
// 返回 false: 没有被占用
bool IsMicInUse(std::wstring& processName);

#pragma once
#include <string>

// �����˷��Ƿ�ռ�ã������ص�һ��ռ�ý�����
// ���� true: ��ռ�ã�ͬʱ processName Ϊռ�ý�����
// ���� false: û�б�ռ��
bool IsMicInUse(std::wstring& processName);

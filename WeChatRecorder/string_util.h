#pragma once
#include <string>

// �����ֽ��ַ����������� UTF-8��Ҳ������ϵͳ ANSI��ת���� UTF-16 ���ַ�����
// �ڲ��ȳ��� UTF-8����ʧ�ܻ���ִ����滻�ַ����ٻ��� CP_ACP��
std::wstring MultiByteToWide(const std::string& src);
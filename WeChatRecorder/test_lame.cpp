#include "lame.h"
#include <iostream>

int main() {
    lame_t lame = lame_init();
    if (lame) {
        std::cout << "lame ��ʼ���ɹ���" << std::endl;
        lame_close(lame);
    }
    else {
        std::cout << "lame ��ʼ��ʧ�ܣ�" << std::endl;
    }
    return 0;
}
#include "lame.h"
#include <iostream>

int main() {
    lame_t lame = lame_init();
    if (lame) {
        std::cout << "lame 初始化成功！" << std::endl;
        lame_close(lame);
    }
    else {
        std::cout << "lame 初始化失败！" << std::endl;
    }
    return 0;
}
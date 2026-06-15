#ifndef 公共_H
#define 公共_H

#include <iostream>
#include <string>

// 调试开关（由命令行参数控制，默认关闭）
extern bool 调试模式;

// 调试打印宏（仅在调试模式开启时生效）
#define 调试打印(...) \
    do { \
        if (调试模式) { \
            std::cout << "[调试] " << __VA_ARGS__ << std::endl; \
        } \
    } while (0)

#endif
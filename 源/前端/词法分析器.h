#ifndef 词法分析器_H
#define 词法分析器_H

#include <string>
#include <cstdint>
#include "公共.h"

enum class 标记类型 {
    未知,
    文件结束,
    整数,
    浮点,
    字符,
    字符串,
    字符串插值,
    标识符,
    加号,
    减号,
    乘号,
    除号,
    百分号,
    加等于,
    减等于,
    乘等于,
    除等于,
    模等于,
    自增,
    自减,
    等于,
    等于等于,
    感叹号等于,
    小于,
    大于,
    小于等于,
    大于等于,
    左移,
    右移,
    位与,
    位或,
    位异或,
    位与等于,
    位或等于,
    位异或等于,
    左移等于,
    右移等于,
    与与,
    或或,
    感叹号,
    左括号,
    右括号,
    左花括号,
    右花括号,
    左方括号,
    右方括号,
    分号,
    逗号,
    冒号,
    句号,
    省略号,
    箭头,
    函数,
    空,
    如果,
    否则,
    否则如果,
    循环,
    当,
    做,
    到,
    中断,
    继续,
    变量,
    常量,
    打印,
    返回,
    匹配,
    遍历,
    导入,
    整数类型,
    浮点类型,
    布尔类型,
    字符串类型,
    结构体,
    枚举,
    真,
    假,
    问号,
    问问,
    入
};

struct 标记 {
    标记类型 类型;
    std::string 值;
    int 行号;
    标记(标记类型 t, std::string v, int ln) : 类型(t), 值(v), 行号(ln) {}
};

std::string 标记类型转字符串(标记类型 类型);

class 词法分析器 {
private:
    std::string 源代码;
    size_t 位置 = 0;
    int 当前行 = 1;

    char 字节(size_t 偏移 = 0) const;
    void 前进字节(size_t 步长);
    bool 是空白字节(uint8_t 字节) const;
    bool 是全角空格() const;
    void 跳过空白和注释();
    int 多字节长度(uint8_t 首字节) const;
    bool 验证后续字节(size_t 总长度) const;
    std::string 读取多字节字符();
    bool 是多字节首字节(uint8_t 字节) const { return (字节 & 0xC0) == 0xC0; }

public:
    词法分析器(const std::string& 文件名);
    词法分析器(const char* 源码, size_t 长度) : 位置(0), 当前行(1) {
        源代码.assign(源码, 长度);
    }
    bool 已到文件尾() const { return 位置 >= 源代码.size(); }
    标记 下一个标记();
    size_t 获取位置() const { return 位置; }
    void 设置位置(size_t 新位置) { 位置 = 新位置; }
    int 获取当前行() const { return 当前行; }
    void 设置当前行(int 行) { 当前行 = 行; }
};

#endif // 词法分析器_H
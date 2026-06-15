#ifndef 字节码编译器_H
#define 字节码编译器_H

#include "抽象语法树.h"
#include "字节码.h"
#include <unordered_map>
#include <stack>

class 字节码编译器 {
    字节码 结果;
    int 当前函数索引 = -1;
    函数信息* 当前函数 = nullptr;
    uint8_t 下一个寄存器 = 0;
    uint8_t 最大寄存器 = 0;
    std::unordered_map<std::string, uint8_t> 变量映射;
    std::unordered_map<std::string, bool> 浮点变量映射;
    std::unordered_map<std::string, bool> 字符串变量映射;
    std::unordered_map<std::string, bool> 结构体变量映射;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> 结构体定义映射;
    std::unordered_map<std::string, bool> 闭包变量映射;
    std::vector<std::unordered_map<std::string, uint8_t>> 作用域栈;

    struct 循环上下文 {
        size_t 继续目标;
        std::vector<size_t> 中断跳转;
    };
    std::vector<循环上下文> 循环栈;

    uint8_t 分配寄存器() {
        uint8_t reg = 下一个寄存器++;
        if (下一个寄存器 > 最大寄存器) 最大寄存器 = 下一个寄存器;
        return reg;
    }

    void 释放寄存器() {
        if (下一个寄存器 > 0) 下一个寄存器--;
    }

    void 进入作用域() { 作用域栈.push_back({}); }
    void 退出作用域() {
        if (!作用域栈.empty()) {
            for (const auto& [名, reg] : 作用域栈.back()) {
                变量映射.erase(名);
            }
            作用域栈.pop_back();
        }
    }

    uint8_t 声明变量(const std::string& 名称) {
        uint8_t reg = 分配寄存器();
        变量映射[名称] = reg;
        if (!作用域栈.empty()) 作用域栈.back()[名称] = reg;
        return reg;
    }

    void 设置浮点变量(const std::string& 名称) {
        浮点变量映射[名称] = true;
    }

    void 设置字符串变量(const std::string& 名称) {
        字符串变量映射[名称] = true;
    }

    bool 是浮点变量(const std::string& 名称) const {
        return 浮点变量映射.count(名称) > 0 && 浮点变量映射.at(名称);
    }

    bool 是字符串变量(const std::string& 名称) const {
        return 字符串变量映射.count(名称) > 0 && 字符串变量映射.at(名称);
    }

    int 查找变量(const std::string& 名称) {
        auto it = 变量映射.find(名称);
        return (it != 变量映射.end()) ? it->second : -1;
    }

    void 发射(指令 i) { 当前函数->指令列表.push_back(i); }
    size_t 当前位置() { return 当前函数->指令列表.size(); }
    void 回填跳转(size_t 位置, int16_t 偏移) { 当前函数->指令列表[位置].偏移 = 偏移; }

    void 编译表达式(const 表达式& 表达式, uint8_t 目标寄存器);
    void 编译语句(const 语句& 语句);
    void 编译代码块(const std::vector<std::unique_ptr<语句>>& 语句列表);
    int 编译匿名函数(const 匿名函数表达式& 匿名, const std::vector<std::string>& 捕获变量名);

public:
    字节码 编译(const 程序& 程序);
};

#endif

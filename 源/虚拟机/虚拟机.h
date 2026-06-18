#ifndef 虚拟机_H
#define 虚拟机_H

#include "字节码.h"
#include <vector>
#include <stack>
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

// 运行时函数声明
extern "C" {
    int 获取参数数量();
    const char* 获取参数(int 索引);
    void 输出错误(const char* 消息);
    void 输出错误值(int 值);
    int 创建目录(const char* 路径);
    int 目录存在(const char* 路径);
    int 删除文件(const char* 路径);
    int 分割行数(const char* 文本);
    const char* 获取行(const char* 文本, int 行号);
    const char* 去除空白(const char* 文本);
    const char* 转小写(const char* 文本);
    const char* 转大写(const char* 文本);
    int 字符串开头(const char* 文本, const char* 前缀);
    int 字符串结尾(const char* 文本, const char* 后缀);
    int 字符码(const char* 文本, int 位置);
    const char* 字符(int 码点);

    // 动态数组
    typedef struct { int* 数据; int 大小; int 容量; } 动态数组;
    动态数组* 创建动态数组();
    void 动态数组添加(动态数组* arr, int 值);
    int 动态数组获取(动态数组* arr, int 索引);
    void 动态数组设置(动态数组* arr, int 索引, int 值);
    int 动态数组大小(动态数组* arr);
    void 动态数组删除(动态数组* arr, int 索引);
    void 动态数组插入(动态数组* arr, int 索引, int 值);
    void 释放动态数组(动态数组* arr);
}

class 虚拟机 {
    字节码 程序;

    struct 调用帧 {
        int 函数索引;
        size_t 返回地址;
        uint8_t 返回寄存器;
        std::vector<值> 寄存器;
        size_t 指令指针;
    };

    std::vector<值> 全局变量;
    std::stack<调用帧> 调用栈;
    调用帧* 当前帧 = nullptr;
    值 返回值;

    值& 读寄存器(uint8_t reg) { return 当前帧->寄存器[reg]; }
    void 写寄存器(uint8_t reg, 值 v) { 当前帧->寄存器[reg] = std::move(v); }

    const 指令& 当前指令() {
        return 程序.函数表[当前帧->函数索引].指令列表[当前帧->指令指针];
    }

    void 调用函数(int 函数索引, uint8_t 参数数量, uint8_t 返回寄存器) {
        const auto& 函数信息 = 程序.函数表[函数索引];

        调用帧 新帧;
        新帧.函数索引 = 函数索引;
        新帧.返回寄存器 = 返回寄存器;
        新帧.指令指针 = 0;
        新帧.寄存器.resize(函数信息.最大寄存器);

        for (int i = 0; i < 参数数量; i++) {
            新帧.寄存器[i] = 读寄存器(static_cast<uint8_t>(i));
        }

        调用栈.push(std::move(新帧));
        当前帧 = &调用栈.top();
    }

    bool 是真(const 值& v) {
        if (v.类型 == 值::整数) return v.整数值 != 0;
        if (v.类型 == 值::浮点数) return v.浮点值 != 0.0;
        if (v.类型 == 值::字符串) return !v.字符串值.empty();
        return false;
    }

    int32_t 转整数(const 值& v) {
        if (v.类型 == 值::整数) return v.整数值;
        if (v.类型 == 值::浮点数) return static_cast<int32_t>(v.浮点值);
        return 0;
    }

    double 转浮点(const 值& v) {
        if (v.类型 == 值::浮点数) return v.浮点值;
        if (v.类型 == 值::整数) return static_cast<double>(v.整数值);
        return 0.0;
    }

    void 执行当前帧();

public:
    void 加载(const 字节码& 码) { 程序 = 码; }
    void 加载文件(const std::string& 文件名) { 程序 = 字节码::读取(文件名); }

    int 执行() {
        if (程序.入口函数索引 < 0) throw std::runtime_error("无入口函数");

        全局变量.resize(256);

        int 初始化索引 = -1;
        for (size_t i = 0; i < 程序.函数表.size(); i++) {
            if (程序.函数表[i].名称 == "__初始化全局变量") {
                初始化索引 = static_cast<int>(i);
                break;
            }
        }

        if (初始化索引 >= 0) {
            调用帧 初始化帧;
            初始化帧.函数索引 = 初始化索引;
            初始化帧.指令指针 = 0;
            初始化帧.寄存器.resize(程序.函数表[初始化索引].最大寄存器);
            调用栈.push(std::move(初始化帧));
            当前帧 = &调用栈.top();
            执行当前帧();
            调用栈.pop();
        }

        const auto& 入口函数 = 程序.函数表[程序.入口函数索引];
        调用帧 入口帧;
        入口帧.函数索引 = 程序.入口函数索引;
        入口帧.指令指针 = 0;
        入口帧.寄存器.resize(入口函数.最大寄存器);
        调用栈.push(std::move(入口帧));
        当前帧 = &调用栈.top();
        执行当前帧();

        return 转整数(返回值);
    }
};

#endif

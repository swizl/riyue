#ifndef 虚拟机_H
#define 虚拟机_H

#include "字节码.h"
#include <vector>
#include <stack>
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <deque>
#include <condition_variable>
#include <mutex>

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
    const char* 文本转小写(const char* 文本);
    const char* 文本转大写(const char* 文本);
    int 字符串开头(const char* 文本, const char* 前缀);
    int 字符串结尾(const char* 文本, const char* 后缀);
    int 字符码(const char* 文本, int 位置);
    const char* 字符序列(int 码点);

    // 动态数组
    typedef struct { int* 数据; int 数组大小; int 容量; } 动态数组;
    动态数组* 创建动态数组();
    void 动态数组添加(动态数组* arr, int 值);
    int 动态数组获取(动态数组* arr, int 索引);
    void 动态数组设置(动态数组* arr, int 索引, int 值);
    int 动态数组大小(动态数组* arr);
    void 动态数组删除(动态数组* arr, int 索引);
    void 动态数组插入(动态数组* arr, int 索引, int 值);
    void 释放动态数组(动态数组* arr);

    // 目录操作
    int 列出目录(const char* 路径);
    const char* 获取目录项名称(int 索引);
    int 获取目录项是否目录(int 索引);
    int64_t 获取目录项大小(int 索引);
    int 递归遍历目录(const char* 路径, int 深度);
}

struct 调用帧 {
    int 函数索引;
    size_t 返回地址 = 0;
    uint8_t 返回寄存器组 = 0;
    std::vector<值> 寄存器组;
    size_t 指令指针 = 0;
};

struct 协程状态 {
    调用帧 帧;
    enum 状态 { 运行中, 已暂停, 已完成 } 当前状态 = 已暂停;
    值 最后让出值;
    bool 有异常 = false;
    值 异常值;
};

struct 通道结构 {
    std::deque<值> 缓冲区;
    int 容量;
    bool 已关闭 = false;
    std::mutex 互斥锁;
    std::condition_variable 有数据;
    std::condition_variable 有空间;

    通道结构(int cap) : 容量(cap) {}
};

class 虚拟机 {
    字节码 程序;

    std::vector<值> 全局变量;
    std::stack<调用帧> 调用栈;
    调用帧* 当前帧 = nullptr;
    值 返回值;

    struct 异常处理器 {
        size_t catch地址;
        size_t 帧指针;
    };
    std::vector<异常处理器> 异常处理栈;

    值& 读寄存器(uint8_t reg) { return 当前帧->寄存器组[reg]; }
    void 写寄存器(uint8_t reg, 值 v) { 当前帧->寄存器组[reg] = std::move(v); }

    const 指令& 当前指令() {
        return 程序.函数表[当前帧->函数索引].指令列表[当前帧->指令指针];
    }

    void 调用函数(int 函数索引, uint8_t 参数数量, uint8_t 返回寄存器组) {
        const auto& 函数信息 = 程序.函数表[函数索引];

        调用帧 新帧;
        新帧.函数索引 = 函数索引;
        新帧.返回寄存器组 = 返回寄存器组;
        新帧.指令指针 = 0;
        新帧.寄存器组.resize(函数信息.最大寄存器);

        for (int i = 0; i < 参数数量; i++) {
            新帧.寄存器组[i] = 读寄存器(static_cast<uint8_t>(i));
        }

        调用栈.push(std::move(新帧));
        当前帧 = &调用栈.top();
    }

    bool 是真(const 值& v) {
        if (v.类型 == 值::值类型_整数) return v.整数值 != 0;
        if (v.类型 == 值::值类型_浮点数) return v.浮点值 != 0.0;
        if (v.类型 == 值::字符串) return !v.字符串值.empty();
        return false;
    }

    int32_t 转整数(const 值& v) {
        if (v.类型 == 值::值类型_整数) return v.整数值;
        if (v.类型 == 值::值类型_浮点数) return static_cast<int32_t>(v.浮点值);
        return 0;
    }

    double 转浮点(const 值& v) {
        if (v.类型 == 值::值类型_浮点数) return v.浮点值;
        if (v.类型 == 值::值类型_整数) return static_cast<double>(v.整数值);
        return 0.0;
    }

    void 执行当前帧(size_t 目标栈深度);

public:
    void 加载(const 字节码& 码) { 程序 = 码; }
    void 加载文件(const std::string& 文件名) { 程序 = 字节码::读取(文件名); }

    int 执行();
};

#endif

#include "字节码.h"
#include <fstream>
#include <iostream>

std::string 操作码名称(操作码 码) {
    switch (码) {
        case 操作码::停止: return "停止";
        case 操作码::加载常量: return "加载常量";
        case 操作码::加载变量: return "加载变量";
        case 操作码::存储变量: return "存储变量";
        case 操作码::复制寄存器: return "复制寄存器";
        case 操作码::加法: return "加法";
        case 操作码::减法: return "减法";
        case 操作码::乘法: return "乘法";
        case 操作码::除法: return "除法";
        case 操作码::取模: return "取模";
        case 操作码::等于: return "等于";
        case 操作码::不等于: return "不等于";
        case 操作码::小于: return "小于";
        case 操作码::大于: return "大于";
        case 操作码::小于等于: return "小于等于";
        case 操作码::大于等于: return "大于等于";
        case 操作码::左移: return "左移";
        case 操作码::右移: return "右移";
        case 操作码::位与: return "位与";
        case 操作码::位或: return "位或";
        case 操作码::位异或: return "位异或";
        case 操作码::逻辑与: return "逻辑与";
        case 操作码::逻辑或: return "逻辑或";
        case 操作码::逻辑非: return "逻辑非";
        case 操作码::取负: return "取负";
        case 操作码::浮点加法: return "浮点加法";
        case 操作码::浮点减法: return "浮点减法";
        case 操作码::浮点乘法: return "浮点乘法";
        case 操作码::浮点除法: return "浮点除法";
        case 操作码::浮点等于: return "浮点等于";
        case 操作码::浮点不等于: return "浮点不等于";
        case 操作码::浮点小于: return "浮点小于";
        case 操作码::浮点大于: return "浮点大于";
        case 操作码::浮点小于等于: return "浮点小于等于";
        case 操作码::浮点大于等于: return "浮点大于等于";
        case 操作码::浮点取负: return "浮点取负";
        case 操作码::整数转浮点: return "整数转浮点";
        case 操作码::浮点转整数: return "浮点转整数";
        case 操作码::跳转: return "跳转";
        case 操作码::假跳转: return "假跳转";
        case 操作码::调用: return "调用";
        case 操作码::返回值: return "返回值";
        case 操作码::返回空: return "返回空";
        case 操作码::打印: return "打印";
        case 操作码::打印浮点: return "打印浮点";
        case 操作码::打印字符串: return "打印字符串";
        case 操作码::加载字符串: return "加载字符串";
        case 操作码::字符串拼接: return "字符串拼接";
        case 操作码::字符串长度: return "字符串长度";
        case 操作码::字符串查找: return "字符串查找";
        case 操作码::创建数组: return "创建数组";
        case 操作码::加载数组元素: return "加载数组元素";
        case 操作码::存储数组元素: return "存储数组元素";
        case 操作码::绝对值运算: return "绝对值运算";
        case 操作码::最小值运算: return "最小值运算";
        case 操作码::最大值运算: return "最大值运算";
        case 操作码::创建闭包: return "创建闭包";
        case 操作码::调用闭包: return "调用闭包";
        case 操作码::创建结构体: return "创建结构体";
        case 操作码::存储成员: return "存储成员";
        case 操作码::加载成员: return "加载成员";
        case 操作码::列出目录运算: return "列出目录运算";
        case 操作码::获取目录项名称运算: return "获取目录项名称运算";
        case 操作码::获取目录项是否目录运算: return "获取目录项是否目录运算";
        case 操作码::获取目录项大小运算: return "获取目录项大小运算";
        case 操作码::递归遍历目录运算: return "递归遍历目录运算";
        case 操作码::设置异常处理: return "设置异常处理";
        case 操作码::清除异常处理: return "清除异常处理";
        case 操作码::抛出异常: return "抛出异常";
        case 操作码::创建协程: return "创建协程";
        case 操作码::让出协程: return "让出协程";
        case 操作码::恢复协程: return "恢复协程";
        case 操作码::创建通道运算: return "创建通道运算";
        case 操作码::发送通道运算: return "发送通道运算";
        case 操作码::接收通道运算: return "接收通道运算";
        case 操作码::关闭通道运算: return "关闭通道运算";
        default: return "未知";
    }
}

void 字节码::输出(const std::string& 文件名) const {
    std::ofstream 文件(文件名, std::ios::binary);
    if (!文件.is_open()) {
        throw std::runtime_error("无法写入字节码文件: " + 文件名);
    }

    // 魔数
    uint32_t 魔数 = 0x52595545; // "RIUE"
    文件.write(reinterpret_cast<const char*>(&魔数), 4);

    // 常量池
    uint32_t 常量数量 = static_cast<uint32_t>(常量池.size());
    文件.write(reinterpret_cast<const char*>(&常量数量), 4);
    for (auto 值 : 常量池) {
        文件.write(reinterpret_cast<const char*>(&值), 4);
    }

    // 函数表
    uint32_t 函数数量 = static_cast<uint32_t>(函数表.size());
    文件.write(reinterpret_cast<const char*>(&函数数量), 4);

    for (const auto& 函数 : 函数表) {
        // 函数名
        uint32_t 名称长度 = static_cast<uint32_t>(函数.名称.size());
        文件.write(reinterpret_cast<const char*>(&名称长度), 4);
        文件.write(函数.名称.data(), 名称长度);

        // 参数数量、最大寄存器
        文件.write(reinterpret_cast<const char*>(&函数.参数数量), 1);
        文件.write(reinterpret_cast<const char*>(&函数.最大寄存器), 1);

        // 指令
        uint32_t 指令数量 = static_cast<uint32_t>(函数.指令列表.size());
        文件.write(reinterpret_cast<const char*>(&指令数量), 4);
        for (const auto& 指令 : 函数.指令列表) {
            uint8_t 码 = static_cast<uint8_t>(指令.码);
            文件.write(reinterpret_cast<const char*>(&码), 1);
            文件.write(reinterpret_cast<const char*>(&指令.A), 1);
            文件.write(reinterpret_cast<const char*>(&指令.B), 1);
            文件.write(reinterpret_cast<const char*>(&指令.C), 1);
            文件.write(reinterpret_cast<const char*>(&指令.偏移), 2);
        }
    }

    // 入口函数索引
    文件.write(reinterpret_cast<const char*>(&入口函数索引), 4);
    文件.close();
}

字节码 字节码::读取(const std::string& 文件名) {
    std::ifstream 文件(文件名, std::ios::binary);
    if (!文件.is_open()) {
        throw std::runtime_error("无法读取字节码文件: " + 文件名);
    }

    字节码 结果;

    // 魔数
    uint32_t 魔数;
    文件.read(reinterpret_cast<char*>(&魔数), 4);
    if (魔数 != 0x52595545) {
        throw std::runtime_error("无效的字节码文件格式");
    }

    // 常量池
    uint32_t 常量数量;
    文件.read(reinterpret_cast<char*>(&常量数量), 4);
    结果.常量池.resize(常量数量);
    for (uint32_t i = 0; i < 常量数量; i++) {
        文件.read(reinterpret_cast<char*>(&结果.常量池[i]), 4);
    }

    // 函数表
    uint32_t 函数数量;
    文件.read(reinterpret_cast<char*>(&函数数量), 4);
    结果.函数表.resize(函数数量);

    for (uint32_t f = 0; f < 函数数量; f++) {
        auto& 函数 = 结果.函数表[f];

        uint32_t 名称长度;
        文件.read(reinterpret_cast<char*>(&名称长度), 4);
        函数.名称.resize(名称长度);
        文件.read(&函数.名称[0], 名称长度);

        文件.read(reinterpret_cast<char*>(&函数.参数数量), 1);
        文件.read(reinterpret_cast<char*>(&函数.最大寄存器), 1);

        uint32_t 指令数量;
        文件.read(reinterpret_cast<char*>(&指令数量), 4);
        函数.指令列表.resize(指令数量);
        for (uint32_t i = 0; i < 指令数量; i++) {
            uint8_t 码;
            文件.read(reinterpret_cast<char*>(&码), 1);
            函数.指令列表[i].码 = static_cast<操作码>(码);
            文件.read(reinterpret_cast<char*>(&函数.指令列表[i].A), 1);
            文件.read(reinterpret_cast<char*>(&函数.指令列表[i].B), 1);
            文件.read(reinterpret_cast<char*>(&函数.指令列表[i].C), 1);
            文件.read(reinterpret_cast<char*>(&函数.指令列表[i].偏移), 2);
        }
    }

    文件.read(reinterpret_cast<char*>(&结果.入口函数索引), 4);
    文件.close();
    return 结果;
}

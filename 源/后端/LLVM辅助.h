#ifndef LLVM辅助_H
#define LLVM辅助_H

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include <string>
#include "../共享/LLVM中文.h"

using namespace llvm中文;

LLVM值* 确保浮点(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, LLVM值* 值);

LLVM值* 拼接字符串(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    LLVM值* 左, LLVM值* 右);

LLVM值* 读取行并剥离换行(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    LLVM值* 文件指针, int 缓冲大小 = 4096);

LLVM值* 读取标准输入行(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    int 缓冲大小 = 4096);

LLVM值* 调用外部函数0(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    const std::string& 名称, 类型* 返回类型);

LLVM值* 调用数学函数1(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    const std::string& C名称, const std::string& 显示名称, LLVM值* 参数);

LLVM值* 调用数学函数2(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    const std::string& C名称, const std::string& 显示名称, LLVM值* 参数1, LLVM值* 参数2);

LLVM值* 生成极值选择(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    LLVM值* 左, LLVM值* 右, bool 是最小);

// 短路逻辑运算（逻辑与/逻辑或）
// 是与操作: true = AND, false = OR
LLVM值* 短路逻辑运算(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    LLVM值* 左值, std::function<LLVM值*()> 生成右值, bool 是与操作);

// 查找结构体成员地址
LLVM值* 查找结构体成员地址(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    const std::string& 对象名, const std::string& 成员名,
    class 符号表& 符号表实例,
    std::unordered_map<std::string, LLVM结构体类型*>& 结构体类型映射,
    std::unordered_map<std::string, std::vector<std::string>>& 结构体成员映射);

template<typename 返回值列表类型>
类型* 计算返回类型(llvm::LLVMContext& 上下文, const 返回值列表类型& 返回值列表,
    std::function<类型*(const std::string&)> 类型转换) {
    类型* 返回LLVM类型 = 获取空类型(上下文);
    if (返回值列表.size() == 1 && !返回值列表[0].类型.empty() && 返回值列表[0].类型 != "空") {
        返回LLVM类型 = 类型转换(返回值列表[0].类型);
    } else if (返回值列表.size() > 1) {
        std::vector<类型*> 返回类型列表;
        for (const auto& 返回值 : 返回值列表) {
            if (!返回值.类型.empty() && 返回值.类型 != "空") {
                返回类型列表.push_back(类型转换(返回值.类型));
            }
        }
        if (!返回类型列表.empty()) {
            返回LLVM类型 = llvm::StructType::get(上下文, 返回类型列表);
        }
    }
    return 返回LLVM类型;
}

#endif
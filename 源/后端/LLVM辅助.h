#ifndef LLVM辅助_H
#define LLVM辅助_H

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include <string>

llvm::Value* 确保浮点(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Value* 值);

llvm::Value* 拼接字符串(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    llvm::Value* 左, llvm::Value* 右);

llvm::Value* 读取行并剥离换行(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    llvm::Value* 文件指针, int 缓冲大小 = 4096);

llvm::Value* 读取标准输入行(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    int 缓冲大小 = 4096);

llvm::Value* 调用外部函数0(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    const std::string& 名称, llvm::Type* 返回类型);

llvm::Value* 调用数学函数1(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    const std::string& C名称, const std::string& 显示名称, llvm::Value* 参数);

llvm::Value* 调用数学函数2(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    const std::string& C名称, const std::string& 显示名称, llvm::Value* 参数1, llvm::Value* 参数2);

llvm::Value* 生成极值选择(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    llvm::Value* 左, llvm::Value* 右, bool 是最小);

template<typename 返回值列表类型>
llvm::Type* 计算返回类型(llvm::LLVMContext& 上下文, const 返回值列表类型& 返回值列表,
    std::function<llvm::Type*(const std::string&)> 类型转换) {
    llvm::Type* 返回LLVM类型 = llvm::Type::getVoidTy(上下文);
    if (返回值列表.size() == 1 && !返回值列表[0].类型.empty() && 返回值列表[0].类型 != "空") {
        返回LLVM类型 = 类型转换(返回值列表[0].类型);
    } else if (返回值列表.size() > 1) {
        std::vector<llvm::Type*> 返回类型列表;
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

#ifndef C运行时声明_H
#define C运行时声明_H

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"

struct C运行时函数 {
    static void 预声明全部(llvm::LLVMContext& 上下文, llvm::Module& 模块) {
        llvm::Type* i8Ptr = llvm::PointerType::get(上下文, 0);
        llvm::Type* i32 = llvm::Type::getInt32Ty(上下文);
        llvm::Type* i64 = llvm::Type::getInt64Ty(上下文);
        llvm::Type* f64 = llvm::Type::getDoubleTy(上下文);

        auto 声明 = [&](const std::string& 名称, llvm::Type* 返回类型, std::vector<llvm::Type*> 参数类型, bool 变长 = false) {
            llvm::FunctionType* 类型 = llvm::FunctionType::get(返回类型, 参数类型, 变长);
            模块.getOrInsertFunction(名称, 类型);
        };

        声明("malloc", i8Ptr, {i64});
        声明("strlen", i64, {i8Ptr});
        声明("sprintf", i32, {i8Ptr, i8Ptr}, true);
        声明("printf", i32, {i8Ptr}, true);
        声明("fopen", i8Ptr, {i8Ptr, i8Ptr});
        声明("fclose", i32, {i8Ptr});
        声明("fprintf", i32, {i8Ptr, i8Ptr}, true);
        声明("fscanf", i32, {i8Ptr, i8Ptr}, true);
        声明("fgets", i8Ptr, {i8Ptr, i32, i8Ptr});
        声明("strcmp", i32, {i8Ptr, i8Ptr});
        声明("strcpy", i8Ptr, {i8Ptr, i8Ptr});
        声明("strncpy", i8Ptr, {i8Ptr, i8Ptr, i64});
        声明("strstr", i8Ptr, {i8Ptr, i8Ptr});
        声明("atoi", i32, {i8Ptr});
        声明("atof", f64, {i8Ptr});
        声明("sqrt", f64, {f64});
        声明("sin", f64, {f64});
        声明("cos", f64, {f64});
        声明("tan", f64, {f64});
        声明("pow", f64, {f64, f64});
        声明("round", f64, {f64});
        声明("fabs", f64, {f64});
        声明("log", f64, {f64});
        声明("log10", f64, {f64});
    }
};

#endif

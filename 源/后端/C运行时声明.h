#ifndef C运行时声明_H
#define C运行时声明_H

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "../共享/LLVM中文.h"

using namespace llvm中文;

struct C运行时函数 {
    static void 预声明全部(llvm::LLVMContext& 上下文, llvm::Module& 模块) {
        类型* i8Ptr = 获取指针类型(上下文);
        类型* i32 = 整型32(上下文);
        类型* i64 = 整型64(上下文);
        类型* f64 = 双精度浮点(上下文);

        auto 声明 = [&](const std::string& 名称, 类型* 返回类型, std::vector<类型*> 参数类型, bool 变长 = false) {
            函数类型* 类型 = 函数类型::get(返回类型, 参数类型, 变长);
            获取或插入函数(模块, 名称, 类型);
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

        声明("__gxx_personality_seh0", i32, {}, true);
        声明("__cxa_allocate_exception", i8Ptr, {i64});
        声明("__cxa_throw", 获取空类型(上下文), {i8Ptr, i8Ptr, i8Ptr}, false);
        声明("__cxa_begin_catch", i8Ptr, {i8Ptr});
        声明("__cxa_end_catch", 获取空类型(上下文), {});
        声明("_Unwind_Resume", 获取空类型(上下文), {i8Ptr});

        声明("创建通道函数", i8Ptr, {i32});
        声明("发送到通道函数", i32, {i8Ptr, i32});
        声明("从通道接收函数", i32, {i8Ptr});
        声明("通道是否为空", i32, {i8Ptr});
        声明("通道是否已关闭", i32, {i8Ptr});
        声明("关闭通道函数", 获取空类型(上下文), {i8Ptr});

        声明("断点命中", 获取空类型(上下文), {i8Ptr, i32, i8Ptr});
        声明("设置调试器", 获取空类型(上下文), {i32});
        声明("记录行执行", 获取空类型(上下文), {i32});
        声明("输出覆盖率报告", 获取空类型(上下文), {});
        声明("设置异常类型", 获取空类型(上下文), {i8Ptr, i32});
        声明("获取异常类型", i32, {i8Ptr});
        声明("清除异常类型", 获取空类型(上下文), {i8Ptr});
        声明("HTTP获取", i8Ptr, {i8Ptr});
        声明("写入文件", i32, {i8Ptr, i8Ptr});
        声明("读取文件", i8Ptr, {i8Ptr});
        声明("异步写入文件", i32, {i8Ptr, i8Ptr});
        声明("异步读取文件", i32, {i8Ptr});
        声明("IO是否完成", i32, {i32});
        声明("IO获取结果", i32, {i32});
        声明("IO获取读取结果", i8Ptr, {i32});
        声明("异步IO等待", i8Ptr, {i32});
        声明("等待IO完成", i32, {i32});
        声明("创建TCP客户端", i32, {});
        声明("TCP连接", i32, {i32, i8Ptr, i32});
        声明("TCP发送", i32, {i32, i8Ptr});
        声明("TCP接收", i8Ptr, {i32});
        声明("TCP关闭", 获取空类型(上下文), {i32});
        声明("创建TCP服务器", i32, {i32});
        声明("TCP接受连接", i32, {i32});
        声明("创建UDP套接字", i32, {});
        声明("UDP发送到", i32, {i32, i8Ptr, i8Ptr, i32});
        声明("注册GC对象", 获取空类型(上下文), {i8Ptr});
        声明("标记对象", 获取空类型(上下文), {i8Ptr});
        声明("执行GC", 获取空类型(上下文), {});
        声明("启用GC", 获取空类型(上下文), {});
        声明("设置GC阈值", 获取空类型(上下文), {i32});
        声明("获取GC对象数量", i32, {});
    }
};

#endif
#ifndef 代码生成器_H
#define 代码生成器_H

#include "../前端/抽象语法树.h"
#include "符号表.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Support/TargetSelect.h"
#include "../共享/LLVM中文.h"
#include <stack>
#include <unordered_map>

using namespace llvm中文;

class 代码生成器 {
    llvm::LLVMContext& 上下文;
    std::unique_ptr<llvm::Module> 模块;
    std::unique_ptr<llvm::IRBuilder<>> 构建器;
    符号表 符号表实例;
    std::string 输出文件名;
    std::stack<基本块*> 循环继续栈;
    std::stack<基本块*> 循环退出栈;
    全局变量类型* 文件表指针 = nullptr;
    LLVM数组类型* 文件表类型 = nullptr;
    全局变量类型* 文件句柄计数器 = nullptr;
    std::unordered_map<std::string, LLVM结构体类型*> 结构体类型映射;
    std::unordered_map<std::string, std::vector<std::string>> 结构体成员映射;
    std::unordered_map<std::string, std::vector<std::string>> 结构体成员类型名映射;
    std::unordered_map<std::string, LLVM结构体类型*> 元组类型映射;
    std::unordered_map<std::string, std::string> 类型别名映射;
    bool 启用优化 = false;

    基本块* 当前异常处理块 = nullptr;
    std::string 当前异常变量名;

LLVM值* 创建调用(LLVM函数* 函数, llvm::ArrayRef<LLVM值*> 参数, const std::string& 名称 = "");
LLVM值* 创建调用(函数被调用者 函数, llvm::ArrayRef<LLVM值*> 参数, const std::string& 名称 = "");
LLVM值* 创建调用无返回(LLVM函数* 函数, llvm::ArrayRef<LLVM值*> 参数);
LLVM值* 创建调用无返回(函数被调用者 函数, llvm::ArrayRef<LLVM值*> 参数);
LLVM值* 创建间接调用(llvm::FunctionType* 类型, LLVM值* 指针, llvm::ArrayRef<LLVM值*> 参数, const std::string& 名称 = "");

    类型* 类型名到LLVM类型(const std::string& 类型名);

    void 收集自由变量(const 表达式& 表达式, std::vector<std::string>& 自由变量, const std::vector<std::string>& 局部变量);
    void 收集语句自由变量(const 语句& 语句, std::vector<std::string>& 自由变量, const std::vector<std::string>& 局部变量);
    void 运行优化Pass();

public:
    代码生成器(llvm::LLVMContext& ctx, const std::string& 输出文件, bool 优化 = false);

    void 生成(const 程序& 程序);
    void 生成函数(const 函数& 函数);
    void 生成语句(const 语句& 语句);
    LLVM值* 生成表达式(const 表达式& 表达式);

    void 生成可执行文件();
};

#endif
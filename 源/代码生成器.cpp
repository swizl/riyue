#include "代码生成器.h"
#include "公共.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/LegacyPassManager.h"
#include <system_error>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
static int 执行命令(const std::string& 命令) {
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 命令.c_str(), (int)命令.size(), nullptr, 0);
    std::vector<wchar_t> 宽命令(宽长度 + 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, 命令.c_str(), (int)命令.size(), 宽命令.data(), 宽长度);
    return _wsystem(宽命令.data());
}
#else
static int 执行命令(const std::string& 命令) { return std::system(命令.c_str()); }
#endif

代码生成器::代码生成器(llvm::LLVMContext& ctx, const std::string& 输出文件)
    : 上下文(ctx), 输出文件名(输出文件) {
    模块 = std::make_unique<llvm::Module>("程序", 上下文);
    构建器 = std::make_unique<llvm::IRBuilder<>>(上下文);
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
}

llvm::Type* 代码生成器::类型名到LLVM类型(const std::string& 类型名) {
    if (类型名 == "浮点" || 类型名 == "浮点数") return llvm::Type::getDoubleTy(上下文);
    if (类型名 == "布尔" || 类型名 == "布尔值") return llvm::Type::getInt1Ty(上下文);
    if (类型名 == "字符串") return llvm::PointerType::get(上下文, 0);
    return llvm::Type::getInt32Ty(上下文);
}

static bool 在列表中(const std::string& 名称, const std::vector<std::string>& 列表) {
    for (const auto& 项 : 列表) {
        if (项 == 名称) return true;
    }
    return false;
}

void 代码生成器::收集自由变量(const 表达式& 表达式, std::vector<std::string>& 自由变量, const std::vector<std::string>& 局部变量) {
    switch (表达式.类型) {
        case 表达式类型::变量: {
            const auto& 变量 = static_cast<const 变量表达式&>(表达式);
            if (!在列表中(变量.名称, 局部变量) && !在列表中(变量.名称, 自由变量)) {
                自由变量.push_back(变量.名称);
            }
            break;
        }
        case 表达式类型::二元运算: {
            const auto& 二元 = static_cast<const 二元运算表达式&>(表达式);
            收集自由变量(*二元.左操作数, 自由变量, 局部变量);
            收集自由变量(*二元.右操作数, 自由变量, 局部变量);
            break;
        }
        case 表达式类型::一元运算: {
            const auto& 一元 = static_cast<const 一元运算表达式&>(表达式);
            收集自由变量(*一元.操作数, 自由变量, 局部变量);
            break;
        }
        case 表达式类型::函数调用: {
            const auto& 调用 = static_cast<const 函数调用表达式&>(表达式);
            for (const auto& 参数 : 调用.参数列表) 收集自由变量(*参数, 自由变量, 局部变量);
            break;
        }
        case 表达式类型::下标访问: {
            const auto& 下标 = static_cast<const 下标访问表达式&>(表达式);
            if (!在列表中(下标.数组名, 局部变量) && !在列表中(下标.数组名, 自由变量)) 自由变量.push_back(下标.数组名);
            收集自由变量(*下标.索引, 自由变量, 局部变量);
            break;
        }
        case 表达式类型::数组字面量: {
            const auto& 数组 = static_cast<const 数组字面量表达式&>(表达式);
            for (const auto& 元素 : 数组.元素列表) 收集自由变量(*元素, 自由变量, 局部变量);
            break;
        }
        case 表达式类型::字符串:
            break;
        case 表达式类型::匿名函数: {
            const auto& 匿名 = static_cast<const 匿名函数表达式&>(表达式);
            std::vector<std::string> 内部局部变量 = 局部变量;
            for (const auto& 参数 : 匿名.参数列表) 内部局部变量.push_back(参数.名称);
            for (const auto& 子 : 匿名.主体) 收集语句自由变量(*子, 自由变量, 内部局部变量);
            break;
        }
        default: break;
    }
}

void 代码生成器::收集语句自由变量(const 语句& 语句, std::vector<std::string>& 自由变量, const std::vector<std::string>& 局部变量) {
    switch (语句.类型) {
        case 语句类型::变量声明:
            收集自由变量(*static_cast<const 变量声明&>(语句).初始值, 自由变量, 局部变量);
            break;
        case 语句类型::常量声明:
            收集自由变量(*static_cast<const 常量声明&>(语句).初始值, 自由变量, 局部变量);
            break;
        case 语句类型::赋值语句: {
            const auto& 赋值 = static_cast<const 赋值语句&>(语句);
            if (!在列表中(赋值.变量名, 局部变量) && !在列表中(赋值.变量名, 自由变量)) 自由变量.push_back(赋值.变量名);
            收集自由变量(*赋值.值表达式, 自由变量, 局部变量);
            break;
        }
        case 语句类型::下标赋值语句: {
            const auto& 下标赋值 = static_cast<const 下标赋值语句&>(语句);
            if (!在列表中(下标赋值.数组名, 局部变量) && !在列表中(下标赋值.数组名, 自由变量)) 自由变量.push_back(下标赋值.数组名);
            收集自由变量(*下标赋值.索引, 自由变量, 局部变量);
            收集自由变量(*下标赋值.值表达式, 自由变量, 局部变量);
            break;
        }
        case 语句类型::打印语句:
            收集自由变量(*static_cast<const 打印语句&>(语句).值表达式, 自由变量, 局部变量);
            break;
        case 语句类型::返回语句: {
            const auto& 返回 = static_cast<const 返回语句&>(语句);
            if (返回.返回值) 收集自由变量(*返回.返回值, 自由变量, 局部变量);
            break;
        }
        case 语句类型::表达式语句:
            收集自由变量(*static_cast<const 表达式语句&>(语句).值表达式, 自由变量, 局部变量);
            break;
        case 语句类型::如果语句: {
            const auto& 如果 = static_cast<const 如果语句&>(语句);
            收集自由变量(*如果.条件, 自由变量, 局部变量);
            for (const auto& 子 : 如果.then块) 收集语句自由变量(*子, 自由变量, 局部变量);
            for (const auto& 分支 : 如果.否则如果列表) {
                收集自由变量(*分支.条件, 自由变量, 局部变量);
                for (const auto& 子 : 分支.主体) 收集语句自由变量(*子, 自由变量, 局部变量);
            }
            for (const auto& 子 : 如果.else块) 收集语句自由变量(*子, 自由变量, 局部变量);
            break;
        }
        case 语句类型::循环语句:
            for (const auto& 子 : static_cast<const 循环语句&>(语句).主体) 收集语句自由变量(*子, 自由变量, 局部变量);
            break;
        case 语句类型::当循环语句: {
            const auto& 当循环 = static_cast<const 当循环语句&>(语句);
            收集自由变量(*当循环.条件, 自由变量, 局部变量);
            for (const auto& 子 : 当循环.主体) 收集语句自由变量(*子, 自由变量, 局部变量);
            break;
        }
        case 语句类型::代码块:
            for (const auto& 子 : static_cast<const 代码块语句&>(语句).语句列表) 收集语句自由变量(*子, 自由变量, 局部变量);
            break;
        default: break;
    }
}

void 代码生成器::生成(const 程序& 程序) {
    调试打印("[代码生成器] 开始生成");
    // 预声明C运行时文件IO函数
    llvm::FunctionType* fopen类型 = llvm::FunctionType::get(
        llvm::PointerType::get(上下文, 0),
        {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
    模块->getOrInsertFunction("fopen", fopen类型);

    llvm::FunctionType* fclose类型 = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(上下文),
        {llvm::PointerType::get(上下文, 0)}, false);
    模块->getOrInsertFunction("fclose", fclose类型);

    llvm::FunctionType* fprintf类型 = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(上下文),
        {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, true);
    模块->getOrInsertFunction("fprintf", fprintf类型);

    llvm::FunctionType* fscanf类型 = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(上下文),
        {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, true);
    模块->getOrInsertFunction("fscanf", fscanf类型);

    llvm::FunctionType* fgets类型 = llvm::FunctionType::get(
        llvm::PointerType::get(上下文, 0),
        {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文), llvm::PointerType::get(上下文, 0)}, false);
    模块->getOrInsertFunction("fgets", fgets类型);

    // 创建全局文件句柄表
    文件表类型 = llvm::ArrayType::get(llvm::PointerType::get(上下文, 0), 256);
    文件表指针 = new llvm::GlobalVariable(*模块, 文件表类型, false, llvm::GlobalValue::InternalLinkage,
        llvm::ConstantAggregateZero::get(文件表类型), "__文件表");
    // 文件句柄计数器（下一个可用槽位）
    文件句柄计数器 = new llvm::GlobalVariable(*模块, llvm::Type::getInt32Ty(上下文), false,
        llvm::GlobalValue::InternalLinkage, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "__文件句柄计数器");

    // 预声明运行时辅助函数
    llvm::FunctionType* 字符位置到字节位置类型 = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(上下文),
        {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)}, false);
    模块->getOrInsertFunction("字符位置到字节位置", 字符位置到字节位置类型);

    llvm::FunctionType* 获取字符数类型 = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(上下文),
        {llvm::PointerType::get(上下文, 0)}, false);
    模块->getOrInsertFunction("获取字符数", 获取字符数类型);

    // 映射辅助函数
    llvm::Type* i8Ptr = llvm::PointerType::get(上下文, 0);
    llvm::Type* i32 = llvm::Type::getInt32Ty(上下文);

    llvm::FunctionType* 映射查找类型 = llvm::FunctionType::get(i32,
        {i8Ptr, i32, i8Ptr}, false);
    模块->getOrInsertFunction("映射查找", 映射查找类型);

    llvm::FunctionType* 映射设置类型 = llvm::FunctionType::get(i32,
        {i8Ptr, i8Ptr, i32, i8Ptr, i8Ptr}, false);
    模块->getOrInsertFunction("映射设置", 映射设置类型);

    llvm::FunctionType* 映射获取类型 = llvm::FunctionType::get(i8Ptr,
        {i8Ptr, i8Ptr, i32, i8Ptr}, false);
    模块->getOrInsertFunction("映射获取", 映射获取类型);

    for (const auto& 全局变量 : 程序.全局变量) {
        if (全局变量->初始值 && 全局变量->初始值->类型 == 表达式类型::字符串) {
            // 字符串全局变量
            const auto& str = static_cast<const 字符串表达式&>(*全局变量->初始值);
            llvm::Constant* strConst = llvm::ConstantDataArray::getString(上下文, str.值);
            auto* 全局 = new llvm::GlobalVariable(*模块, strConst->getType(), true,
                llvm::GlobalValue::InternalLinkage, strConst, ".str." + 全局变量->变量名);
            // 创建指针全局变量
            auto* 指针全局 = new llvm::GlobalVariable(*模块, llvm::PointerType::get(上下文, 0), false,
                llvm::GlobalValue::ExternalLinkage, 全局, 全局变量->变量名);
            符号表实例.声明全局变量(全局变量->变量名, 指针全局);
            符号表实例.设置指针变量(全局变量->变量名);
        } else {
            llvm::ConstantInt* 初始值 = nullptr;
            if (全局变量->初始值 && 全局变量->初始值->类型 == 表达式类型::整数) {
                初始值 = llvm::ConstantInt::get(上下文, llvm::APInt(32, static_cast<const 整数表达式&>(*全局变量->初始值).值));
            } else if (全局变量->初始值 && 全局变量->初始值->类型 == 表达式类型::布尔值) {
                初始值 = llvm::ConstantInt::get(上下文, llvm::APInt(32, static_cast<const 布尔表达式&>(*全局变量->初始值).值 ? 1 : 0));
            } else {
                初始值 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
            }
            auto* 全局 = new llvm::GlobalVariable(*模块, llvm::Type::getInt32Ty(上下文), false, llvm::GlobalValue::ExternalLinkage, 初始值, 全局变量->变量名);
            符号表实例.声明全局变量(全局变量->变量名, 全局);
        }
    }

    调试打印("[代码生成器] 预声明完成，开始生成函数");

    // 处理结构体定义
    for (const auto& [名称, 成员列表] : 程序.结构体定义列表) {
        std::vector<llvm::Type*> 成员类型;
        std::vector<std::string> 成员名;
        std::vector<std::string> 成员类型名;
        for (const auto& 成员 : 成员列表) {
            成员类型.push_back(类型名到LLVM类型(成员.类型));
            成员名.push_back(成员.名称);
            成员类型名.push_back(成员.类型);
        }
        llvm::StructType* 结构体类型 = llvm::StructType::create(上下文, 成员类型, 名称);
        结构体类型映射[名称] = 结构体类型;
        结构体成员映射[名称] = 成员名;
        结构体成员类型名映射[名称] = 成员类型名;
        调试打印("[代码生成器] 定义结构体: " << 名称 << " 成员数: " << 成员名.size());
    }

    // 处理枚举定义
    for (const auto& [名称, 成员列表] : 程序.枚举定义列表) {
        符号表实例.声明枚举(名称, 成员列表);
        调试打印("[代码生成器] 定义枚举: " << 名称 << " 成员数: " << 成员列表.size());
    }

    // 第一遍：声明所有函数签名（支持前向引用）
    for (const auto& 函数 : 程序.函数列表) {
        std::vector<llvm::Type*> 参数类型;
        for (size_t i = 0; i < 函数->参数列表.size(); ++i) {
            if (!函数->参数列表[i].类型.empty()) {
                参数类型.push_back(类型名到LLVM类型(函数->参数列表[i].类型));
            } else {
                参数类型.push_back(llvm::PointerType::get(上下文, 0));
            }
        }
        llvm::Type* 返回LLVM类型 = llvm::Type::getVoidTy(上下文);
        if (!函数->返回类型.empty() && 函数->返回类型 != "空") {
            返回LLVM类型 = 类型名到LLVM类型(函数->返回类型);
        }
        llvm::FunctionType* 函数类型 = llvm::FunctionType::get(返回LLVM类型, 参数类型, false);
        llvm::Function::Create(函数类型, llvm::Function::ExternalLinkage, 函数->名称, *模块);
    }

    // 第二遍：生成函数体
    for (const auto& 函数 : 程序.函数列表) 生成函数(*函数);
    调试打印("[代码生成器] 函数生成完成");

    if (!程序.函数列表.empty()) {
        llvm::Function* 入口函数 = 模块->getFunction(程序.函数列表.back()->名称);
        if (入口函数) {
            llvm::FunctionType* main类型 = llvm::FunctionType::get(llvm::Type::getInt32Ty(上下文), false);
            llvm::Function* main函数 = llvm::Function::Create(main类型, llvm::Function::ExternalLinkage, "main", *模块);
            llvm::BasicBlock* 入口块 = llvm::BasicBlock::Create(上下文, "entry", main函数);
            构建器->SetInsertPoint(入口块);
            构建器->CreateCall(入口函数);
            构建器->CreateRet(llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)));
        }
    }
}

void 代码生成器::生成函数(const 函数& 函数) {
    llvm::Function* llvm函数 = 模块->getFunction(函数.名称);
    if (!llvm函数) {
        // 如果函数不存在，创建它
        std::vector<llvm::Type*> 参数类型;
        for (size_t i = 0; i < 函数.参数列表.size(); ++i) {
            if (!函数.参数列表[i].类型.empty()) {
                参数类型.push_back(类型名到LLVM类型(函数.参数列表[i].类型));
            } else {
                参数类型.push_back(llvm::PointerType::get(上下文, 0));
            }
        }
        llvm::Type* 返回LLVM类型 = llvm::Type::getVoidTy(上下文);
        if (!函数.返回类型.empty() && 函数.返回类型 != "空") {
            返回LLVM类型 = 类型名到LLVM类型(函数.返回类型);
        }
        llvm::FunctionType* 函数类型 = llvm::FunctionType::get(返回LLVM类型, 参数类型, false);
        llvm函数 = llvm::Function::Create(函数类型, llvm::Function::ExternalLinkage, 函数.名称, *模块);
    }

    llvm::BasicBlock* 入口块 = llvm::BasicBlock::Create(上下文, "entry", llvm函数);
    构建器->SetInsertPoint(入口块);
    符号表实例.重置为全局作用域();

    size_t i = 0;
    for (auto& 参数 : llvm函数->args()) {
        参数.setName(函数.参数列表[i].名称);
        符号表实例.声明变量(函数.参数列表[i].名称, &参数);
        if (函数.参数列表[i].类型 == "浮点" || 函数.参数列表[i].类型 == "浮点数") {
            符号表实例.设置浮点变量(函数.参数列表[i].名称);
        } else if (函数.参数列表[i].类型 == "字符串") {
            符号表实例.设置指针变量(函数.参数列表[i].名称);
        }
        i++;
    }

    for (const auto& 语句 : 函数.主体) 生成语句(*语句);

    if (构建器->GetInsertBlock()->getTerminator() == nullptr) {
        if (!函数.返回类型.empty() && 函数.返回类型 != "空") {
            构建器->CreateRet(llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)));
        } else {
            构建器->CreateRetVoid();
        }
    }
    llvm::verifyFunction(*llvm函数);
}

void 代码生成器::生成语句(const 语句& 语句) {
    if (构建器->GetInsertBlock()->getTerminator() != nullptr) return;

    switch (语句.类型) {
        case 语句类型::变量声明:
        case 语句类型::常量声明: {
            std::string 名称;
            const 表达式* 初始值表达式;
            if (语句.类型 == 语句类型::变量声明) {
                const auto& 声明 = static_cast<const 变量声明&>(语句);
                名称 = 声明.变量名;
                初始值表达式 = 声明.初始值.get();
            } else {
                const auto& 声明 = static_cast<const 常量声明&>(语句);
                名称 = 声明.常量名;
                初始值表达式 = 声明.初始值.get();
            }
            if (初始值表达式->类型 == 表达式类型::数组字面量) {
                const auto& 数组 = static_cast<const 数组字面量表达式&>(*初始值表达式);
                int 大小 = static_cast<int>(数组.元素列表.size());
                bool 含字符串 = false;
                for (const auto& 元素 : 数组.元素列表) {
                    if (元素->类型 == 表达式类型::字符串) { 含字符串 = true; break; }
                }
                if (含字符串) {
                    llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::PointerType::get(上下文, 0), 大小);
                    llvm::AllocaInst* 分配 = 构建器->CreateAlloca(数组类型, nullptr, 名称);
                    for (int j = 0; j < 大小; j++) {
                        llvm::Value* 索引 = llvm::ConstantInt::get(上下文, llvm::APInt(32, j));
                        llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(数组类型, 分配, {llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 索引}, "元素");
                        构建器->CreateStore(生成表达式(*数组.元素列表[j]), 元素地址);
                    }
                    符号表实例.声明数组(名称, 分配, 大小);
                    符号表实例.设置字符串数组(名称);
                } else {
                    llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::Type::getInt32Ty(上下文), 大小);
                    llvm::AllocaInst* 分配 = 构建器->CreateAlloca(数组类型, nullptr, 名称);
                    for (int j = 0; j < 大小; j++) {
                        llvm::Value* 索引 = llvm::ConstantInt::get(上下文, llvm::APInt(32, j));
                        llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(数组类型, 分配, {llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 索引}, "元素");
                        构建器->CreateStore(生成表达式(*数组.元素列表[j]), 元素地址);
                    }
                    符号表实例.声明数组(名称, 分配, 大小);
                }
            } else if (初始值表达式->类型 == 表达式类型::匿名函数) {
                llvm::Value* 函数值 = 生成表达式(*初始值表达式);
                llvm::Function* 匿名函数 = llvm::cast<llvm::Function>(函数值);
                const 闭包信息* 旧信息 = 符号表实例.获取闭包信息(匿名函数->getName().str());
                if (旧信息) 符号表实例.声明函数指针(名称, 匿名函数, 旧信息->捕获变量名, 旧信息->捕获变量地址);
                else 符号表实例.声明函数指针(名称, 匿名函数);
            } else if (初始值表达式->类型 == 表达式类型::结构体实例) {
                llvm::Value* 初始值 = 生成表达式(*初始值表达式);
                符号表实例.声明变量(名称, 初始值);
                符号表实例.设置结构体变量(名称, static_cast<const 结构体实例表达式&>(*初始值表达式).结构体名);
            } else if (初始值表达式->类型 == 表达式类型::映射字面量) {
                llvm::Value* 初始值 = 生成表达式(*初始值表达式);
                符号表实例.声明变量(名称, 初始值);
                符号表实例.设置映射变量(名称);
            } else if (初始值表达式->类型 == 表达式类型::元组字面量) {
                llvm::Value* 初始值 = 生成表达式(*初始值表达式);
                符号表实例.声明变量(名称, 初始值);
                // 从alloca获取元组类型
                if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(初始值)) {
                    元组类型映射[名称] = llvm::cast<llvm::StructType>(alloca->getAllocatedType());
                }
            } else {
                llvm::Value* 初始值 = 生成表达式(*初始值表达式);
                bool 是浮点 = 初始值->getType()->isDoubleTy();
                bool 是指针 = 初始值->getType()->isPointerTy();
                llvm::Type* 变量类型;
                if (是浮点) 变量类型 = llvm::Type::getDoubleTy(上下文);
                else if (是指针) 变量类型 = llvm::PointerType::get(上下文, 0);
                else 变量类型 = llvm::Type::getInt32Ty(上下文);
                llvm::AllocaInst* 分配 = 构建器->CreateAlloca(变量类型, nullptr, 名称);
                构建器->CreateStore(初始值, 分配);
                符号表实例.声明变量(名称, 分配);
                if (是浮点) 符号表实例.设置浮点变量(名称);
                if (是指针) 符号表实例.设置指针变量(名称);
            }
            if (语句.类型 == 语句类型::常量声明) 符号表实例.声明常量(名称);
            break;
        }
        case 语句类型::赋值语句: {
            const auto& 赋值 = static_cast<const 赋值语句&>(语句);
            // 常量不可变性检查
            if (符号表实例.是常量(赋值.变量名)) {
                throw std::runtime_error("不能对常量 '" + 赋值.变量名 + "' 赋值（行 " + std::to_string(0) + "）");
            }
            // 检查是否为成员赋值 (变量名.成员名)
            size_t 点位置 = 赋值.变量名.find('.');
            if (点位置 != std::string::npos) {
                std::string 对象名 = 赋值.变量名.substr(0, 点位置);
                std::string 成员名 = 赋值.变量名.substr(点位置 + 1);
                llvm::Value* 对象地址 = 符号表实例.获取变量值(对象名);
                if (!对象地址) throw std::runtime_error("未定义的变量: " + 对象名);
                const std::string& 结构体名 = 符号表实例.获取结构体类型(对象名);
                if (结构体名.empty()) throw std::runtime_error("变量不是结构体: " + 对象名);
                auto it = 结构体类型映射.find(结构体名);
                if (it == 结构体类型映射.end()) throw std::runtime_error("未定义的结构体: " + 结构体名);
                llvm::StructType* 结构体类型 = it->second;
                const auto& 成员名列表 = 结构体成员映射[结构体名];
                int 成员索引 = -1;
                for (size_t i = 0; i < 成员名列表.size(); i++) {
                    if (成员名列表[i] == 成员名) { 成员索引 = static_cast<int>(i); break; }
                }
                if (成员索引 < 0) throw std::runtime_error("未定义的成员: " + 成员名);
                llvm::Value* 成员地址 = 构建器->CreateStructGEP(结构体类型, 对象地址, static_cast<unsigned>(成员索引), 成员名);
                构建器->CreateStore(生成表达式(*赋值.值表达式), 成员地址);
            } else {
                llvm::Value* 变量地址 = 符号表实例.获取变量值(赋值.变量名);
                if (!变量地址) throw std::runtime_error("未定义的变量: " + 赋值.变量名);
                构建器->CreateStore(生成表达式(*赋值.值表达式), 变量地址);
            }
            break;
        }
        case 语句类型::下标赋值语句: {
            const auto& 下标赋值 = static_cast<const 下标赋值语句&>(语句);
            if (符号表实例.是常量(下标赋值.数组名)) {
                throw std::runtime_error("不能对常量 '" + 下标赋值.数组名 + "' 赋值");
            }
            llvm::Value* 数组地址 = 符号表实例.获取变量值(下标赋值.数组名);
            if (!数组地址) throw std::runtime_error("未定义的数组: " + 下标赋值.数组名);
            llvm::Value* 索引值 = 生成表达式(*下标赋值.索引);
            if (符号表实例.是字符串数组(下标赋值.数组名)) {
                llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::PointerType::get(上下文, 0), 符号表实例.获取数组大小(下标赋值.数组名));
                llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(数组类型, 数组地址, {llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 索引值}, 下标赋值.数组名 + "_元素");
                构建器->CreateStore(生成表达式(*下标赋值.值表达式), 元素地址);
            } else {
                llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(llvm::Type::getInt32Ty(上下文), 数组地址, 索引值, 下标赋值.数组名 + "_元素");
                构建器->CreateStore(生成表达式(*下标赋值.值表达式), 元素地址);
            }
            break;
        }
        case 语句类型::如果语句: {
            const auto& 如果 = static_cast<const 如果语句&>(语句);
            llvm::Value* 条件值 = 生成表达式(*如果.条件);
            条件值 = 构建器->CreateICmpNE(条件值, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "ifcond");
            llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();

            std::vector<llvm::BasicBlock*> elif块列表;
            for (size_t j = 0; j < 如果.否则如果列表.size(); ++j)
                elif块列表.push_back(llvm::BasicBlock::Create(上下文, "elif" + std::to_string(j), 当前函数));
            llvm::BasicBlock* else块 = llvm::BasicBlock::Create(上下文, "else", 当前函数);
            llvm::BasicBlock* 合并块 = llvm::BasicBlock::Create(上下文, "ifcont", 当前函数);

            llvm::BasicBlock* then块 = llvm::BasicBlock::Create(上下文, "then", 当前函数);
            llvm::BasicBlock* 否则如果目标 = elif块列表.empty() ? else块 : elif块列表[0];
            构建器->CreateCondBr(条件值, then块, 否则如果目标);

            构建器->SetInsertPoint(then块);
            符号表实例.进入作用域();
            for (const auto& 子 : 如果.then块) 生成语句(*子);
            符号表实例.退出作用域();
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(合并块);

            for (size_t j = 0; j < 如果.否则如果列表.size(); ++j) {
                构建器->SetInsertPoint(elif块列表[j]);
                llvm::Value* elif条件 = 生成表达式(*如果.否则如果列表[j].条件);
                elif条件 = 构建器->CreateICmpNE(elif条件, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "elifcond");
                llvm::BasicBlock* elif体块 = llvm::BasicBlock::Create(上下文, "elifbody", 当前函数);
                llvm::BasicBlock* 下一个 = (j + 1 < elif块列表.size()) ? elif块列表[j + 1] : else块;
                构建器->CreateCondBr(elif条件, elif体块, 下一个);
                构建器->SetInsertPoint(elif体块);
                符号表实例.进入作用域();
                for (const auto& 子 : 如果.否则如果列表[j].主体) 生成语句(*子);
                符号表实例.退出作用域();
                if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(合并块);
            }

            构建器->SetInsertPoint(else块);
            符号表实例.进入作用域();
            for (const auto& 子 : 如果.else块) 生成语句(*子);
            符号表实例.退出作用域();
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(合并块);
            构建器->SetInsertPoint(合并块);
            break;
        }
        case 语句类型::循环语句: {
            const auto& 循环 = static_cast<const 循环语句&>(语句);
            llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();

            // 处理初始化
            if (循环.初始化) {
                符号表实例.进入作用域();
                生成语句(*循环.初始化);
            }

            llvm::BasicBlock* 条件块 = llvm::BasicBlock::Create(上下文, "forcond", 当前函数);
            llvm::BasicBlock* 体块 = llvm::BasicBlock::Create(上下文, "forbody", 当前函数);
            llvm::BasicBlock* 步进块 = llvm::BasicBlock::Create(上下文, "forstep", 当前函数);
            llvm::BasicBlock* 后块 = llvm::BasicBlock::Create(上下文, "forend", 当前函数);

            循环继续栈.push(步进块);
            循环退出栈.push(后块);

            // 跳转到条件块
            构建器->CreateBr(条件块);

            // 条件块
            构建器->SetInsertPoint(条件块);
            if (循环.条件) {
                llvm::Value* 条件值 = 生成表达式(*循环.条件);
                条件值 = 构建器->CreateICmpNE(条件值, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "forcond");
                构建器->CreateCondBr(条件值, 体块, 后块);
            } else {
                构建器->CreateBr(体块);
            }

            // 循环体
            构建器->SetInsertPoint(体块);
            符号表实例.进入作用域();
            for (const auto& 子 : 循环.主体) 生成语句(*子);
            符号表实例.退出作用域();
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(步进块);

            // 步进块
            构建器->SetInsertPoint(步进块);
            if (循环.步进) {
                生成语句(*循环.步进);
            }
            构建器->CreateBr(条件块);

            循环继续栈.pop();
            循环退出栈.pop();
            构建器->SetInsertPoint(后块);

            if (循环.初始化) {
                符号表实例.退出作用域();
            }
            break;
        }
        case 语句类型::当循环语句: {
            const auto& 当循环 = static_cast<const 当循环语句&>(语句);
            llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();
            llvm::BasicBlock* 条件块 = llvm::BasicBlock::Create(上下文, "whilecond", 当前函数);
            llvm::BasicBlock* 体块 = llvm::BasicBlock::Create(上下文, "whilebody", 当前函数);
            llvm::BasicBlock* 后块 = llvm::BasicBlock::Create(上下文, "whileend", 当前函数);
            循环继续栈.push(条件块);
            循环退出栈.push(后块);
            构建器->CreateBr(条件块);
            构建器->SetInsertPoint(条件块);
            llvm::Value* 条件值 = 生成表达式(*当循环.条件);
            条件值 = 构建器->CreateICmpNE(条件值, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "whilecond");
            构建器->CreateCondBr(条件值, 体块, 后块);
            构建器->SetInsertPoint(体块);
            符号表实例.进入作用域();
            for (const auto& 子 : 当循环.主体) 生成语句(*子);
            符号表实例.退出作用域();
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(条件块);
            循环继续栈.pop();
            循环退出栈.pop();
            构建器->SetInsertPoint(后块);
            break;
        }
        case 语句类型::中断语句: {
            if (循环退出栈.empty()) throw std::runtime_error("'中断'必须在循环内使用");
            构建器->CreateBr(循环退出栈.top());
            llvm::BasicBlock* 新块 = llvm::BasicBlock::Create(上下文, "afterbreak", 构建器->GetInsertBlock()->getParent());
            构建器->SetInsertPoint(新块);
            break;
        }
        case 语句类型::继续语句: {
            if (循环继续栈.empty()) throw std::runtime_error("'继续'必须在循环内使用");
            构建器->CreateBr(循环继续栈.top());
            llvm::BasicBlock* 新块 = llvm::BasicBlock::Create(上下文, "aftercontinue", 构建器->GetInsertBlock()->getParent());
            构建器->SetInsertPoint(新块);
            break;
        }
        case 语句类型::返回语句: {
            const auto& 返回 = static_cast<const 返回语句&>(语句);
            if (返回.返回值) 构建器->CreateRet(生成表达式(*返回.返回值));
            else 构建器->CreateRetVoid();
            llvm::BasicBlock* 新块 = llvm::BasicBlock::Create(上下文, "afterreturn", 构建器->GetInsertBlock()->getParent());
            构建器->SetInsertPoint(新块);
            break;
        }
        case 语句类型::打印语句: {
            const auto& 打印 = static_cast<const 打印语句&>(语句);
            llvm::Value* 打印值 = 生成表达式(*打印.值表达式);
            llvm::FunctionType* printf类型 = llvm::FunctionType::get(llvm::Type::getInt32Ty(上下文), {llvm::PointerType::get(上下文, 0)}, true);
            模块->getOrInsertFunction("printf", printf类型);
            if (打印.值表达式->类型 == 表达式类型::字符串) {
                构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("%s\n"), 打印值});
            } else if (打印值->getType()->isDoubleTy() || 打印值->getType()->isFloatTy()) {
                构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("%.6g\n"), 打印值});
            } else if (打印值->getType()->isPointerTy()) {
                构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("%s\n"), 打印值});
            } else if (打印.值表达式->类型 == 表达式类型::变量 && 符号表实例.是浮点变量(static_cast<const 变量表达式&>(*打印.值表达式).名称)) {
                构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("%.6g\n"), 打印值});
            } else if (打印.值表达式->类型 == 表达式类型::下标访问 && 符号表实例.是映射变量(static_cast<const 下标访问表达式&>(*打印.值表达式).数组名)) {
                构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("%s\n"), 打印值});
            } else {
                构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("%d\n"), 打印值});
            }
            break;
        }
        case 语句类型::表达式语句:
            生成表达式(*static_cast<const 表达式语句&>(语句).值表达式);
            break;
        case 语句类型::代码块: {
            符号表实例.进入作用域();
            for (const auto& 子 : static_cast<const 代码块语句&>(语句).语句列表) 生成语句(*子);
            符号表实例.退出作用域();
            break;
        }
        case 语句类型::匹配语句: {
            const auto& 匹配 = static_cast<const 匹配语句&>(语句);
            llvm::Value* 匹配值 = 生成表达式(*匹配.匹配值);
            llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();
            llvm::BasicBlock* 合并块 = llvm::BasicBlock::Create(上下文, "matchend", 当前函数);

            // 创建所有分支的基本块
            std::vector<llvm::BasicBlock*> 分支块;
            for (size_t i = 0; i < 匹配.分支列表.size(); i++) {
                分支块.push_back(llvm::BasicBlock::Create(上下文, "match_" + std::to_string(i), 当前函数));
            }

            // 跳转到第一个分支
            构建器->CreateBr(分支块[0]);

            // 生成每个分支
            for (size_t i = 0; i < 匹配.分支列表.size(); i++) {
                构建器->SetInsertPoint(分支块[i]);
                const auto& 分支 = 匹配.分支列表[i];

                if (分支.条件) {
                    // 有条件的分支
                    llvm::Value* 分支值 = 生成表达式(*分支.条件);
                    llvm::Value* 比较结果 = 构建器->CreateICmpEQ(匹配值, 分支值, "matchcmp");

                    llvm::BasicBlock* 下一个块 = (i + 1 < 分支块.size()) ? 分支块[i + 1] : 合并块;
                    llvm::BasicBlock* 执行块 = llvm::BasicBlock::Create(上下文, "match_body_" + std::to_string(i), 当前函数);
                    构建器->CreateCondBr(比较结果, 执行块, 下一个块);

                    构建器->SetInsertPoint(执行块);
                    符号表实例.进入作用域();
                    for (const auto& 子 : 分支.主体) 生成语句(*子);
                    符号表实例.退出作用域();
                    if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(合并块);
                } else {
                    // 默认分支
                    符号表实例.进入作用域();
                    for (const auto& 子 : 分支.主体) 生成语句(*子);
                    符号表实例.退出作用域();
                    if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(合并块);
                }
            }

            构建器->SetInsertPoint(合并块);
            break;
        }
        case 语句类型::遍历语句: {
            const auto& 遍历 = static_cast<const 遍历语句&>(语句);
            llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();

            // 获取数组地址和大小
            llvm::Value* 数组地址 = 生成表达式(*遍历.数组表达式);
            int 数组大小值 = 100;
            // 尝试从数组表达式获取大小
            if (遍历.数组表达式->类型 == 表达式类型::变量) {
                const auto& 变量名 = static_cast<const 变量表达式&>(*遍历.数组表达式).名称;
                int 大小 = 符号表实例.获取数组大小(变量名);
                if (大小 > 0) 数组大小值 = 大小;
            }
            llvm::Value* 数组大小 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 数组大小值));

            // 创建循环变量
            llvm::AllocaInst* 索引变量 = 构建器->CreateAlloca(llvm::Type::getInt32Ty(上下文), nullptr, "遍历索引");
            构建器->CreateStore(llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 索引变量);

            llvm::BasicBlock* 条件块 = llvm::BasicBlock::Create(上下文, "forin_cond", 当前函数);
            llvm::BasicBlock* 体块 = llvm::BasicBlock::Create(上下文, "forin_body", 当前函数);
            llvm::BasicBlock* 步进块 = llvm::BasicBlock::Create(上下文, "forin_step", 当前函数);
            llvm::BasicBlock* 后块 = llvm::BasicBlock::Create(上下文, "forin_end", 当前函数);

            循环继续栈.push(步进块);
            循环退出栈.push(后块);

            构建器->CreateBr(条件块);

            // 条件块
            构建器->SetInsertPoint(条件块);
            llvm::Value* 当前索引 = 构建器->CreateLoad(llvm::Type::getInt32Ty(上下文), 索引变量, "当前索引");
            llvm::Value* 条件 = 构建器->CreateICmpSLT(当前索引, 数组大小, "遍历条件");
            构建器->CreateCondBr(条件, 体块, 后块);

            // 循环体
            构建器->SetInsertPoint(体块);
            符号表实例.进入作用域();

            // 加载当前元素
            if (符号表实例.是字符串数组(遍历.数组表达式->类型 == 表达式类型::变量 ? static_cast<const 变量表达式&>(*遍历.数组表达式).名称 : "")) {
                llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::PointerType::get(上下文, 0), 数组大小值);
                llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(数组类型, 数组地址, {llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 当前索引}, "元素地址");
                llvm::Value* 元素值 = 构建器->CreateLoad(llvm::PointerType::get(上下文, 0), 元素地址, "元素值");
                llvm::AllocaInst* 循环变量 = 构建器->CreateAlloca(llvm::PointerType::get(上下文, 0), nullptr, 遍历.变量名);
                构建器->CreateStore(元素值, 循环变量);
                符号表实例.声明变量(遍历.变量名, 循环变量);
                符号表实例.设置指针变量(遍历.变量名);
            } else {
                llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(llvm::Type::getInt32Ty(上下文), 数组地址, 当前索引, "元素地址");
                llvm::Value* 元素值 = 构建器->CreateLoad(llvm::Type::getInt32Ty(上下文), 元素地址, "元素值");
                llvm::AllocaInst* 循环变量 = 构建器->CreateAlloca(llvm::Type::getInt32Ty(上下文), nullptr, 遍历.变量名);
                构建器->CreateStore(元素值, 循环变量);
                符号表实例.声明变量(遍历.变量名, 循环变量);
            }

            for (const auto& 子 : 遍历.主体) 生成语句(*子);
            符号表实例.退出作用域();
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(步进块);

            // 步进块
            构建器->SetInsertPoint(步进块);
            llvm::Value* 新索引 = 构建器->CreateAdd(当前索引, llvm::ConstantInt::get(上下文, llvm::APInt(32, 1)), "新索引");
            构建器->CreateStore(新索引, 索引变量);
            构建器->CreateBr(条件块);

            循环继续栈.pop();
            循环退出栈.pop();
            构建器->SetInsertPoint(后块);
            break;
        }
        case 语句类型::结构体定义: {
            const auto& 结构体 = static_cast<const 结构体定义语句&>(语句);
            std::vector<llvm::Type*> 成员类型;
            std::vector<std::string> 成员名;
            std::vector<std::string> 成员类型名;
            for (const auto& 成员 : 结构体.成员列表) {
                成员类型.push_back(类型名到LLVM类型(成员.类型));
                成员名.push_back(成员.名称);
                成员类型名.push_back(成员.类型);
            }
            llvm::StructType* 结构体类型 = llvm::StructType::create(上下文, 成员类型, 结构体.名称);
            结构体类型映射[结构体.名称] = 结构体类型;
            结构体成员映射[结构体.名称] = 成员名;
            结构体成员类型名映射[结构体.名称] = 成员类型名;
            调试打印("[代码生成器] 定义结构体: " << 结构体.名称 << " 成员数: " << 成员名.size());
            break;
        }
        case 语句类型::枚举定义:
            // 枚举定义在全局阶段已处理，此处无需操作
            break;
        case 语句类型::导入语句:
            // 导入语句在全局阶段已处理，此处无需操作
            break;
    }
}

llvm::Value* 代码生成器::生成表达式(const 表达式& 表达式) {
    switch (表达式.类型) {
        case 表达式类型::整数:
            return llvm::ConstantInt::get(上下文, llvm::APInt(32, static_cast<const 整数表达式&>(表达式).值));
        case 表达式类型::浮点数:
            return llvm::ConstantFP::get(上下文, llvm::APFloat(static_cast<const 浮点表达式&>(表达式).值));
        case 表达式类型::布尔值:
            return llvm::ConstantInt::get(上下文, llvm::APInt(32, static_cast<const 布尔表达式&>(表达式).值 ? 1 : 0));
        case 表达式类型::字符串:
            return 构建器->CreateGlobalString(static_cast<const 字符串表达式&>(表达式).值);
        case 表达式类型::变量: {
            const auto& 变量 = static_cast<const 变量表达式&>(表达式);
            llvm::Value* 地址 = 符号表实例.获取变量值(变量.名称);
            if (!地址) throw std::runtime_error("未定义的变量: " + 变量.名称);
            if (符号表实例.是数组(变量.名称)) return 地址;
            llvm::Type* 类型;
            if (符号表实例.是浮点变量(变量.名称)) 类型 = llvm::Type::getDoubleTy(上下文);
            else if (符号表实例.是指针变量(变量.名称)) 类型 = llvm::PointerType::get(上下文, 0);
            else 类型 = llvm::Type::getInt32Ty(上下文);
            return 构建器->CreateLoad(类型, 地址, 变量.名称);
        }
        case 表达式类型::一元运算: {
            const auto& 一元 = static_cast<const 一元运算表达式&>(表达式);
            llvm::Value* 操作数 = 生成表达式(*一元.操作数);
            if (一元.操作符 == 一元操作符::逻辑非) {
                llvm::Value* 比较 = 构建器->CreateICmpEQ(操作数, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "nottmp");
                return 构建器->CreateZExt(比较, llvm::Type::getInt32Ty(上下文), "notext");
            }
            if (操作数->getType()->isDoubleTy()) {
                return 构建器->CreateFNeg(操作数, "fnegtmp");
            }
            return 构建器->CreateNeg(操作数, "negtmp");
        }
        case 表达式类型::二元运算: {
            const auto& 二元 = static_cast<const 二元运算表达式&>(表达式);
            if (二元.操作符 == 二元操作符::逻辑或) {
                llvm::Value* 左 = 生成表达式(*二元.左操作数);
                if (左->getType()->isIntegerTy(1)) 左 = 构建器->CreateZExt(左, llvm::Type::getInt32Ty(上下文));
                llvm::Value* 左结果 = 构建器->CreateICmpNE(左, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "lortmp");
                llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();
                llvm::BasicBlock* 右块 = llvm::BasicBlock::Create(上下文, "lor_rhs", 当前函数);
                llvm::BasicBlock* 合并块 = llvm::BasicBlock::Create(上下文, "lor_merge");
                构建器->CreateCondBr(左结果, 合并块, 右块);
                llvm::BasicBlock* 左块 = 构建器->GetInsertBlock();
                构建器->SetInsertPoint(右块);
                llvm::Value* 右 = 生成表达式(*二元.右操作数);
                if (右->getType()->isIntegerTy(1)) 右 = 构建器->CreateZExt(右, llvm::Type::getInt32Ty(上下文));
                llvm::Value* 右结果 = 构建器->CreateICmpNE(右, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "lortmp2");
                右块 = 构建器->GetInsertBlock();
                构建器->CreateBr(合并块);
                当前函数->insert(当前函数->end(), 合并块);
                构建器->SetInsertPoint(合并块);
                llvm::PHINode* phi = 构建器->CreatePHI(llvm::Type::getInt1Ty(上下文), 2, "lor");
                phi->addIncoming(llvm::ConstantInt::getTrue(上下文), 左块);
                phi->addIncoming(右结果, 右块);
                return 构建器->CreateZExt(phi, llvm::Type::getInt32Ty(上下文), "lorext");
            }
            if (二元.操作符 == 二元操作符::逻辑与) {
                llvm::Value* 左 = 生成表达式(*二元.左操作数);
                if (左->getType()->isIntegerTy(1)) 左 = 构建器->CreateZExt(左, llvm::Type::getInt32Ty(上下文));
                llvm::Value* 左结果 = 构建器->CreateICmpNE(左, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "landtmp");
                llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();
                llvm::BasicBlock* 右块 = llvm::BasicBlock::Create(上下文, "land_rhs", 当前函数);
                llvm::BasicBlock* 合并块 = llvm::BasicBlock::Create(上下文, "land_merge");
                构建器->CreateCondBr(左结果, 右块, 合并块);
                llvm::BasicBlock* 左块 = 构建器->GetInsertBlock();
                构建器->SetInsertPoint(右块);
                llvm::Value* 右 = 生成表达式(*二元.右操作数);
                if (右->getType()->isIntegerTy(1)) 右 = 构建器->CreateZExt(右, llvm::Type::getInt32Ty(上下文));
                llvm::Value* 右结果 = 构建器->CreateICmpNE(右, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "landtmp2");
                右块 = 构建器->GetInsertBlock();
                构建器->CreateBr(合并块);
                当前函数->insert(当前函数->end(), 合并块);
                构建器->SetInsertPoint(合并块);
                llvm::PHINode* phi = 构建器->CreatePHI(llvm::Type::getInt1Ty(上下文), 2, "land");
                phi->addIncoming(llvm::ConstantInt::getFalse(上下文), 左块);
                phi->addIncoming(右结果, 右块);
                return 构建器->CreateZExt(phi, llvm::Type::getInt32Ty(上下文), "landext");
            }
            llvm::Value* 左 = 生成表达式(*二元.左操作数);
            llvm::Value* 右 = 生成表达式(*二元.右操作数);

            // 字符串拼接
            if (二元.操作符 == 二元操作符::加法 &&
                左->getType()->isPointerTy() && 右->getType()->isPointerTy()) {
                // 声明 C 函数: sprintf, malloc, strlen
                llvm::Type* i8Ptr = llvm::PointerType::get(上下文, 0);
                llvm::FunctionType* sprintf类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文), {i8Ptr, i8Ptr}, true);
                llvm::FunctionType* malloc类型 = llvm::FunctionType::get(
                    i8Ptr, {llvm::Type::getInt64Ty(上下文)}, false);
                llvm::FunctionType* strlen类型 = llvm::FunctionType::get(
                    llvm::Type::getInt64Ty(上下文), {i8Ptr}, false);
                模块->getOrInsertFunction("sprintf", sprintf类型);
                模块->getOrInsertFunction("malloc", malloc类型);
                模块->getOrInsertFunction("strlen", strlen类型);

                // 计算精确缓冲区大小
                llvm::Value* 左长度 = 构建器->CreateCall(模块->getFunction("strlen"), {左}, "左长度");
                llvm::Value* 右长度 = 构建器->CreateCall(模块->getFunction("strlen"), {右}, "右长度");
                llvm::Value* 总长度 = 构建器->CreateAdd(左长度, 右长度, "总长度");
                llvm::Value* 缓冲区大小 = 构建器->CreateAdd(总长度, llvm::ConstantInt::get(上下文, llvm::APInt(64, 1)), "缓冲区大小");

                llvm::Value* 缓冲区 = 构建器->CreateCall(模块->getFunction("malloc"), {缓冲区大小}, "拼接缓冲区");
                构建器->CreateCall(模块->getFunction("sprintf"),
                    {缓冲区, 构建器->CreateGlobalString("%s%s"), 左, 右});
                return 缓冲区;
            }

            // 字符串比较
            if (二元.操作符 == 二元操作符::等于 &&
                左->getType()->isPointerTy() && 右->getType()->isPointerTy()) {
                llvm::FunctionType* strcmp类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("strcmp", strcmp类型);
                llvm::Value* 结果 = 构建器->CreateCall(模块->getFunction("strcmp"), {左, 右}, "strcmp结果");
                return 构建器->CreateZExt(
                    构建器->CreateICmpEQ(结果, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "streql"),
                    llvm::Type::getInt32Ty(上下文));
            }

            // 检查是否为浮点运算
            bool 是浮点 = (左->getType()->isDoubleTy() || 右->getType()->isDoubleTy());
            if (是浮点) {
                // 类型提升：整数转浮点
                if (左->getType()->isIntegerTy()) 左 = 构建器->CreateSIToFP(左, llvm::Type::getDoubleTy(上下文), "int2float_l");
                if (右->getType()->isIntegerTy()) 右 = 构建器->CreateSIToFP(右, llvm::Type::getDoubleTy(上下文), "int2float_r");
                switch (二元.操作符) {
                    case 二元操作符::加法: return 构建器->CreateFAdd(左, 右, "faddtmp");
                    case 二元操作符::减法: return 构建器->CreateFSub(左, 右, "fsubtmp");
                    case 二元操作符::乘法: return 构建器->CreateFMul(左, 右, "fmultmp");
                    case 二元操作符::除法: return 构建器->CreateFDiv(左, 右, "fdivtmp");
                    case 二元操作符::等于: return 构建器->CreateZExt(构建器->CreateFCmpOEQ(左, 右, "feqtmp"), llvm::Type::getInt32Ty(上下文));
                    case 二元操作符::不等于: return 构建器->CreateZExt(构建器->CreateFCmpONE(左, 右, "fnetmp"), llvm::Type::getInt32Ty(上下文));
                    case 二元操作符::小于: return 构建器->CreateZExt(构建器->CreateFCmpOLT(左, 右, "flttmp"), llvm::Type::getInt32Ty(上下文));
                    case 二元操作符::大于: return 构建器->CreateZExt(构建器->CreateFCmpOGT(左, 右, "fgttmp"), llvm::Type::getInt32Ty(上下文));
                    case 二元操作符::小于等于: return 构建器->CreateZExt(构建器->CreateFCmpOLE(左, 右, "fletmp"), llvm::Type::getInt32Ty(上下文));
                    case 二元操作符::大于等于: return 构建器->CreateZExt(构建器->CreateFCmpOGE(左, 右, "fgetmp"), llvm::Type::getInt32Ty(上下文));
                    default: throw std::runtime_error("不支持的浮点运算符");
                }
            }

            // 整数运算
            switch (二元.操作符) {
                case 二元操作符::加法: return 构建器->CreateAdd(左, 右, "addtmp");
                case 二元操作符::减法: return 构建器->CreateSub(左, 右, "subtmp");
                case 二元操作符::乘法: return 构建器->CreateMul(左, 右, "multmp");
                case 二元操作符::除法: return 构建器->CreateSDiv(左, 右, "divtmp");
                case 二元操作符::取模: return 构建器->CreateSRem(左, 右, "modtmp");
                case 二元操作符::等于: return 构建器->CreateZExt(构建器->CreateICmpEQ(左, 右, "eqtmp"), llvm::Type::getInt32Ty(上下文));
                case 二元操作符::不等于: return 构建器->CreateZExt(构建器->CreateICmpNE(左, 右, "netmp"), llvm::Type::getInt32Ty(上下文));
                case 二元操作符::小于: return 构建器->CreateZExt(构建器->CreateICmpSLT(左, 右, "lttmp"), llvm::Type::getInt32Ty(上下文));
                case 二元操作符::大于: return 构建器->CreateZExt(构建器->CreateICmpSGT(左, 右, "gttmp"), llvm::Type::getInt32Ty(上下文));
                case 二元操作符::小于等于: return 构建器->CreateZExt(构建器->CreateICmpSLE(左, 右, "letmp"), llvm::Type::getInt32Ty(上下文));
                case 二元操作符::大于等于: return 构建器->CreateZExt(构建器->CreateICmpSGE(左, 右, "getmp"), llvm::Type::getInt32Ty(上下文));
                default: throw std::runtime_error("未知二元运算符");
            }
        }
        case 表达式类型::函数调用: {
            const auto& 调用 = static_cast<const 函数调用表达式&>(表达式);

            // 文件IO内置函数（通过全局文件表管理，句柄为表索引i32）
            if (调用.函数名 == "打开") {
                // 将UTF-8字符串转为UTF-16宽字符串用于_wfopen
                auto 转宽字符串 = [&](const std::string& utf8) -> std::vector<uint16_t> {
                    std::vector<uint16_t> 宽;
                    size_t i = 0;
                    while (i < utf8.size()) {
                        uint8_t c = utf8[i];
                        uint32_t 码点;
                        if (c < 0x80) { 码点 = c; i++; }
                        else if ((c & 0xE0) == 0xC0) { 码点 = (c & 0x1F) << 6 | (utf8[i+1] & 0x3F); i += 2; }
                        else if ((c & 0xF0) == 0xE0) { 码点 = (c & 0x0F) << 12 | (utf8[i+1] & 0x3F) << 6 | (utf8[i+2] & 0x3F); i += 3; }
                        else { 码点 = (c & 0x07) << 18 | (utf8[i+1] & 0x3F) << 12 | (utf8[i+2] & 0x3F) << 6 | (utf8[i+3] & 0x3F); i += 4; }
                        if (码点 <= 0xFFFF) 宽.push_back(static_cast<uint16_t>(码点));
                        else { 宽.push_back(static_cast<uint16_t>(0xD800 + ((码点 - 0x10000) >> 10))); 宽.push_back(static_cast<uint16_t>(0xDC00 + ((码点 - 0x10000) & 0x3FF))); }
                    }
                    宽.push_back(0);
                    return 宽;
                };

                auto 路径宽 = 转宽字符串(static_cast<const 字符串表达式&>(*调用.参数列表[0]).值);
                auto 模式宽 = 转宽字符串(static_cast<const 字符串表达式&>(*调用.参数列表[1]).值);

                llvm::ArrayType* 路径类型 = llvm::ArrayType::get(llvm::Type::getInt16Ty(上下文), 路径宽.size());
                llvm::ArrayType* 模式类型 = llvm::ArrayType::get(llvm::Type::getInt16Ty(上下文), 模式宽.size());

                std::vector<llvm::Constant*> 路径常量;
                for (auto w : 路径宽) 路径常量.push_back(llvm::ConstantInt::get(llvm::Type::getInt16Ty(上下文), w));
                std::vector<llvm::Constant*> 模式常量;
                for (auto w : 模式宽) 模式常量.push_back(llvm::ConstantInt::get(llvm::Type::getInt16Ty(上下文), w));

                auto* 路径全局 = new llvm::GlobalVariable(*模块, 路径类型, true, llvm::GlobalValue::PrivateLinkage,
                    llvm::ConstantArray::get(路径类型, 路径常量), ".路径宽");
                auto* 模式全局 = new llvm::GlobalVariable(*模块, 模式类型, true, llvm::GlobalValue::PrivateLinkage,
                    llvm::ConstantArray::get(模式类型, 模式常量), ".模式宽");

                llvm::FunctionType* wfopen类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                auto wfopen调用 = 模块->getOrInsertFunction("_wfopen", wfopen类型);

                llvm::Value* 零 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                llvm::Value* 路径指针 = 构建器->CreateInBoundsGEP(路径类型, 路径全局, {零, 零}, "路径指针");
                llvm::Value* 模式指针 = 构建器->CreateInBoundsGEP(模式类型, 模式全局, {零, 零}, "模式指针");
                llvm::Value* 文件指针 = 构建器->CreateCall(wfopen调用, {路径指针, 模式指针}, "wfopen结果");
                // 分配文件槽位
                llvm::Value* 当前计数 = 构建器->CreateLoad(llvm::Type::getInt32Ty(上下文), 文件句柄计数器, "当前计数");
                llvm::Value* 槽地址 = 构建器->CreateInBoundsGEP(文件表类型, 文件表指针, {零, 当前计数}, "槽地址");
                构建器->CreateStore(文件指针, 槽地址);
                // 计数器+1
                llvm::Value* 新计数 = 构建器->CreateAdd(当前计数, llvm::ConstantInt::get(上下文, llvm::APInt(32, 1)));
                构建器->CreateStore(新计数, 文件句柄计数器);
                return 当前计数;
            }
            if (调用.函数名 == "关闭") {
                llvm::FunctionType* fclose类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                auto fclose调用 = 模块->getOrInsertFunction("fclose", fclose类型);
                llvm::Value* 句柄 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 零 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                llvm::Value* 槽地址 = 构建器->CreateInBoundsGEP(文件表类型, 文件表指针, {零, 句柄}, "槽地址");
                llvm::Value* 文件指针 = 构建器->CreateLoad(llvm::PointerType::get(上下文, 0), 槽地址, "文件指针");
                return 构建器->CreateCall(fclose调用, {文件指针});
            }
            if (调用.函数名 == "写入") {
                llvm::FunctionType* fprintf类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, true);
                auto fprintf调用 = 模块->getOrInsertFunction("fprintf", fprintf类型);
                llvm::Value* 句柄 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 零 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                llvm::Value* 槽地址 = 构建器->CreateInBoundsGEP(文件表类型, 文件表指针, {零, 句柄}, "槽地址");
                llvm::Value* 文件指针 = 构建器->CreateLoad(llvm::PointerType::get(上下文, 0), 槽地址, "文件指针");
                llvm::Value* 内容 = 生成表达式(*调用.参数列表[1]);
                if (调用.参数列表[1]->类型 == 表达式类型::字符串) {
                    return 构建器->CreateCall(fprintf调用, {文件指针, 内容});
                } else if (内容->getType()->isDoubleTy()) {
                    return 构建器->CreateCall(fprintf调用, {文件指针, 构建器->CreateGlobalString("%.6g"), 内容});
                } else {
                    return 构建器->CreateCall(fprintf调用, {文件指针, 构建器->CreateGlobalString("%d"), 内容});
                }
            }
            if (调用.函数名 == "读取") {
                llvm::FunctionType* fscanf类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, true);
                auto fscanf调用 = 模块->getOrInsertFunction("fscanf", fscanf类型);
                llvm::Value* 句柄 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 零 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                llvm::Value* 槽地址 = 构建器->CreateInBoundsGEP(文件表类型, 文件表指针, {零, 句柄}, "槽地址");
                llvm::Value* 文件指针 = 构建器->CreateLoad(llvm::PointerType::get(上下文, 0), 槽地址, "文件指针");
                llvm::AllocaInst* 临时存储 = 构建器->CreateAlloca(llvm::Type::getInt32Ty(上下文), nullptr, "读取临时");
                构建器->CreateCall(fscanf调用, {文件指针, 构建器->CreateGlobalString("%d"), 临时存储});
                return 构建器->CreateLoad(llvm::Type::getInt32Ty(上下文), 临时存储, "读取结果");
            }
            if (调用.函数名 == "读取行") {
                llvm::FunctionType* fgets类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文), llvm::PointerType::get(上下文, 0)}, false);
                auto fgets调用 = 模块->getOrInsertFunction("fgets", fgets类型);
                llvm::FunctionType* malloc类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::Type::getInt64Ty(上下文)}, false);
                模块->getOrInsertFunction("malloc", malloc类型);
                llvm::FunctionType* strlen类型 = llvm::FunctionType::get(
                    llvm::Type::getInt64Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("strlen", strlen类型);
                llvm::Value* 句柄 = 调用.参数列表.size() > 0 ? 生成表达式(*调用.参数列表[0]) : llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                llvm::Value* 零 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                llvm::Value* 槽地址 = 构建器->CreateInBoundsGEP(文件表类型, 文件表指针, {零, 句柄}, "槽地址");
                llvm::Value* 文件指针 = 构建器->CreateLoad(llvm::PointerType::get(上下文, 0), 槽地址, "文件指针");
                llvm::Value* 缓冲区 = 构建器->CreateCall(模块->getFunction("malloc"),
                    {llvm::ConstantInt::get(上下文, llvm::APInt(64, 4096))}, "读取行缓冲区");
                构建器->CreateCall(fgets调用, {缓冲区,
                    llvm::ConstantInt::get(上下文, llvm::APInt(32, 4096)), 文件指针});
                // 去掉末尾换行符（安全检查：长度 > 0）
                llvm::Value* 长度 = 构建器->CreateCall(模块->getFunction("strlen"), {缓冲区}, "缓冲区长度");
                llvm::Value* 长度大于零 = 构建器->CreateICmpUGT(长度, llvm::ConstantInt::get(上下文, llvm::APInt(64, 0)), "长度大于零");
                llvm::Value* 最后位置 = 构建器->CreateSub(长度, llvm::ConstantInt::get(上下文, llvm::APInt(64, 1)), "最后位置");
                llvm::Value* 安全最后位置 = 构建器->CreateSelect(长度大于零, 最后位置, llvm::ConstantInt::get(上下文, llvm::APInt(64, 0)), "安全最后位置");
                llvm::Value* 最后字符地址 = 构建器->CreateInBoundsGEP(llvm::Type::getInt8Ty(上下文), 缓冲区, 安全最后位置, "最后字符地址");
                llvm::Value* 最后字符 = 构建器->CreateLoad(llvm::Type::getInt8Ty(上下文), 最后字符地址, "最后字符");
                llvm::Value* 是换行 = 构建器->CreateICmpEQ(最后字符,
                    llvm::ConstantInt::get(上下文, llvm::APInt(8, '\n')), "是换行");
                llvm::Value* 应替换 = 构建器->CreateAnd(长度大于零, 是换行, "应替换");
                llvm::Value* 空终止 = llvm::ConstantInt::get(llvm::Type::getInt8Ty(上下文), 0);
                llvm::Value* 原始字节 = 构建器->CreateLoad(llvm::Type::getInt8Ty(上下文), 最后字符地址, "原始字节");
                llvm::Value* 最终字节 = 构建器->CreateSelect(应替换, 空终止, 原始字节, "最终字节");
                构建器->CreateStore(最终字节, 最后字符地址);
                return 缓冲区;
            }

            // 字符串操作内置函数
            if (调用.函数名 == "长度") {
                llvm::Value* str = 生成表达式(*调用.参数列表[0]);
                llvm::Value* len = 构建器->CreateCall(模块->getFunction("获取字符数"), {str}, "字符长度");
                return len;
            }
            if (调用.函数名 == "拼接") {
                // 使用 sprintf 拼接字符串（动态计算大小）
                llvm::FunctionType* sprintf类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, true);
                模块->getOrInsertFunction("sprintf", sprintf类型);
                llvm::FunctionType* malloc类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::Type::getInt64Ty(上下文)}, false);
                模块->getOrInsertFunction("malloc", malloc类型);
                llvm::FunctionType* strlen类型 = llvm::FunctionType::get(
                    llvm::Type::getInt64Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("strlen", strlen类型);
                llvm::Value* str1 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* str2 = 生成表达式(*调用.参数列表[1]);
                // 计算精确缓冲区大小
                llvm::Value* 左长度 = 构建器->CreateCall(模块->getFunction("strlen"), {str1}, "左长度");
                llvm::Value* 右长度 = 构建器->CreateCall(模块->getFunction("strlen"), {str2}, "右长度");
                llvm::Value* 总长度 = 构建器->CreateAdd(左长度, 右长度, "总长度");
                llvm::Value* 缓冲区大小 = 构建器->CreateAdd(总长度, llvm::ConstantInt::get(上下文, llvm::APInt(64, 1)), "缓冲区大小");
                llvm::Value* 缓冲区 = 构建器->CreateCall(模块->getFunction("malloc"), {缓冲区大小}, "拼接缓冲区");
                构建器->CreateCall(模块->getFunction("sprintf"), {缓冲区, 构建器->CreateGlobalString("%s%s"), str1, str2});
                return 缓冲区;
            }
            if (调用.函数名 == "转字符串") {
                llvm::Value* 值 = 生成表达式(*调用.参数列表[0]);
                if (值->getType()->isPointerTy()) {
                    return 值;
                }
                llvm::FunctionType* sprintf类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, true);
                模块->getOrInsertFunction("sprintf", sprintf类型);
                llvm::FunctionType* malloc类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::Type::getInt64Ty(上下文)}, false);
                模块->getOrInsertFunction("malloc", malloc类型);
                llvm::Value* 缓冲区 = 构建器->CreateCall(模块->getFunction("malloc"),
                    {llvm::ConstantInt::get(上下文, llvm::APInt(64, 64))}, "转字符串缓冲区");
                if (值->getType()->isDoubleTy()) {
                    构建器->CreateCall(模块->getFunction("sprintf"), {缓冲区, 构建器->CreateGlobalString("%.6g"), 值});
                } else {
                    构建器->CreateCall(模块->getFunction("sprintf"), {缓冲区, 构建器->CreateGlobalString("%d"), 值});
                }
                return 缓冲区;
            }
            if (调用.函数名 == "查找") {
                llvm::FunctionType* strstr类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("strstr", strstr类型);
                llvm::Value* haystack = 生成表达式(*调用.参数列表[0]);
                llvm::Value* needle = 生成表达式(*调用.参数列表[1]);
                llvm::Value* result = 构建器->CreateCall(模块->getFunction("strstr"), {haystack, needle}, "查找结果");
                // 返回找到的位置，未找到返回-1
                llvm::Value* nullPtr = llvm::ConstantPointerNull::get(llvm::PointerType::get(上下文, 0));
                llvm::Value* isNull = 构建器->CreateICmpEQ(result, nullPtr, "isNull");
                // 使用 ptrtoint 计算偏移
                llvm::Value* resultInt = 构建器->CreatePtrToInt(result, llvm::Type::getInt64Ty(上下文), "resultInt");
                llvm::Value* haystackInt = 构建器->CreatePtrToInt(haystack, llvm::Type::getInt64Ty(上下文), "haystackInt");
                llvm::Value* offset = 构建器->CreateSub(resultInt, haystackInt, "偏移");
                // 使用 select 返回结果
                llvm::Value* notFound = llvm::ConstantInt::get(上下文, llvm::APInt(64, (uint64_t)-1));
                llvm::Value* finalOffset = 构建器->CreateSelect(isNull, notFound, offset, "查找位置64");
                return 构建器->CreateTrunc(finalOffset, llvm::Type::getInt32Ty(上下文), "查找位置");
            }
            if (调用.函数名 == "子串") {
                llvm::FunctionType* strncpy类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0), llvm::Type::getInt64Ty(上下文)}, false);
                模块->getOrInsertFunction("strncpy", strncpy类型);
                llvm::FunctionType* malloc类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::Type::getInt64Ty(上下文)}, false);
                模块->getOrInsertFunction("malloc", malloc类型);
                llvm::Value* src = 生成表达式(*调用.参数列表[0]);
                llvm::Value* charStart = 生成表达式(*调用.参数列表[1]);
                llvm::Value* charLen = 生成表达式(*调用.参数列表[2]);

                // 将字符位置转换为字节位置
                llvm::Value* byteStart = 构建器->CreateCall(模块->getFunction("字符位置到字节位置"), {src, charStart}, "字节起始");
                llvm::Value* byteEnd = 构建器->CreateCall(模块->getFunction("字符位置到字节位置"),
                    {src, 构建器->CreateAdd(charStart, charLen, "endChar")}, "字节结束");
                llvm::Value* byteLen = 构建器->CreateSub(byteEnd, byteStart, "字节长度");
                // 边界检查：确保byteLen >= 0
                llvm::Value* lenNonNeg = 构建器->CreateICmpSGE(byteLen, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)));
                byteLen = 构建器->CreateSelect(lenNonNeg, byteLen, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "安全字节长度");

                llvm::Value* srcPtr = 构建器->CreateGEP(llvm::Type::getInt8Ty(上下文), src, byteStart, "srcPtr");
                llvm::Value* 缓冲区 = 构建器->CreateCall(模块->getFunction("malloc"),
                    {构建器->CreateZExt(构建器->CreateAdd(byteLen, llvm::ConstantInt::get(上下文, llvm::APInt(32, 1)), "bufsz"),
                        llvm::Type::getInt64Ty(上下文))}, "子串缓冲区");
                构建器->CreateCall(模块->getFunction("strncpy"), {缓冲区, srcPtr, 构建器->CreateZExt(byteLen, llvm::Type::getInt64Ty(上下文))});
                // null terminate
                llvm::Value* termPos = 构建器->CreateInBoundsGEP(llvm::Type::getInt8Ty(上下文), 缓冲区, byteLen, "termPos");
                构建器->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt8Ty(上下文), 0), termPos);
                return 缓冲区;
            }

            // 数学函数
            if (调用.函数名 == "绝对值") {
                llvm::Value* val = 生成表达式(*调用.参数列表[0]);
                if (val->getType()->isDoubleTy()) {
                    llvm::FunctionType* fabs类型 = llvm::FunctionType::get(
                        llvm::Type::getDoubleTy(上下文), {llvm::Type::getDoubleTy(上下文)}, false);
                    模块->getOrInsertFunction("fabs", fabs类型);
                    return 构建器->CreateCall(模块->getFunction("fabs"), {val}, "fabs结果");
                }
                llvm::Value* 零 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                llvm::Value* 负值 = 构建器->CreateNeg(val, "负值");
                llvm::Value* 比较 = 构建器->CreateICmpSGE(val, 零, "非负");
                return 构建器->CreateSelect(比较, val, 负值, "绝对值");
            }
            if (调用.函数名 == "最小值") {
                llvm::Value* 左 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 右 = 生成表达式(*调用.参数列表[1]);
                if (左->getType()->isDoubleTy() || 右->getType()->isDoubleTy()) {
                    if (左->getType()->isIntegerTy()) 左 = 构建器->CreateSIToFP(左, llvm::Type::getDoubleTy(上下文));
                    if (右->getType()->isIntegerTy()) 右 = 构建器->CreateSIToFP(右, llvm::Type::getDoubleTy(上下文));
                    llvm::Value* 比较 = 构建器->CreateFCmpOLT(左, 右, "小于");
                    return 构建器->CreateSelect(比较, 左, 右, "最小值");
                }
                llvm::Value* 比较 = 构建器->CreateICmpSLT(左, 右, "小于");
                return 构建器->CreateSelect(比较, 左, 右, "最小值");
            }
            if (调用.函数名 == "最大值") {
                llvm::Value* 左 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 右 = 生成表达式(*调用.参数列表[1]);
                if (左->getType()->isDoubleTy() || 右->getType()->isDoubleTy()) {
                    if (左->getType()->isIntegerTy()) 左 = 构建器->CreateSIToFP(左, llvm::Type::getDoubleTy(上下文));
                    if (右->getType()->isIntegerTy()) 右 = 构建器->CreateSIToFP(右, llvm::Type::getDoubleTy(上下文));
                    llvm::Value* 比较 = 构建器->CreateFCmpOGT(左, 右, "大于");
                    return 构建器->CreateSelect(比较, 左, 右, "最大值");
                }
                llvm::Value* 比较 = 构建器->CreateICmpSGT(左, 右, "大于");
                return 构建器->CreateSelect(比较, 左, 右, "最大值");
            }

            // 新增数学函数
            if (调用.函数名 == "平方根") {
                llvm::Value* val = 生成表达式(*调用.参数列表[0]);
                if (!val->getType()->isDoubleTy()) val = 构建器->CreateSIToFP(val, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* sqrt类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文), {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("sqrt", sqrt类型);
                return 构建器->CreateCall(模块->getFunction("sqrt"), {val}, "平方根结果");
            }
            if (调用.函数名 == "四舍五入") {
                llvm::Value* val = 生成表达式(*调用.参数列表[0]);
                if (!val->getType()->isDoubleTy()) val = 构建器->CreateSIToFP(val, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* round类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文), {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("round", round类型);
                llvm::Value* rounded = 构建器->CreateCall(模块->getFunction("round"), {val}, "四舍五入结果");
                return 构建器->CreateFPToSI(rounded, llvm::Type::getInt32Ty(上下文), "四舍五入整数");
            }
            if (调用.函数名 == "幂运算") {
                llvm::Value* base = 生成表达式(*调用.参数列表[0]);
                llvm::Value* exp = 生成表达式(*调用.参数列表[1]);
                if (!base->getType()->isDoubleTy()) base = 构建器->CreateSIToFP(base, llvm::Type::getDoubleTy(上下文));
                if (!exp->getType()->isDoubleTy()) exp = 构建器->CreateSIToFP(exp, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* pow类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文), {llvm::Type::getDoubleTy(上下文), llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("pow", pow类型);
                return 构建器->CreateCall(模块->getFunction("pow"), {base, exp}, "幂运算结果");
            }
            if (调用.函数名 == "正弦") {
                llvm::Value* val = 生成表达式(*调用.参数列表[0]);
                if (!val->getType()->isDoubleTy()) val = 构建器->CreateSIToFP(val, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* sin类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文), {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("sin", sin类型);
                return 构建器->CreateCall(模块->getFunction("sin"), {val}, "正弦结果");
            }
            if (调用.函数名 == "余弦") {
                llvm::Value* val = 生成表达式(*调用.参数列表[0]);
                if (!val->getType()->isDoubleTy()) val = 构建器->CreateSIToFP(val, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* cos类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文), {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("cos", cos类型);
                return 构建器->CreateCall(模块->getFunction("cos"), {val}, "余弦结果");
            }
            if (调用.函数名 == "正切") {
                llvm::Value* val = 生成表达式(*调用.参数列表[0]);
                if (!val->getType()->isDoubleTy()) val = 构建器->CreateSIToFP(val, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* tan类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文), {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("tan", tan类型);
                return 构建器->CreateCall(模块->getFunction("tan"), {val}, "正切结果");
            }

            // 输入函数 — 从stdin读取一行
            if (调用.函数名 == "输入") {
                llvm::FunctionType* fgets类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("fgets", fgets类型);
                llvm::FunctionType* malloc类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0), {llvm::Type::getInt64Ty(上下文)}, false);
                模块->getOrInsertFunction("malloc", malloc类型);
                llvm::FunctionType* strlen类型 = llvm::FunctionType::get(
                    llvm::Type::getInt64Ty(上下文), {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("strlen", strlen类型);
                // stdin 在MSYS2/MinGW中是外部全局变量
                llvm::Value* stdinPtr = 模块->getNamedGlobal("stdin");
                if (!stdinPtr) {
                    auto* stdinGV = new llvm::GlobalVariable(*模块, llvm::PointerType::get(上下文, 0),
                        false, llvm::GlobalValue::ExternalLinkage, nullptr, "stdin");
                    stdinGV->setDLLStorageClass(llvm::GlobalValue::DLLImportStorageClass);
                    stdinPtr = stdinGV;
                }
                stdinPtr = 构建器->CreateLoad(llvm::PointerType::get(上下文, 0), stdinPtr, "stdin_val");
                llvm::Value* 缓冲区 = 构建器->CreateCall(模块->getFunction("malloc"),
                    {llvm::ConstantInt::get(上下文, llvm::APInt(64, 4096))}, "输入缓冲区");
                构建器->CreateCall(模块->getFunction("fgets"), {缓冲区,
                    llvm::ConstantInt::get(上下文, llvm::APInt(32, 4096)), stdinPtr});
                // 去掉末尾换行符（与读取行相同的load-select-store模式）
                llvm::Value* 长度 = 构建器->CreateCall(模块->getFunction("strlen"), {缓冲区}, "缓冲区长度");
                llvm::Value* 长度大于零 = 构建器->CreateICmpUGT(长度, llvm::ConstantInt::get(上下文, llvm::APInt(64, 0)));
                llvm::Value* 最后位置 = 构建器->CreateSub(长度, llvm::ConstantInt::get(上下文, llvm::APInt(64, 1)));
                llvm::Value* 安全位置 = 构建器->CreateSelect(长度大于零, 最后位置, llvm::ConstantInt::get(上下文, llvm::APInt(64, 0)));
                llvm::Value* 最后字符地址 = 构建器->CreateInBoundsGEP(llvm::Type::getInt8Ty(上下文), 缓冲区, 安全位置);
                llvm::Value* 最后字符 = 构建器->CreateLoad(llvm::Type::getInt8Ty(上下文), 最后字符地址);
                llvm::Value* 是换行 = 构建器->CreateICmpEQ(最后字符, llvm::ConstantInt::get(上下文, llvm::APInt(8, '\n')));
                llvm::Value* 应替换 = 构建器->CreateAnd(长度大于零, 是换行);
                llvm::Value* 空终止 = llvm::ConstantInt::get(llvm::Type::getInt8Ty(上下文), 0);
                llvm::Value* 原始字节 = 构建器->CreateLoad(llvm::Type::getInt8Ty(上下文), 最后字符地址, "原始字节");
                llvm::Value* 最终字节 = 构建器->CreateSelect(应替换, 空终止, 原始字节, "最终字节");
                构建器->CreateStore(最终字节, 最后字符地址);
                return 缓冲区;
            }

            // 随机数函数
            if (调用.函数名 == "随机数") {
                llvm::FunctionType* rand类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文), {}, false);
                模块->getOrInsertFunction("rand", rand类型);
                return 构建器->CreateCall(模块->getFunction("rand"), {}, "随机数");
            }

            // 类型检查函数
            if (调用.函数名 == "类型") {
                llvm::Value* val = 生成表达式(*调用.参数列表[0]);
                if (val->getType()->isDoubleTy()) {
                    return 构建器->CreateGlobalString("浮点数");
                } else if (val->getType()->isPointerTy()) {
                    return 构建器->CreateGlobalString("字符串");
                } else {
                    return 构建器->CreateGlobalString("整数");
                }
            }

            // 转整数函数
            if (调用.函数名 == "转整数") {
                llvm::Value* val = 生成表达式(*调用.参数列表[0]);
                if (val->getType()->isDoubleTy()) {
                    return 构建器->CreateFPToSI(val, llvm::Type::getInt32Ty(上下文), "浮点转整数");
                }
                if (val->getType()->isPointerTy()) {
                    llvm::FunctionType* atoi类型 = llvm::FunctionType::get(
                        llvm::Type::getInt32Ty(上下文), {llvm::PointerType::get(上下文, 0)}, false);
                    模块->getOrInsertFunction("atoi", atoi类型);
                    return 构建器->CreateCall(模块->getFunction("atoi"), {val}, "atoi结果");
                }
                return val;
            }

            // 转浮点函数
            if (调用.函数名 == "转浮点") {
                llvm::Value* val = 生成表达式(*调用.参数列表[0]);
                if (val->getType()->isIntegerTy()) {
                    return 构建器->CreateSIToFP(val, llvm::Type::getDoubleTy(上下文), "整数转浮点");
                }
                if (val->getType()->isPointerTy()) {
                    llvm::FunctionType* atof类型 = llvm::FunctionType::get(
                        llvm::Type::getDoubleTy(上下文), {llvm::PointerType::get(上下文, 0)}, false);
                    模块->getOrInsertFunction("atof", atof类型);
                    return 构建器->CreateCall(模块->getFunction("atof"), {val}, "atof结果");
                }
                return val;
            }

            // 字符串替换函数
            if (调用.函数名 == "替换") {
                llvm::Value* 原串 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 旧串 = 生成表达式(*调用.参数列表[1]);
                llvm::Value* 新串 = 生成表达式(*调用.参数列表[2]);

                llvm::FunctionType* malloc类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0), {llvm::Type::getInt64Ty(上下文)}, false);
                模块->getOrInsertFunction("malloc", malloc类型);
                llvm::FunctionType* strstr类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0), {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("strstr", strstr类型);
                llvm::FunctionType* strlen类型 = llvm::FunctionType::get(
                    llvm::Type::getInt64Ty(上下文), {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("strlen", strlen类型);
                llvm::FunctionType* strcpy类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0), {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("strcpy", strcpy类型);
                llvm::FunctionType* strcat类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0), {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("strcat", strcat类型);
                llvm::FunctionType* strncpy类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0), {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0), llvm::Type::getInt64Ty(上下文)}, false);
                模块->getOrInsertFunction("strncpy", strncpy类型);

                // 动态计算缓冲区大小: 原串长度 * (新串长度 + 1) + 1 作为安全上界
                llvm::Value* 原串长度 = 构建器->CreateCall(模块->getFunction("strlen"), {原串}, "原串长度");
                llvm::Value* 新串长度 = 构建器->CreateCall(模块->getFunction("strlen"), {新串}, "新串长度");
                llvm::Value* 新串加一 = 构建器->CreateAdd(新串长度, llvm::ConstantInt::get(上下文, llvm::APInt(64, 1)));
                llvm::Value* 缓冲区大小 = 构建器->CreateAdd(
                    构建器->CreateMul(原串长度, 新串加一),
                    llvm::ConstantInt::get(上下文, llvm::APInt(64, 1)), "缓冲区大小");
                llvm::Value* 缓冲区 = 构建器->CreateCall(模块->getFunction("malloc"), {缓冲区大小}, "替换缓冲区");
                llvm::Value* 空串 = 构建器->CreateGlobalString("");
                构建器->CreateCall(模块->getFunction("strcpy"), {缓冲区, 空串});

                llvm::Value* 当前位置 = 原串;

                llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();
                llvm::BasicBlock* 循环块 = llvm::BasicBlock::Create(上下文, "替换循环", 当前函数);
                llvm::BasicBlock* 复制块 = llvm::BasicBlock::Create(上下文, "替换复制", 当前函数);
                llvm::BasicBlock* 合并块 = llvm::BasicBlock::Create(上下文, "替换合并", 当前函数);

                llvm::BasicBlock* 入口块 = 构建器->GetInsertBlock();
                构建器->CreateBr(循环块);
                构建器->SetInsertPoint(循环块);

                llvm::PHINode* 位置phi = 构建器->CreatePHI(llvm::PointerType::get(上下文, 0), 2, "当前位置");
                位置phi->addIncoming(当前位置, 入口块);

                llvm::Value* 旧串长度 = 构建器->CreateCall(模块->getFunction("strlen"), {旧串}, "旧串长度");

                llvm::Value* 找到位置 = 构建器->CreateCall(模块->getFunction("strstr"), {位置phi, 旧串}, "找到位置");
                llvm::Value* 空指针 = llvm::ConstantPointerNull::get(llvm::PointerType::get(上下文, 0));
                llvm::Value* 找到 = 构建器->CreateICmpNE(找到位置, 空指针, "找到");
                构建器->CreateCondBr(找到, 复制块, 合并块);

                构建器->SetInsertPoint(复制块);
                llvm::Value* 前缀长度 = 构建器->CreateSub(
                    构建器->CreatePtrToInt(找到位置, llvm::Type::getInt64Ty(上下文)),
                    构建器->CreatePtrToInt(位置phi, llvm::Type::getInt64Ty(上下文)), "前缀长度");
                构建器->CreateCall(模块->getFunction("strncpy"), {
                    构建器->CreateInBoundsGEP(llvm::Type::getInt8Ty(上下文), 缓冲区,
                        构建器->CreateCall(模块->getFunction("strlen"), {缓冲区}, "缓冲区长度")),
                    位置phi, 前缀长度});
                llvm::Value* 缓冲区长度 = 构建器->CreateCall(模块->getFunction("strlen"), {缓冲区}, "缓冲区长度2");
                llvm::Value* 终止位置 = 构建器->CreateInBoundsGEP(llvm::Type::getInt8Ty(上下文), 缓冲区, 缓冲区长度);
                构建器->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt8Ty(上下文), 0), 终止位置);
                构建器->CreateCall(模块->getFunction("strcat"), {缓冲区, 新串});

                llvm::Value* 下一位置 = 构建器->CreateInBoundsGEP(llvm::Type::getInt8Ty(上下文), 找到位置, 旧串长度);
                位置phi->addIncoming(下一位置, 构建器->GetInsertBlock());
                构建器->CreateBr(循环块);

                构建器->SetInsertPoint(合并块);
                构建器->CreateCall(模块->getFunction("strcat"), {缓冲区, 位置phi});
                return 缓冲区;
            }

            // 用户自定义函数
            llvm::Function* 目标函数 = 模块->getFunction(调用.函数名);
            const 闭包信息* 闭包 = nullptr;
            if (!目标函数) { 闭包 = 符号表实例.获取闭包信息(调用.函数名); if (闭包) 目标函数 = 闭包->函数; }
            if (!目标函数) throw std::runtime_error("未定义的函数: " + 调用.函数名);
            std::vector<llvm::Value*> 参数值;
            size_t 参数索引 = 0;
            for (const auto& 参数 : 调用.参数列表) {
                llvm::Value* 值 = 生成表达式(*参数);
                if (参数->类型 == 表达式类型::变量 && 符号表实例.是数组(static_cast<const 变量表达式&>(*参数).名称)) {
                    参数值.push_back(值);
                } else {
                    llvm::Type* 参数LLVM类型 = llvm::Type::getInt32Ty(上下文);
                    if (参数索引 < 目标函数->arg_size()) {
                        参数LLVM类型 = 目标函数->getArg(参数索引)->getType();
                    }
                    llvm::AllocaInst* 临时 = 构建器->CreateAlloca(参数LLVM类型, nullptr, "参数临时");
                    构建器->CreateStore(值, 临时);
                    参数值.push_back(临时);
                }
                参数索引++;
            }
            if (闭包) for (const auto& 地址 : 闭包->捕获变量地址) 参数值.push_back(地址);
            if (目标函数->getReturnType()->isVoidTy()) return 构建器->CreateCall(目标函数, 参数值);
            return 构建器->CreateCall(目标函数, 参数值, 调用.函数名 + "_结果");
        }
        case 表达式类型::匿名函数: {
            const auto& 匿名 = static_cast<const 匿名函数表达式&>(表达式);
            std::string 函数名 = 符号表实例.生成匿名函数名();
            std::vector<std::string> 参数名列表;
            for (const auto& 参数 : 匿名.参数列表) 参数名列表.push_back(参数.名称);
            std::vector<std::string> 自由变量;
            for (const auto& 子 : 匿名.主体) 收集语句自由变量(*子, 自由变量, 参数名列表);
            std::vector<std::string> 捕获变量名;
            std::vector<llvm::Value*> 捕获变量地址;
            for (const auto& 变量名 : 自由变量) { llvm::Value* 地址 = 符号表实例.获取变量值(变量名); if (地址) { 捕获变量名.push_back(变量名); 捕获变量地址.push_back(地址); } }
            std::vector<llvm::Type*> 参数类型;
            for (size_t i = 0; i < 匿名.参数列表.size(); ++i) {
                if (!匿名.参数列表[i].类型.empty()) {
                    参数类型.push_back(类型名到LLVM类型(匿名.参数列表[i].类型));
                } else {
                    参数类型.push_back(llvm::PointerType::get(上下文, 0));
                }
            }
            for (size_t i = 0; i < 捕获变量名.size(); ++i) 参数类型.push_back(llvm::PointerType::get(上下文, 0));
            llvm::Type* 返回LLVM类型 = llvm::Type::getVoidTy(上下文);
            if (!匿名.返回类型.empty() && 匿名.返回类型 != "空") {
                返回LLVM类型 = 类型名到LLVM类型(匿名.返回类型);
            }
            llvm::FunctionType* 函数类型 = llvm::FunctionType::get(返回LLVM类型, 参数类型, false);
            llvm::Function* 匿名函数 = llvm::Function::Create(函数类型, llvm::Function::InternalLinkage, 函数名, *模块);
            llvm::BasicBlock* 入口块 = llvm::BasicBlock::Create(上下文, "entry", 匿名函数);
            llvm::IRBuilder<> 匿名构建器(上下文);
            匿名构建器.SetInsertPoint(入口块);
            符号表 保存表 = 符号表实例;
            符号表实例 = 符号表();
            auto 参数迭代器 = 匿名函数->args().begin();
            for (size_t i = 0; i < 匿名.参数列表.size(); ++i, ++参数迭代器) {
                参数迭代器->setName(匿名.参数列表[i].名称);
                符号表实例.声明变量(匿名.参数列表[i].名称, &*参数迭代器);
                if (匿名.参数列表[i].类型 == "浮点" || 匿名.参数列表[i].类型 == "浮点数") {
                    符号表实例.设置浮点变量(匿名.参数列表[i].名称);
                } else if (匿名.参数列表[i].类型 == "字符串") {
                    符号表实例.设置指针变量(匿名.参数列表[i].名称);
                }
            }
            for (size_t i = 0; i < 捕获变量名.size(); ++i, ++参数迭代器) {
                参数迭代器->setName(捕获变量名[i] + "_ptr");
                符号表实例.声明变量(捕获变量名[i], &*参数迭代器);
            }
            auto 保存构建器 = std::move(构建器);
            构建器 = std::make_unique<llvm::IRBuilder<>>(入口块);
            for (const auto& 子 : 匿名.主体) 生成语句(*子);
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) {
                if (!匿名.返回类型.empty() && 匿名.返回类型 != "空") {
                    构建器->CreateRet(llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)));
                } else {
                    构建器->CreateRetVoid();
                }
            }
            构建器 = std::move(保存构建器);
            符号表实例 = 保存表;
            llvm::verifyFunction(*匿名函数);
            符号表实例.声明函数指针(函数名, 匿名函数, 捕获变量名, 捕获变量地址);
            return 匿名函数;
        }
        case 表达式类型::数组字面量: {
            const auto& 数组 = static_cast<const 数组字面量表达式&>(表达式);
            int 大小 = static_cast<int>(数组.元素列表.size());
            bool 含字符串 = false;
            for (const auto& 元素 : 数组.元素列表) {
                if (元素->类型 == 表达式类型::字符串) { 含字符串 = true; break; }
            }
            if (含字符串) {
                llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::PointerType::get(上下文, 0), 大小);
                llvm::AllocaInst* 分配 = 构建器->CreateAlloca(数组类型, nullptr, "字符串数组");
                for (int i = 0; i < 大小; i++) {
                    llvm::Value* 索引 = llvm::ConstantInt::get(上下文, llvm::APInt(32, i));
                    llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(数组类型, 分配, {llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 索引}, "元素");
                    构建器->CreateStore(生成表达式(*数组.元素列表[i]), 元素地址);
                }
                return 分配;
            }
            llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::Type::getInt32Ty(上下文), 大小);
            llvm::AllocaInst* 分配 = 构建器->CreateAlloca(数组类型, nullptr, "数组");
            for (int i = 0; i < 大小; i++) {
                llvm::Value* 索引 = llvm::ConstantInt::get(上下文, llvm::APInt(32, i));
                llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(数组类型, 分配, {llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 索引}, "元素");
                构建器->CreateStore(生成表达式(*数组.元素列表[i]), 元素地址);
            }
            return 分配;
        }
        case 表达式类型::映射字面量: {
            const auto& 映射 = static_cast<const 映射字面量表达式&>(表达式);
            int 大小 = static_cast<int>(映射.键值对列表.size());
            // 创建映射结构体：包含键数组和值数组
            llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::PointerType::get(上下文, 0), 256);
            llvm::StructType* 映射结构体类型 = llvm::StructType::create(上下文, {数组类型, 数组类型}, "映射类型");
            static int 映射计数 = 0;
            std::string 前缀 = ".映射" + std::to_string(映射计数++);
            auto* 映射全局 = new llvm::GlobalVariable(*模块, 映射结构体类型, false, llvm::GlobalValue::InternalLinkage,
                llvm::ConstantAggregateZero::get(映射结构体类型), 前缀);
            llvm::Value* 零 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
            llvm::Value* 键数组地址 = 构建器->CreateInBoundsGEP(映射结构体类型, 映射全局, {零, 零}, "键数组地址");
            llvm::Value* 值数组地址 = 构建器->CreateInBoundsGEP(映射结构体类型, 映射全局, {零, llvm::ConstantInt::get(上下文, llvm::APInt(32, 1))}, "值数组地址");
            // 初始化大小为0
            llvm::AllocaInst* 大小变量 = 构建器->CreateAlloca(llvm::Type::getInt32Ty(上下文), nullptr, "映射大小");
            构建器->CreateStore(llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 大小变量);
            // 设置键值对
            for (int i = 0; i < 大小; i++) {
                llvm::Value* 键 = 生成表达式(*映射.键值对列表[i].first);
                llvm::Value* 值 = 生成表达式(*映射.键值对列表[i].second);
                llvm::Value* 当前大小 = 构建器->CreateLoad(llvm::Type::getInt32Ty(上下文), 大小变量);
                llvm::Value* 新大小 = 构建器->CreateCall(模块->getFunction("映射设置"),
                    {键数组地址, 值数组地址, 当前大小, 键, 值});
                构建器->CreateStore(新大小, 大小变量);
            }
            // 返回映射结构体指针
            return 映射全局;
        }
        case 表达式类型::下标访问: {
            const auto& 下标 = static_cast<const 下标访问表达式&>(表达式);
            llvm::Value* 数组地址 = 符号表实例.获取变量值(下标.数组名);
            if (!数组地址) throw std::runtime_error("未定义的变量: " + 下标.数组名);
            if (符号表实例.是映射变量(下标.数组名)) {
                // 映射访问：从结构体中获取键数组和值数组
                llvm::Value* 键 = 生成表达式(*下标.索引);
                llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::PointerType::get(上下文, 0), 256);
                llvm::StructType* 映射结构体类型 = llvm::StructType::get(上下文, {数组类型, 数组类型});
                llvm::Value* 零 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                llvm::Value* 键数组地址 = 构建器->CreateInBoundsGEP(映射结构体类型, 数组地址, {零, 零}, "键数组地址");
                llvm::Value* 值数组地址 = 构建器->CreateInBoundsGEP(映射结构体类型, 数组地址, {零, llvm::ConstantInt::get(上下文, llvm::APInt(32, 1))}, "值数组地址");
                llvm::Value* 大小 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 256));
                return 构建器->CreateCall(模块->getFunction("映射获取"), {键数组地址, 值数组地址, 大小, 键});
            }
            // 普通数组访问
            llvm::Value* 索引值 = 生成表达式(*下标.索引);
            if (符号表实例.是字符串数组(下标.数组名)) {
                llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::PointerType::get(上下文, 0), 符号表实例.获取数组大小(下标.数组名));
                llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(数组类型, 数组地址, {llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 索引值}, 下标.数组名 + "_元素");
                return 构建器->CreateLoad(llvm::PointerType::get(上下文, 0), 元素地址, 下标.数组名 + "_值");
            }
            llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(llvm::Type::getInt32Ty(上下文), 数组地址, 索引值, 下标.数组名 + "_元素");
            return 构建器->CreateLoad(llvm::Type::getInt32Ty(上下文), 元素地址, 下标.数组名 + "_值");
        }
        case 表达式类型::结构体实例: {
            const auto& 实例 = static_cast<const 结构体实例表达式&>(表达式);
            auto it = 结构体类型映射.find(实例.结构体名);
            if (it == 结构体类型映射.end()) throw std::runtime_error("未定义的结构体: " + 实例.结构体名);
            llvm::StructType* 结构体类型 = it->second;
            llvm::AllocaInst* 分配 = 构建器->CreateAlloca(结构体类型, nullptr, 实例.结构体名 + "_实例");
            for (size_t i = 0; i < 实例.成员列表.size(); i++) {
                llvm::Value* 成员地址 = 构建器->CreateStructGEP(结构体类型, 分配, static_cast<unsigned>(i), 实例.成员列表[i].first);
                构建器->CreateStore(生成表达式(*实例.成员列表[i].second), 成员地址);
            }
            return 分配;
        }
        case 表达式类型::成员访问: {
            const auto& 访问 = static_cast<const 成员访问表达式&>(表达式);
            llvm::Value* 对象地址 = 符号表实例.获取变量值(访问.对象名);
            if (!对象地址) throw std::runtime_error("未定义的变量: " + 访问.对象名);
            // 从符号表获取结构体类型名
            const std::string& 结构体名 = 符号表实例.获取结构体类型(访问.对象名);
            if (结构体名.empty()) throw std::runtime_error("变量不是结构体: " + 访问.对象名);
            auto it = 结构体类型映射.find(结构体名);
            if (it == 结构体类型映射.end()) throw std::runtime_error("未定义的结构体: " + 结构体名);
            llvm::StructType* 结构体类型 = it->second;
            const auto& 成员名列表 = 结构体成员映射[结构体名];
            int 成员索引 = -1;
            for (size_t i = 0; i < 成员名列表.size(); i++) {
                if (成员名列表[i] == 访问.成员名) { 成员索引 = static_cast<int>(i); break; }
            }
            if (成员索引 < 0) throw std::runtime_error("未定义的成员: " + 访问.成员名);
            llvm::Value* 成员地址 = 构建器->CreateStructGEP(结构体类型, 对象地址, static_cast<unsigned>(成员索引), 访问.成员名);
            llvm::Type* 成员LLVM类型 = 结构体类型->getElementType(static_cast<unsigned>(成员索引));
            return 构建器->CreateLoad(成员LLVM类型, 成员地址, 访问.成员名);
        }
        case 表达式类型::枚举成员: {
            const auto& 枚举成员 = static_cast<const 枚举成员表达式&>(表达式);
            int 值 = 符号表实例.获取枚举成员值(枚举成员.枚举名, 枚举成员.成员名);
            if (值 < 0) throw std::runtime_error("未定义的枚举成员: " + 枚举成员.枚举名 + "::" + 枚举成员.成员名);
            return llvm::ConstantInt::get(上下文, llvm::APInt(32, 值));
        }
        case 表达式类型::元组字面量: {
            const auto& 元组 = static_cast<const 元组字面量表达式&>(表达式);
            int 大小 = static_cast<int>(元组.元素列表.size());
            // 先收集所有元素值
            std::vector<llvm::Value*> 元素值列表;
            std::vector<llvm::Type*> 元素类型;
            for (int i = 0; i < 大小; i++) {
                llvm::Value* 元素值 = 生成表达式(*元组.元素列表[i]);
                元素值列表.push_back(元素值);
                元素类型.push_back(元素值->getType());
            }
            llvm::StructType* 元组类型 = llvm::StructType::create(上下文, 元素类型, "元组");
            llvm::AllocaInst* 分配 = 构建器->CreateAlloca(元组类型, nullptr, "元组实例");
            for (int i = 0; i < 大小; i++) {
                llvm::Value* 元素地址 = 构建器->CreateStructGEP(元组类型, 分配, static_cast<unsigned>(i), "元素" + std::to_string(i));
                构建器->CreateStore(元素值列表[i], 元素地址);
            }
            return 分配;
        }
        case 表达式类型::元组访问: {
            const auto& 访问 = static_cast<const 元组访问表达式&>(表达式);
            llvm::Value* 元组地址 = 符号表实例.获取变量值(访问.元组名);
            if (!元组地址) throw std::runtime_error("未定义的变量: " + 访问.元组名);
            auto 类型it = 元组类型映射.find(访问.元组名);
            if (类型it == 元组类型映射.end()) throw std::runtime_error("变量不是元组: " + 访问.元组名);
            llvm::StructType* 结构体类型 = 类型it->second;
            if (访问.索引 < 0 || 访问.索引 >= static_cast<int>(结构体类型->getNumElements())) {
                throw std::runtime_error("元组索引越界: " + std::to_string(访问.索引));
            }
            llvm::Value* 元素地址 = 构建器->CreateStructGEP(结构体类型, 元组地址, static_cast<unsigned>(访问.索引), "元组元素");
            return 构建器->CreateLoad(结构体类型->getElementType(static_cast<unsigned>(访问.索引)), 元素地址);
        }
        default:
            throw std::runtime_error("未知表达式类型");
    }
}

void 代码生成器::生成可执行文件() {
    std::error_code 错误码;
    llvm::raw_fd_ostream ir文件(输出文件名 + ".ll", 错误码, llvm::sys::fs::OF_None);
    if (错误码) throw std::runtime_error("无法写入IR文件: " + 错误码.message());
    模块->print(ir文件, nullptr);
    ir文件.close();
    std::string llc命令 = "llc " + 输出文件名 + ".ll -o " + 输出文件名 + ".s";
    if (执行命令(llc命令) != 0) throw std::runtime_error("llc编译失败");
    // 链接时包含运行时辅助函数
    std::string 链接命令 = "clang " + 输出文件名 + ".s " + "源/运行时辅助.c" + " -o " + 输出文件名;
    if (执行命令(链接命令) != 0) throw std::runtime_error("链接失败");
}

#include "代码生成器_内部.h"

std::unordered_map<std::string, const 函数*> 全局函数定义映射;

代码生成器::代码生成器(llvm::LLVMContext& ctx, const std::string& 输出文件, bool 优化)
    : 上下文(ctx), 输出文件名(输出文件), 启用优化(优化) {
    模块 = std::make_unique<llvm::Module>("程序", 上下文);
    构建器 = std::make_unique<llvm::IRBuilder<>>(上下文);
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
}

llvm::Type* 代码生成器::类型名到LLVM类型(const std::string& 类型名) {
    // 解析类型别名
    std::string 解析后类型名 = 类型名;
    auto 别名It = 类型别名映射.find(类型名);
    if (别名It != 类型别名映射.end()) {
        解析后类型名 = 别名It->second;
    }
    // 处理数组类型后缀
    if (解析后类型名.size() > 2 && 解析后类型名.substr(解析后类型名.size() - 2) == "[]") {
        return llvm::PointerType::get(上下文, 0);  // 数组类型返回指针
    }
    if (解析后类型名 == "浮点" || 解析后类型名 == "浮点数") return llvm::Type::getDoubleTy(上下文);
    if (解析后类型名 == "布尔" || 解析后类型名 == "布尔值") return llvm::Type::getInt1Ty(上下文);
    if (解析后类型名 == "字符串" || 解析后类型名 == "函数") return llvm::PointerType::get(上下文, 0);
    // 检查是否为结构体类型
    if (结构体类型映射.count(解析后类型名)) return llvm::PointerType::get(上下文, 0);
    return llvm::Type::getInt32Ty(上下文);
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

    C运行时函数::预声明全部(上下文, *模块);

    // 创建全局文件句柄表
    文件表类型 = llvm::ArrayType::get(llvm::PointerType::get(上下文, 0), 256);
    文件表指针 = new llvm::GlobalVariable(*模块, 文件表类型, false, llvm::GlobalValue::InternalLinkage,
        llvm::ConstantAggregateZero::get(文件表类型), "__文件表");
    文件句柄计数器 = new llvm::GlobalVariable(*模块, llvm::Type::getInt32Ty(上下文), false,
        llvm::GlobalValue::InternalLinkage, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "__文件句柄计数器");

    // 预声明运行时辅助函数
    llvm::Type* i8Ptr = llvm::PointerType::get(上下文, 0);
    llvm::Type* i32 = llvm::Type::getInt32Ty(上下文);

    llvm::FunctionType* 字符位置到字节位置类型 = llvm::FunctionType::get(i32, {i8Ptr, i32}, false);
    模块->getOrInsertFunction("字符位置到字节位置", 字符位置到字节位置类型);

    llvm::FunctionType* 获取字符数类型 = llvm::FunctionType::get(i32, {i8Ptr}, false);
    模块->getOrInsertFunction("获取字符数", 获取字符数类型);

    llvm::FunctionType* 映射查找类型 = llvm::FunctionType::get(i32, {i8Ptr, i32, i8Ptr}, false);
    模块->getOrInsertFunction("映射查找", 映射查找类型);

    llvm::FunctionType* 映射设置类型 = llvm::FunctionType::get(i32, {i8Ptr, i8Ptr, i32, i8Ptr, i8Ptr}, false);
    模块->getOrInsertFunction("映射设置", 映射设置类型);

    llvm::FunctionType* 映射获取类型 = llvm::FunctionType::get(i8Ptr, {i8Ptr, i8Ptr, i32, i8Ptr}, false);
    模块->getOrInsertFunction("映射获取", 映射获取类型);

    for (const auto& 全局变量 : 程序.全局变量) {
        if (全局变量->初始值 && 全局变量->初始值->类型 == 表达式类型::字符串) {
            const auto& str = static_cast<const 字符串表达式&>(*全局变量->初始值);
            llvm::Constant* strConst = llvm::ConstantDataArray::getString(上下文, str.值);
            auto* 全局 = new llvm::GlobalVariable(*模块, strConst->getType(), true,
                llvm::GlobalValue::InternalLinkage, strConst, ".str." + 全局变量->变量名);
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

    // 处理类型别名
    for (const auto& [别名, 原始类型] : 程序.类型别名列表) {
        类型别名映射[别名] = 原始类型;
        调试打印("[代码生成器] 定义类型别名: " << 别名 << " -> " << 原始类型);
    }

    // 处理枚举定义
    for (const auto& [名称, 成员列表] : 程序.枚举定义列表) {
        符号表实例.声明枚举(名称, 成员列表);
        调试打印("[代码生成器] 定义枚举: " << 名称 << " 成员数: " << 成员列表.size());
    }

    // 第一遍：声明所有函数签名（支持前向引用）
    全局函数定义映射.clear();
    for (const auto& 函数 : 程序.函数列表) {
        全局函数定义映射[函数->名称] = 函数.get();
        // 跳过泛型函数（在调用时单态化）
        if (!函数->类型参数列表.empty()) continue;
        std::vector<llvm::Type*> 参数类型;
        for (size_t i = 0; i < 函数->参数列表.size(); ++i) {
            if (!函数->参数列表[i].类型.empty()) {
                参数类型.push_back(类型名到LLVM类型(函数->参数列表[i].类型));
            } else {
                参数类型.push_back(llvm::Type::getInt32Ty(上下文));
            }
        }
        llvm::Type* 返回LLVM类型 = 计算返回类型(上下文, 函数->返回值列表, [this](const std::string& t) { return 类型名到LLVM类型(t); });
        llvm::FunctionType* 函数类型 = llvm::FunctionType::get(返回LLVM类型, 参数类型, false);
        llvm::Function::Create(函数类型, llvm::Function::ExternalLinkage, 函数->名称, *模块);
    }

    // 第二遍：生成函数体（跳过泛型函数）
    for (const auto& 函数 : 程序.函数列表) {
        if (!函数->类型参数列表.empty()) continue;
        生成函数(*函数);
    }
    调试打印("[代码生成器] 函数生成完成");

    if (!程序.函数列表.empty()) {
        llvm::Function* 入口函数 = 模块->getFunction(程序.函数列表.back()->名称);
        if (入口函数) {
            llvm::FunctionType* main类型 = llvm::FunctionType::get(llvm::Type::getInt32Ty(上下文), false);
            llvm::Function* main函数 = llvm::Function::Create(main类型, llvm::Function::ExternalLinkage, "main", *模块);
            llvm::Function* 人格函数 = 模块->getFunction("__gxx_personality_seh0");
            if (人格函数) {
                main函数->setPersonalityFn(llvm::ConstantExpr::getBitCast(人格函数, llvm::PointerType::get(上下文, 0)));
            }
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
        std::vector<llvm::Type*> 参数类型;
        for (size_t i = 0; i < 函数.参数列表.size(); ++i) {
            if (函数.参数列表[i].是否变长) {
                // 变长参数作为数组（指针）传递
                参数类型.push_back(llvm::PointerType::get(上下文, 0));
            } else if (!函数.参数列表[i].类型.empty()) {
                参数类型.push_back(类型名到LLVM类型(函数.参数列表[i].类型));
            } else {
                参数类型.push_back(llvm::Type::getInt32Ty(上下文));
            }
        }
        // 计算返回类型
        llvm::Type* 返回LLVM类型 = 计算返回类型(上下文, 函数.返回值列表, [this](const std::string& t) { return 类型名到LLVM类型(t); });
        llvm::FunctionType* 函数类型 = llvm::FunctionType::get(返回LLVM类型, 参数类型, false);
        llvm函数 = llvm::Function::Create(函数类型, llvm::Function::ExternalLinkage, 函数.名称, *模块);
    }

    llvm::Function* 人格函数 = 模块->getFunction("__gxx_personality_seh0");
    if (人格函数) {
        llvm函数->setPersonalityFn(llvm::ConstantExpr::getBitCast(人格函数, llvm::PointerType::get(上下文, 0)));
    }

    llvm::BasicBlock* 入口块 = llvm::BasicBlock::Create(上下文, "entry", llvm函数);
    构建器->SetInsertPoint(入口块);
    符号表实例.重置为全局作用域();

    // 函数体独立作用域（RAII守卫）
    {
        作用域守卫 守卫(符号表实例);

        size_t i = 0;
        for (auto& 参数 : llvm函数->args()) {
            参数.setName(函数.参数列表[i].名称);

            // 为参数创建 alloca
            llvm::Type* 参数类型;
            if (函数.参数列表[i].是否变长) {
                参数类型 = llvm::PointerType::get(上下文, 0);
            } else if (!函数.参数列表[i].类型.empty()) {
                参数类型 = 类型名到LLVM类型(函数.参数列表[i].类型);
            } else {
                参数类型 = llvm::Type::getInt32Ty(上下文);
            }
            llvm::AllocaInst* 分配 = 构建器->CreateAlloca(参数类型, nullptr, 函数.参数列表[i].名称);
            构建器->CreateStore(&参数, 分配);
            符号表实例.声明变量(函数.参数列表[i].名称, 分配);

            if (函数.参数列表[i].是否变长) {
                // 变长参数标记为数组
                符号表实例.声明数组(函数.参数列表[i].名称, 分配, 0);
            } else if (函数.参数列表[i].类型 == "浮点" || 函数.参数列表[i].类型 == "浮点数") {
                符号表实例.设置浮点变量(函数.参数列表[i].名称);
            } else if (函数.参数列表[i].类型 == "字符串") {
                符号表实例.设置指针变量(函数.参数列表[i].名称);
            } else if (结构体类型映射.count(函数.参数列表[i].类型)) {
                符号表实例.设置结构体变量(函数.参数列表[i].名称, 函数.参数列表[i].类型);
            }
            i++;
        }

        // 为返回值创建 alloca（支持多返回值）
        std::vector<llvm::AllocaInst*> 返回值分配列表;
        for (const auto& 返回值 : 函数.返回值列表) {
            if (!返回值.名称.empty() && !返回值.类型.empty() && 返回值.类型 != "空") {
                llvm::Type* 返回LLVM类型 = 类型名到LLVM类型(返回值.类型);
                llvm::AllocaInst* 分配 = 构建器->CreateAlloca(返回LLVM类型, nullptr, 返回值.名称);
                符号表实例.声明变量(返回值.名称, 分配);
                if (返回LLVM类型->isDoubleTy()) {
                    符号表实例.设置浮点变量(返回值.名称);
                } else if (返回LLVM类型->isPointerTy()) {
                    符号表实例.设置指针变量(返回值.名称);
                }
                // 存储默认值
                if (返回值.默认值) {
                    llvm::Value* 默认值 = 生成表达式(*返回值.默认值);
                    构建器->CreateStore(默认值, 分配);
                }
                返回值分配列表.push_back(分配);
            }
        }

        for (const auto& 语句 : 函数.主体) 生成语句(*语句);

        // 自动返回返回值
        if (构建器->GetInsertBlock()->getTerminator() == nullptr) {
            if (返回值分配列表.size() == 1) {
                // 单返回值
                llvm::Type* 返回LLVM类型 = 返回值分配列表[0]->getAllocatedType();
                llvm::Value* 返回值 = 构建器->CreateLoad(返回LLVM类型, 返回值分配列表[0], "返回值");
                构建器->CreateRet(返回值);
            } else if (返回值分配列表.size() > 1) {
                // 多返回值：创建结构体
                std::vector<llvm::Type*> 返回类型列表;
                for (const auto& 分配 : 返回值分配列表) {
                    返回类型列表.push_back(分配->getAllocatedType());
                }
                llvm::StructType* 返回结构体类型 = llvm::StructType::get(上下文, 返回类型列表);
                llvm::Value* 返回结构体 = llvm::UndefValue::get(返回结构体类型);
                for (size_t j = 0; j < 返回值分配列表.size(); j++) {
                    llvm::Value* 值 = 构建器->CreateLoad(返回值分配列表[j]->getAllocatedType(), 返回值分配列表[j], "返回值" + std::to_string(j));
                    返回结构体 = 构建器->CreateInsertValue(返回结构体, 值, {static_cast<unsigned>(j)});
                }
                构建器->CreateRet(返回结构体);
            } else {
                构建器->CreateRetVoid();
            }
        }
    } // 作用域守卫在此析构，自动退出作用域

    llvm::verifyFunction(*llvm函数);
}

void 代码生成器::运行优化Pass() {
    调试打印("[优化] 开始运行优化Pass");

    llvm::legacy::FunctionPassManager FPM(模块.get());

    // 尾递归优化 (Tail Call Elimination)
    // 将尾递归调用转换为循环，避免栈溢出
    FPM.add(llvm::createTailCallEliminationPass());

    // 死代码消除 (Dead Code Elimination)
    // 删除不可达的代码块和无用指令
    FPM.add(llvm::createDeadCodeEliminationPass());

    // 循环优化 (Loop Invariant Code Motion)
    // 将循环不变量外提到循环外
    FPM.add(llvm::createLICMPass());

    // 简化控制流图
    FPM.add(llvm::createCFGSimplificationPass());

    // 内存到寄存器提升
    FPM.add(llvm::createPromoteMemoryToRegisterPass());

    // 指令组合
    FPM.add(llvm::createInstructionCombiningPass());

    // 重新关联表达式
    FPM.add(llvm::createReassociatePass());

    // 全局值编号
    FPM.add(llvm::createGVNPass());

    FPM.doInitialization();

    for (auto& 函数 : *模块) {
        FPM.run(函数);
    }

    FPM.doFinalization();

    调试打印("[优化] 优化Pass完成");
}

void 代码生成器::生成可执行文件() {
    if (启用优化) {
        运行优化Pass();
    }

    std::error_code 错误码;
    llvm::raw_fd_ostream ir文件(输出文件名 + ".ll", 错误码, llvm::sys::fs::OF_None);
    if (错误码) throw std::runtime_error("无法写入IR文件: " + 错误码.message());
    模块->print(ir文件, nullptr);
    ir文件.close();
    std::string llc命令 = "llc " + 输出文件名 + ".ll -o " + 输出文件名 + ".s";
    if (执行命令(llc命令) != 0) throw std::runtime_error("llc编译失败");
    std::string 链接命令 = "clang++ " + 输出文件名 + ".s " + "源/运行时/运行时辅助.o" + " -o " + 输出文件名 + " -lws2_32 -lm -lstdc++";
    if (执行命令(链接命令) != 0) throw std::runtime_error("链接失败");
}

#include "代码生成器_内部.h"

static bool 是终止语句(const 语句& 语句) {
    return 语句.类型 == 语句类型::返回语句 ||
           语句.类型 == 语句类型::中断语句 ||
           语句.类型 == 语句类型::继续语句;
}

llvm::Value* 代码生成器::创建调用(llvm::Function* 函数, llvm::ArrayRef<llvm::Value*> 参数, const std::string& 名称) {
    if (当前异常处理块 && 函数) {
        llvm::BasicBlock* 正常块 = llvm::BasicBlock::Create(上下文, "invoke_ok", 构建器->GetInsertBlock()->getParent());
        auto 结果 = 构建器->CreateInvoke(函数->getFunctionType(), 函数, 正常块, 当前异常处理块, 参数, 名称);
        构建器->SetInsertPoint(正常块);
        return 结果;
    }
    return 构建器->CreateCall(函数, 参数, 名称);
}

llvm::Value* 代码生成器::创建调用(llvm::FunctionCallee 函数, llvm::ArrayRef<llvm::Value*> 参数, const std::string& 名称) {
    if (当前异常处理块) {
        llvm::BasicBlock* 正常块 = llvm::BasicBlock::Create(上下文, "invoke_ok", 构建器->GetInsertBlock()->getParent());
        auto 结果 = 构建器->CreateInvoke(函数, 正常块, 当前异常处理块, 参数, 名称);
        构建器->SetInsertPoint(正常块);
        return 结果;
    }
    return 构建器->CreateCall(函数, 参数, 名称);
}

llvm::Value* 代码生成器::创建调用无返回(llvm::Function* 函数, llvm::ArrayRef<llvm::Value*> 参数) {
    if (当前异常处理块 && 函数) {
        llvm::BasicBlock* 正常块 = llvm::BasicBlock::Create(上下文, "invoke_ok", 构建器->GetInsertBlock()->getParent());
        auto 结果 = 构建器->CreateInvoke(函数->getFunctionType(), 函数, 正常块, 当前异常处理块, 参数);
        构建器->SetInsertPoint(正常块);
        return 结果;
    }
    return 构建器->CreateCall(函数, 参数);
}

llvm::Value* 代码生成器::创建调用无返回(llvm::FunctionCallee 函数, llvm::ArrayRef<llvm::Value*> 参数) {
    if (当前异常处理块) {
        llvm::BasicBlock* 正常块 = llvm::BasicBlock::Create(上下文, "invoke_ok", 构建器->GetInsertBlock()->getParent());
        auto 结果 = 构建器->CreateInvoke(函数, 正常块, 当前异常处理块, 参数);
        构建器->SetInsertPoint(正常块);
        return 结果;
    }
    return 构建器->CreateCall(函数, 参数);
}

llvm::Value* 代码生成器::创建间接调用(llvm::FunctionType* 类型, llvm::Value* 指针, llvm::ArrayRef<llvm::Value*> 参数, const std::string& 名称) {
    if (当前异常处理块) {
        llvm::BasicBlock* 正常块 = llvm::BasicBlock::Create(上下文, "invoke_ok", 构建器->GetInsertBlock()->getParent());
        auto 结果 = 构建器->CreateInvoke(类型, 指针, 正常块, 当前异常处理块, 参数, 名称);
        构建器->SetInsertPoint(正常块);
        return 结果;
    }
    return 构建器->CreateCall(类型, 指针, 参数, 名称);
}

void 代码生成器::生成语句(const 语句& 语句) {
    if (构建器->GetInsertBlock()->getTerminator() != nullptr) return;

    if (语句.行号 > 0) {
        llvm::Function* 记录函数 = 模块->getFunction("记录行执行");
        if (记录函数) {
            创建调用无返回(记录函数, {llvm::ConstantInt::get(llvm::Type::getInt32Ty(上下文), 语句.行号)});
        }
    }

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
                if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(初始值)) {
                    元组类型映射[名称] = llvm::cast<llvm::StructType>(alloca->getAllocatedType());
                }
            } else {
                llvm::Value* 初始值 = 生成表达式(*初始值表达式);
                if (!初始值 || 初始值->getType()->isVoidTy()) {
                    初始值 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                }
                bool 是浮点 = 初始值->getType()->isDoubleTy();
                bool 是指针 = 初始值->getType()->isPointerTy();

                // 类型标注检查（渐进式：仅警告，不报错）
                if (语句.类型 == 语句类型::变量声明) {
                    const auto& 声明 = static_cast<const 变量声明&>(语句);
                    if (!声明.类型标注.empty()) {
                        std::string 标注 = 声明.类型标注;
                        // 递归解析类型别名
                        for (int i = 0; i < 10; i++) {
                            auto it = 类型别名映射.find(标注);
                            if (it == 类型别名映射.end()) break;
                            标注 = it->second;
                        }
                        bool 标注是浮点 = (标注 == "浮点" || 标注 == "浮点数");
                        bool 标注是整数 = (标注 == "整数" || 标注 == "整数类型");
                        bool 标注是字符串 = (标注 == "字符串" || 标注 == "字符串类型");
                        bool 标注是布尔 = (标注 == "布尔" || 标注 == "布尔类型");

                        if (标注是浮点 && !是浮点 && !是指针) {
                            std::cerr << "警告: 变量 '" << 名称 << "' 类型标注为浮点，但初始值是整数（行 " << 语句.行号 << "）" << std::endl;
                        } else if (标注是整数 && 是浮点) {
                            std::cerr << "警告: 变量 '" << 名称 << "' 类型标注为整数，但初始值是浮点（行 " << 语句.行号 << "）" << std::endl;
                        } else if (标注是字符串 && !是指针) {
                            std::cerr << "警告: 变量 '" << 名称 << "' 类型标注为字符串，但初始值不是字符串（行 " << 语句.行号 << "）" << std::endl;
                        }
                    }
                }

                // 检查函数调用是否返回结构体类型
                std::string 结构体类型名;
                if (初始值表达式->类型 == 表达式类型::函数调用) {
                    const auto& 调用 = static_cast<const 函数调用表达式&>(*初始值表达式);
                    调试打印("[codegen] 检查函数返回结构体: " << 调用.函数名);
                    auto 定义迭代 = 全局函数定义映射.find(调用.函数名);
                    if (定义迭代 != 全局函数定义映射.end()) {
                        const 函数* func = 定义迭代->second;
                        调试打印("[codegen] 找到函数: " << func->名称 << " 返回值数: " << func->返回值列表.size());
                        if (!func->返回值列表.empty()) {
                            调试打印("[codegen] 返回类型: " << func->返回值列表[0].类型 << " 是否结构体: " << 结构体类型映射.count(func->返回值列表[0].类型));
                        }
                        if (!func->返回值列表.empty() && 结构体类型映射.count(func->返回值列表[0].类型)) {
                            结构体类型名 = func->返回值列表[0].类型;
                        }
                    }
                }

                // 检查变量是否已存在
                llvm::Value* 已有地址 = 符号表实例.获取变量值(名称);
                if (已有地址) {
                    // 变量已存在，更新值
                    构建器->CreateStore(初始值, 已有地址);
                } else {
                    // 变量不存在，创建新变量
                    llvm::Type* 变量类型;
                    if (!结构体类型名.empty()) 变量类型 = llvm::PointerType::get(上下文, 0);
                    else if (是浮点) 变量类型 = llvm::Type::getDoubleTy(上下文);
                    else if (是指针) 变量类型 = llvm::PointerType::get(上下文, 0);
                    else 变量类型 = llvm::Type::getInt32Ty(上下文);
                    llvm::AllocaInst* 分配 = 构建器->CreateAlloca(变量类型, nullptr, 名称);
                    构建器->CreateStore(初始值, 分配);
                    符号表实例.声明变量(名称, 分配);
                    if (!结构体类型名.empty()) {
                        符号表实例.设置结构体变量(名称, 结构体类型名);
                    } else if (是浮点) {
                        符号表实例.设置浮点变量(名称);
                    } else if (是指针) {
                        符号表实例.设置指针变量(名称);
                    }
                }
                if (!结构体类型名.empty()) {
                    // 已在上面设置，跳过
                } else if (是浮点) {
                    符号表实例.设置浮点变量(名称);
                } else if (是指针) {
                    符号表实例.设置指针变量(名称);
                }
            }
            if (语句.类型 == 语句类型::常量声明) 符号表实例.声明常量(名称);
            break;
        }
        case 语句类型::赋值语句: {
            const auto& 赋值 = static_cast<const 赋值语句&>(语句);
            if (符号表实例.是常量(赋值.变量名)) {
                throw std::runtime_error("不能对常量 '" + 赋值.变量名 + "' 赋值（行 " + std::to_string(语句.行号) + "）");
            }
            size_t 点位置 = 赋值.变量名.find('.');
            if (点位置 != std::string::npos) {
                std::string 对象名 = 赋值.变量名.substr(0, 点位置);
                std::string 成员名 = 赋值.变量名.substr(点位置 + 1);
                llvm::Value* 成员地址 = 查找结构体成员地址(*构建器, 上下文, 对象名, 成员名, 符号表实例, 结构体类型映射, 结构体成员映射);
                构建器->CreateStore(生成表达式(*赋值.值表达式), 成员地址);
            } else {
                llvm::Value* 变量地址 = 符号表实例.获取变量值(赋值.变量名);
                if (!变量地址) throw std::runtime_error("未定义的变量: " + 赋值.变量名 + "（行 " + std::to_string(语句.行号) + "）");
                llvm::Value* 赋值结果 = 生成表达式(*赋值.值表达式);
                if (!赋值结果 || 赋值结果->getType()->isVoidTy()) {
                    赋值结果 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                }
                构建器->CreateStore(赋值结果, 变量地址);
            }
            break;
        }
        case 语句类型::复合赋值: {
            const auto& 复合赋值 = static_cast<const 复合赋值语句&>(语句);
            llvm::Value* 变量地址 = 符号表实例.获取变量值(复合赋值.变量名);
            if (!变量地址) throw std::runtime_error("未定义的变量: " + 复合赋值.变量名 + "（行 " + std::to_string(语句.行号) + "）");
            llvm::Value* 当前值 = 构建器->CreateLoad(llvm::Type::getInt32Ty(上下文), 变量地址, 复合赋值.变量名);
            llvm::Value* 增量值 = 生成表达式(*复合赋值.值表达式);
            llvm::Value* 新值;
            switch (复合赋值.操作) {
                case 二元操作符::加法: 新值 = 构建器->CreateAdd(当前值, 增量值); break;
                case 二元操作符::减法: 新值 = 构建器->CreateSub(当前值, 增量值); break;
                case 二元操作符::乘法: 新值 = 构建器->CreateMul(当前值, 增量值); break;
                case 二元操作符::除法: 新值 = 构建器->CreateSDiv(当前值, 增量值); break;
                case 二元操作符::取模: 新值 = 构建器->CreateSRem(当前值, 增量值); break;
                default: throw std::runtime_error("不支持的复合赋值操作");
            }
            构建器->CreateStore(新值, 变量地址);
            break;
        }
        case 语句类型::下标赋值语句: {
            const auto& 下标赋值 = static_cast<const 下标赋值语句&>(语句);
            if (符号表实例.是常量(下标赋值.数组名)) {
                throw std::runtime_error("不能对常量 '" + 下标赋值.数组名 + "' 赋值（行 " + std::to_string(语句.行号) + "）");
            }
            llvm::Value* 数组地址 = 符号表实例.获取变量值(下标赋值.数组名);
            if (!数组地址) throw std::runtime_error("未定义的数组: " + 下标赋值.数组名 + "（行 " + std::to_string(语句.行号) + "）");
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
            
            // 处理初始化语句（if (初始化; 条件) 形式）
            if (如果.初始化) {
                生成语句(*如果.初始化);
            }
            
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
            {
                作用域守卫 守卫(符号表实例);
                for (const auto& 子 : 如果.then块) 生成语句(*子);
            }
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(合并块);

            for (size_t j = 0; j < 如果.否则如果列表.size(); ++j) {
                构建器->SetInsertPoint(elif块列表[j]);
                llvm::Value* elif条件 = 生成表达式(*如果.否则如果列表[j].条件);
                elif条件 = 构建器->CreateICmpNE(elif条件, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "elifcond");
                llvm::BasicBlock* elif体块 = llvm::BasicBlock::Create(上下文, "elifbody", 当前函数);
                llvm::BasicBlock* 下一个 = (j + 1 < elif块列表.size()) ? elif块列表[j + 1] : else块;
                构建器->CreateCondBr(elif条件, elif体块, 下一个);
                构建器->SetInsertPoint(elif体块);
                {
                    作用域守卫 守卫(符号表实例);
                    for (const auto& 子 : 如果.否则如果列表[j].主体) 生成语句(*子);
                }
                if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(合并块);
            }

            构建器->SetInsertPoint(else块);
            {
                作用域守卫 守卫(符号表实例);
                for (const auto& 子 : 如果.else块) 生成语句(*子);
            }
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(合并块);
            构建器->SetInsertPoint(合并块);
            break;
        }
        case 语句类型::循环语句: {
            const auto& 循环 = static_cast<const 循环语句&>(语句);
            llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();

            std::unique_ptr<作用域守卫> 初始化守卫;
            if (循环.初始化) {
                初始化守卫 = std::make_unique<作用域守卫>(符号表实例);
                生成语句(*循环.初始化);
            }

            llvm::BasicBlock* 条件块 = llvm::BasicBlock::Create(上下文, "forcond", 当前函数);
            llvm::BasicBlock* 体块 = llvm::BasicBlock::Create(上下文, "forbody", 当前函数);
            llvm::BasicBlock* 步进块 = llvm::BasicBlock::Create(上下文, "forstep", 当前函数);
            llvm::BasicBlock* 后块 = llvm::BasicBlock::Create(上下文, "forend", 当前函数);

            循环继续栈.push(步进块);
            循环退出栈.push(后块);

            构建器->CreateBr(条件块);

            构建器->SetInsertPoint(条件块);
            if (循环.条件) {
                llvm::Value* 条件值 = 生成表达式(*循环.条件);
                条件值 = 构建器->CreateICmpNE(条件值, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "forcond");
                构建器->CreateCondBr(条件值, 体块, 后块);
            } else {
                构建器->CreateBr(体块);
            }

            构建器->SetInsertPoint(体块);
            {
                作用域守卫 守卫(符号表实例);
                for (const auto& 子 : 循环.主体) 生成语句(*子);
            }
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(步进块);

            构建器->SetInsertPoint(步进块);
            if (循环.步进) {
                生成语句(*循环.步进);
            }
            构建器->CreateBr(条件块);

            循环继续栈.pop();
            循环退出栈.pop();
            构建器->SetInsertPoint(后块);

            初始化守卫.reset();
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
            {
                作用域守卫 守卫(符号表实例);
                for (const auto& 子 : 当循环.主体) 生成语句(*子);
            }
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(条件块);
            循环继续栈.pop();
            循环退出栈.pop();
            构建器->SetInsertPoint(后块);
            break;
        }
        case 语句类型::做循环语句: {
            const auto& 做循环 = static_cast<const 做循环语句&>(语句);
            llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();
            llvm::BasicBlock* 体块 = llvm::BasicBlock::Create(上下文, "do_body", 当前函数);
            llvm::BasicBlock* 条件块 = llvm::BasicBlock::Create(上下文, "do_cond", 当前函数);
            llvm::BasicBlock* 后块 = llvm::BasicBlock::Create(上下文, "do_end");

            构建器->CreateBr(体块);
            构建器->SetInsertPoint(体块);
            {
                作用域守卫 守卫(符号表实例);
                循环继续栈.push(条件块);
                循环退出栈.push(后块);
                for (const auto& 子语句 : 做循环.主体) 生成语句(*子语句);
            }
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(条件块);

            构建器->SetInsertPoint(条件块);
            llvm::Value* 条件值 = 生成表达式(*做循环.条件);
            if (条件值->getType()->isPointerTy()) {
                条件值 = 构建器->CreatePtrToInt(条件值, llvm::Type::getInt32Ty(上下文));
            }
            llvm::Value* 条件结果 = 构建器->CreateICmpNE(条件值, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "do_cond");
            if (做循环.是当循环) {
                构建器->CreateCondBr(条件结果, 体块, 后块);
            } else {
                构建器->CreateCondBr(条件结果, 后块, 体块);
            }

            循环继续栈.pop();
            循环退出栈.pop();
            当前函数->insert(当前函数->end(), 后块);
            构建器->SetInsertPoint(后块);
            break;
        }
        case 语句类型::中断语句: {
            if (循环退出栈.empty()) throw std::runtime_error("'中断'必须在循环内使用（行 " + std::to_string(语句.行号) + "）");
            构建器->CreateBr(循环退出栈.top());
            llvm::BasicBlock* 新块 = llvm::BasicBlock::Create(上下文, "afterbreak", 构建器->GetInsertBlock()->getParent());
            构建器->SetInsertPoint(新块);
            break;
        }
        case 语句类型::继续语句: {
            if (循环继续栈.empty()) throw std::runtime_error("'继续'必须在循环内使用（行 " + std::to_string(语句.行号) + "）");
            构建器->CreateBr(循环继续栈.top());
            llvm::BasicBlock* 新块 = llvm::BasicBlock::Create(上下文, "aftercontinue", 构建器->GetInsertBlock()->getParent());
            构建器->SetInsertPoint(新块);
            break;
        }
        case 语句类型::返回语句: {
            throw std::runtime_error("不允许使用'返回'关键字，请直接赋值返回值变量（行 " + std::to_string(语句.行号) + "）");
        }
        case 语句类型::打印语句: {
            const auto& 打印 = static_cast<const 打印语句&>(语句);
            llvm::Value* 打印值 = 生成表达式(*打印.值表达式);
            llvm::FunctionType* printf类型 = llvm::FunctionType::get(llvm::Type::getInt32Ty(上下文), {llvm::PointerType::get(上下文, 0)}, true);
            模块->getOrInsertFunction("printf", printf类型);
            if (打印.值表达式->类型 == 表达式类型::字符串) {
                创建调用无返回(模块->getFunction("printf"), {构建器->CreateGlobalString("%s\n"), 打印值});
            } else if (打印值->getType()->isDoubleTy() || 打印值->getType()->isFloatTy()) {
                创建调用无返回(模块->getFunction("printf"), {构建器->CreateGlobalString("%.6g\n"), 打印值});
            } else if (打印值->getType()->isPointerTy()) {
                创建调用无返回(模块->getFunction("printf"), {构建器->CreateGlobalString("%s\n"), 打印值});
            } else if (打印.值表达式->类型 == 表达式类型::下标访问 && 符号表实例.是映射变量(static_cast<const 下标访问表达式&>(*打印.值表达式).数组名)) {
                创建调用无返回(模块->getFunction("printf"), {构建器->CreateGlobalString("%s\n"), 打印值});
            } else {
                创建调用无返回(模块->getFunction("printf"), {构建器->CreateGlobalString("%d\n"), 打印值});
            }
            break;
        }
        case 语句类型::表达式语句:
            生成表达式(*static_cast<const 表达式语句&>(语句).值表达式);
            break;
        case 语句类型::解构赋值: {
            const auto& 解构 = static_cast<const 解构赋值语句&>(语句);
            llvm::Value* 值 = 生成表达式(*解构.值表达式);
            
            // 检查是否为结构体类型（多返回值）
            if (值->getType()->isStructTy()) {
                auto* 结构体类型 = llvm::cast<llvm::StructType>(值->getType());
                for (size_t i = 0; i < 解构.变量名列表.size() && i < 结构体类型->getNumElements(); i++) {
                    llvm::Value* 元素值 = 构建器->CreateExtractValue(值, {static_cast<unsigned>(i)});
                    std::string 变量名 = 解构.变量名列表[i];
                    
                    // 检查变量是否已存在
                    llvm::Value* 已有地址 = 符号表实例.获取变量值(变量名);
                    if (已有地址) {
                        构建器->CreateStore(元素值, 已有地址);
                    } else {
                        llvm::Type* 元素类型 = 结构体类型->getElementType(i);
                        llvm::AllocaInst* 分配 = 构建器->CreateAlloca(元素类型, nullptr, 变量名);
                        构建器->CreateStore(元素值, 分配);
                        符号表实例.声明变量(变量名, 分配);
                        if (元素类型->isDoubleTy()) 符号表实例.设置浮点变量(变量名);
                        else if (元素类型->isPointerTy()) 符号表实例.设置指针变量(变量名);
                    }
                }
            }
            break;
        }
        case 语句类型::代码块: {
            作用域守卫 守卫(符号表实例);
            for (size_t i = 0; i < static_cast<const 代码块语句&>(语句).语句列表.size(); i++) {
                const auto& 子 = static_cast<const 代码块语句&>(语句).语句列表[i];
                生成语句(*子);
                // 死代码消除：如果当前语句是终止语句，跳过后续语句
                if (启用优化 && 是终止语句(*子) && i + 1 < static_cast<const 代码块语句&>(语句).语句列表.size()) {
                    调试打印("[优化] 消除死代码：跳过终止语句后的代码");
                    break;
                }
            }
            break;
        }
        case 语句类型::匹配语句: {
            const auto& 匹配 = static_cast<const 匹配语句&>(语句);
            llvm::Value* 匹配值 = 生成表达式(*匹配.匹配值);
            llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();
            llvm::BasicBlock* 合并块 = llvm::BasicBlock::Create(上下文, "matchend", 当前函数);

            std::vector<llvm::BasicBlock*> 分支块;
            for (size_t i = 0; i < 匹配.分支列表.size(); i++) {
                分支块.push_back(llvm::BasicBlock::Create(上下文, "match_" + std::to_string(i), 当前函数));
            }

            构建器->CreateBr(分支块[0]);

            for (size_t i = 0; i < 匹配.分支列表.size(); i++) {
                构建器->SetInsertPoint(分支块[i]);
                const auto& 分支 = 匹配.分支列表[i];

                if (分支.条件) {
                    llvm::Value* 分支值 = 生成表达式(*分支.条件);
                    llvm::Value* 比较结果;
                    if (匹配值->getType()->isPointerTy() && 分支值->getType()->isPointerTy()) {
                        // 字符串比较：使用 strcmp
                        llvm::Function* strcmp函数 = 模块->getFunction("strcmp");
                        llvm::Value* cmp结果 = 创建调用(strcmp函数, {匹配值, 分支值}, "strcmp结果");
                        比较结果 = 构建器->CreateICmpEQ(cmp结果, llvm::ConstantInt::get(llvm::Type::getInt32Ty(上下文), 0), "streq");
                    } else {
                        比较结果 = 构建器->CreateICmpEQ(匹配值, 分支值, "matchcmp");
                    }

                    llvm::BasicBlock* 下一个块 = (i + 1 < 分支块.size()) ? 分支块[i + 1] : 合并块;
                    llvm::BasicBlock* 执行块 = llvm::BasicBlock::Create(上下文, "match_body_" + std::to_string(i), 当前函数);
                    构建器->CreateCondBr(比较结果, 执行块, 下一个块);

                    构建器->SetInsertPoint(执行块);
                    {
                        作用域守卫 守卫(符号表实例);
                        for (const auto& 子 : 分支.主体) 生成语句(*子);
                    }
                    if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(合并块);
                } else {
                    作用域守卫 守卫(符号表实例);
                    for (const auto& 子 : 分支.主体) 生成语句(*子);
                    if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(合并块);
                }
            }

            构建器->SetInsertPoint(合并块);
            break;
        }
        case 语句类型::遍历语句: {
            const auto& 遍历 = static_cast<const 遍历语句&>(语句);
            llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();

            llvm::Value* 数组地址 = 生成表达式(*遍历.数组表达式);
            int 数组大小值 = 100;
            if (遍历.数组表达式->类型 == 表达式类型::变量) {
                const auto& 变量名 = static_cast<const 变量表达式&>(*遍历.数组表达式).名称;
                int 大小 = 符号表实例.获取数组大小(变量名);
                if (大小 > 0) 数组大小值 = 大小;
            }
            llvm::Value* 数组大小 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 数组大小值));

            llvm::AllocaInst* 索引变量 = 构建器->CreateAlloca(llvm::Type::getInt32Ty(上下文), nullptr, "遍历索引");
            构建器->CreateStore(llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 索引变量);

            llvm::BasicBlock* 条件块 = llvm::BasicBlock::Create(上下文, "forin_cond", 当前函数);
            llvm::BasicBlock* 体块 = llvm::BasicBlock::Create(上下文, "forin_body", 当前函数);
            llvm::BasicBlock* 步进块 = llvm::BasicBlock::Create(上下文, "forin_step", 当前函数);
            llvm::BasicBlock* 后块 = llvm::BasicBlock::Create(上下文, "forin_end", 当前函数);

            循环继续栈.push(步进块);
            循环退出栈.push(后块);

            构建器->CreateBr(条件块);

            构建器->SetInsertPoint(条件块);
            llvm::Value* 当前索引 = 构建器->CreateLoad(llvm::Type::getInt32Ty(上下文), 索引变量, "当前索引");
            llvm::Value* 条件 = 构建器->CreateICmpSLT(当前索引, 数组大小, "遍历条件");
            构建器->CreateCondBr(条件, 体块, 后块);

            构建器->SetInsertPoint(体块);
            {
                作用域守卫 守卫(符号表实例);

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
            }
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(步进块);

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
            break;
        case 语句类型::导入语句:
            break;
        case 语句类型::类型别名:
            break;
        case 语句类型::让出语句: {
            const auto& 让出 = static_cast<const 让出语句&>(语句);
            llvm::Value* 让出值 = 生成表达式(*让出.值表达式);
            if (!返回值变量列表.empty()) {
                构建器->CreateStore(让出值, 返回值变量列表[0]);
            }

            // LLVM coro.suspend intrinsic
            auto* coro_suspend = llvm::Intrinsic::getOrInsertDeclaration(模块.get(), llvm::Intrinsic::coro_suspend);
            auto* token = llvm::ConstantTokenNone::get(上下文);
            auto* suspend结果 = 构建器->CreateCall(coro_suspend, {token, llvm::ConstantInt::getFalse(上下文)}, "yield_suspend");

            llvm::Function* 当前函数体 = 构建器->GetInsertBlock()->getParent();
            auto* resumeBB = llvm::BasicBlock::Create(上下文, "coro.resume", 当前函数体);
            auto* suspendBB = llvm::BasicBlock::Create(上下文, "coro.suspend", 当前函数体);
            auto* destroyBB = llvm::BasicBlock::Create(上下文, "coro.destroy", 当前函数体);

            auto* sw = 构建器->CreateSwitch(suspend结果, suspendBB, 2);
            sw->addCase(llvm::ConstantInt::get(llvm::Type::getInt8Ty(上下文), 0), resumeBB);
            sw->addCase(llvm::ConstantInt::get(llvm::Type::getInt8Ty(上下文), 1), destroyBB);

            // resume: continue execution
            构建器->SetInsertPoint(resumeBB);

            // destroy: cleanup and return
            构建器->SetInsertPoint(destroyBB);
            auto* coro_free = llvm::Intrinsic::getOrInsertDeclaration(模块.get(), llvm::Intrinsic::coro_free);
            auto* coro_end = llvm::Intrinsic::getOrInsertDeclaration(模块.get(), llvm::Intrinsic::coro_end);
            if (coro_free && 协程句柄) {
                auto* free_mem = 构建器->CreateCall(coro_free, {token, 协程句柄}, "coro_free");
                auto* free函数 = 模块->getFunction("free");
                if (free函数) 构建器->CreateCall(free函数, {free_mem});
            }
            构建器->CreateBr(suspendBB);

            // suspend: yield value and return
            构建器->SetInsertPoint(suspendBB);
            if (coro_end && 协程句柄) {
                构建器->CreateCall(coro_end, {协程句柄, llvm::ConstantInt::getFalse(上下文), token});
            }
            if (返回值变量列表.size() == 1) {
                auto* ret = 构建器->CreateLoad(返回值变量列表[0]->getAllocatedType(), 返回值变量列表[0], "yield_val");
                构建器->CreateRet(ret);
            } else {
                构建器->CreateRetVoid();
            }

            // resume block continues here
            构建器->SetInsertPoint(resumeBB);
            break;
        }

        case 语句类型::抛出语句: {
            const auto& 抛出 = static_cast<const 抛出语句&>(语句);
            llvm::Value* 异常值 = 生成表达式(*抛出.异常值);
            llvm::Value* 异常对象 = 创建调用(模块->getFunction("__cxa_allocate_exception"),
                                                        {llvm::ConstantInt::get(llvm::Type::getInt64Ty(上下文), 8)}, "exc_alloc");
            llvm::Value* 存储值 = 异常值;
            if (异常值->getType()->isIntegerTy(32)) {
                llvm::Value* i64值 = 构建器->CreateSExt(异常值, llvm::Type::getInt64Ty(上下文));
                存储值 = 构建器->CreateIntToPtr(i64值, llvm::PointerType::get(上下文, 0));
                存储值 = 构建器->CreatePtrToInt(存储值, llvm::Type::getInt64Ty(上下文));
            } else if (异常值->getType()->isPointerTy()) {
                存储值 = 构建器->CreatePtrToInt(异常值, llvm::Type::getInt64Ty(上下文));
            }
            构建器->CreateStore(存储值, 构建器->CreateBitCast(异常对象, llvm::PointerType::get(上下文, 0)));
            llvm::Function* throw函数 = 模块->getFunction("__cxa_throw");
            llvm::BasicBlock* 正常块 = llvm::BasicBlock::Create(上下文, "throw_ok", 构建器->GetInsertBlock()->getParent());
            llvm::BasicBlock* 异常块 = 当前异常处理块 ? 当前异常处理块 : llvm::BasicBlock::Create(上下文, "throw_unwind", 构建器->GetInsertBlock()->getParent());
            std::vector<llvm::Value*> throw参数 = {异常对象, llvm::Constant::getNullValue(llvm::PointerType::get(上下文, 0)),
                                                     llvm::Constant::getNullValue(llvm::PointerType::get(上下文, 0))};
            构建器->CreateInvoke(throw函数->getFunctionType(), throw函数, 正常块, 异常块, throw参数);
            构建器->SetInsertPoint(正常块);
            构建器->CreateUnreachable();
            break;
        }
        case 语句类型::尝试语句: {
            const auto& 尝试 = static_cast<const 尝试语句&>(语句);
            llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();

            llvm::Function* 人格函数 = 模块->getFunction("__gxx_personality_seh0");
            llvm::Function* 开始捕获函数 = 模块->getFunction("__cxa_begin_catch");
            llvm::Function* 结束捕获函数 = 模块->getFunction("__cxa_end_catch");
            llvm::Function* 恢复函数 = 模块->getFunction("_Unwind_Resume");

            llvm::BasicBlock* 合并块 = llvm::BasicBlock::Create(上下文, "try_merge", 当前函数);
            llvm::BasicBlock* 清理块 = llvm::BasicBlock::Create(上下文, "try_cleanup", 当前函数);
            llvm::BasicBlock* 捕获块 = nullptr;

            llvm::BasicBlock* 最终正常块 = nullptr;
            llvm::BasicBlock* 最终异常块 = nullptr;
            if (!尝试.最终主体.empty()) {
                最终正常块 = llvm::BasicBlock::Create(上下文, "finally_ok", 当前函数);
                最终异常块 = llvm::BasicBlock::Create(上下文, "finally_ex", 当前函数);
            }

            if (!尝试.捕获主体.empty()) {
                捕获块 = llvm::BasicBlock::Create(上下文, "catch", 当前函数);
            }

            llvm::BasicBlock* 旧异常处理块 = 当前异常处理块;
            当前异常处理块 = 清理块;

            for (const auto& 子 : 尝试.尝试主体) 生成语句(*子);

            当前异常处理块 = 旧异常处理块;

            if (构建器->GetInsertBlock()->getTerminator() == nullptr) {
                if (最终正常块) {
                    构建器->CreateBr(最终正常块);
                } else {
                    构建器->CreateBr(合并块);
                }
            }

            构建器->SetInsertPoint(清理块);
            llvm::LandingPadInst* 着陆垫 = 构建器->CreateLandingPad(
                llvm::StructType::get(llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)), 1, "landingpad");
            着陆垫->addClause(llvm::Constant::getNullValue(llvm::PointerType::get(上下文, 0)));
            llvm::Value* 异常指针 = 构建器->CreateExtractValue(着陆垫, 0, "exc_ptr");
            llvm::Value* 选择器 = 构建器->CreateExtractValue(着陆垫, 1, "sel");

            if (捕获块) {
                构建器->CreateBr(捕获块);

                构建器->SetInsertPoint(捕获块);
                llvm::Value* 捕获值 = 构建器->CreateCall(开始捕获函数, {异常指针}, "catch_val");
                llvm::Value* 异常i64 = 构建器->CreateLoad(llvm::Type::getInt64Ty(上下文),
                    构建器->CreateBitCast(捕获值, llvm::PointerType::get(上下文, 0)), "exc_i64");
                llvm::Value* exc_ptr = 构建器->CreateIntToPtr(异常i64, llvm::PointerType::get(上下文, 0), "exc_ptr");
                {
                    作用域守卫 守卫(符号表实例);
                    llvm::AllocaInst* 变量地址 = 构建器->CreateAlloca(llvm::PointerType::get(上下文, 0));
                    符号表实例.声明变量(尝试.异常变量名, 变量地址);
                    符号表实例.设置指针变量(尝试.异常变量名);
                    构建器->CreateStore(exc_ptr, 变量地址);
                    for (const auto& 子 : 尝试.捕获主体) 生成语句(*子);
                }
                if (构建器->GetInsertBlock()->getTerminator() == nullptr) {
                    构建器->CreateCall(结束捕获函数);
                    if (最终正常块) {
                        构建器->CreateBr(最终正常块);
                    } else {
                        构建器->CreateBr(合并块);
                    }
                }
            } else {
                if (!尝试.最终主体.empty()) {
                    构建器->CreateBr(最终异常块);
                } else {
                    构建器->CreateCall(恢复函数, {异常指针});
                    构建器->CreateUnreachable();
                }
            }

            if (最终正常块) {
                构建器->SetInsertPoint(最终正常块);
                for (const auto& 子 : 尝试.最终主体) 生成语句(*子);
                if (构建器->GetInsertBlock()->getTerminator() == nullptr)
                    构建器->CreateBr(合并块);
            }

            if (最终异常块) {
                构建器->SetInsertPoint(最终异常块);
                for (const auto& 子 : 尝试.最终主体) 生成语句(*子);
                if (构建器->GetInsertBlock()->getTerminator() == nullptr) {
                    构建器->CreateCall(恢复函数, {异常指针});
                    构建器->CreateUnreachable();
                }
            }

            构建器->SetInsertPoint(合并块);
            break;
        }
    }
}

#include "代码生成器_内部.h"

static bool 是终止语句(const 语句& 语句) {
    return 语句.类型 == 语句类型::返回语句 ||
           语句.类型 == 语句类型::中断语句 ||
           语句.类型 == 语句类型::继续语句;
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
                if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(初始值)) {
                    元组类型映射[名称] = llvm::cast<llvm::StructType>(alloca->getAllocatedType());
                }
            } else {
                llvm::Value* 初始值 = 生成表达式(*初始值表达式);
                bool 是浮点 = 初始值->getType()->isDoubleTy();
                bool 是指针 = 初始值->getType()->isPointerTy();

                // 检查变量是否已存在
                llvm::Value* 已有地址 = 符号表实例.获取变量值(名称);
                if (已有地址) {
                    // 变量已存在，更新值
                    构建器->CreateStore(初始值, 已有地址);
                } else {
                    // 变量不存在，创建新变量
                    llvm::Type* 变量类型;
                    if (是浮点) 变量类型 = llvm::Type::getDoubleTy(上下文);
                    else if (是指针) 变量类型 = llvm::PointerType::get(上下文, 0);
                    else 变量类型 = llvm::Type::getInt32Ty(上下文);
                    llvm::AllocaInst* 分配 = 构建器->CreateAlloca(变量类型, nullptr, 名称);
                    构建器->CreateStore(初始值, 分配);
                    符号表实例.声明变量(名称, 分配);
                }
                if (是浮点) 符号表实例.设置浮点变量(名称);
                if (是指针) 符号表实例.设置指针变量(名称);
            }
            if (语句.类型 == 语句类型::常量声明) 符号表实例.声明常量(名称);
            break;
        }
        case 语句类型::赋值语句: {
            const auto& 赋值 = static_cast<const 赋值语句&>(语句);
            if (符号表实例.是常量(赋值.变量名)) {
                throw std::runtime_error("不能对常量 '" + 赋值.变量名 + "' 赋值（行 " + std::to_string(0) + "）");
            }
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
            符号表实例.进入作用域();
            for (const auto& 子 : 循环.主体) 生成语句(*子);
            符号表实例.退出作用域();
            if (构建器->GetInsertBlock()->getTerminator() == nullptr) 构建器->CreateBr(步进块);

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
        case 语句类型::做循环语句: {
            // LLVM codegen 暂不支持 do-while（CreateGlobalString 创建新块导致 body 为空）
            // 请使用 --运行 模式（虚拟机）运行包含做循环的程序
            throw std::runtime_error("做循环暂不支持编译模式，请使用 --运行 模式");
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
            throw std::runtime_error("不允许使用'返回'关键字，请直接赋值返回值变量");
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
            符号表实例.进入作用域();
            for (size_t i = 0; i < static_cast<const 代码块语句&>(语句).语句列表.size(); i++) {
                const auto& 子 = static_cast<const 代码块语句&>(语句).语句列表[i];
                生成语句(*子);
                // 死代码消除：如果当前语句是终止语句，跳过后续语句
                if (启用优化 && 是终止语句(*子) && i + 1 < static_cast<const 代码块语句&>(语句).语句列表.size()) {
                    调试打印("[优化] 消除死代码：跳过终止语句后的代码");
                    break;
                }
            }
            符号表实例.退出作用域();
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
            符号表实例.进入作用域();

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
    }
}

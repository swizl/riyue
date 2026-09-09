#include "代码生成器_内部.h"

static bool 是终止语句(const 语句& 语句) {
    return 语句.类型 == 语句类型::返回语句 ||
           语句.类型 == 语句类型::中断语句 ||
           语句.类型 == 语句类型::继续语句;
}

LLVM值* 代码生成器::创建调用(LLVM函数* 函数, llvm::ArrayRef<LLVM值*> 参数, const std::string& 名称) {
    if (当前异常处理块 && 函数) {
        基本块* 正常块 = llvm::BasicBlock::Create(上下文, "invoke_ok", 获取插入块(*构建器)->getParent());
        auto 结果 = 构建器->CreateInvoke(函数->getFunctionType(), 函数, 正常块, 当前异常处理块, 参数, 名称);
        设置插入点(*构建器, 正常块);
        return 结果;
    }
    return 构建器->CreateCall(函数, 参数, 名称);
}

LLVM值* 代码生成器::创建调用(函数被调用者 函数, llvm::ArrayRef<LLVM值*> 参数, const std::string& 名称) {
    if (当前异常处理块) {
        基本块* 正常块 = llvm::BasicBlock::Create(上下文, "invoke_ok", 获取插入块(*构建器)->getParent());
        auto 结果 = 构建器->CreateInvoke(函数, 正常块, 当前异常处理块, 参数, 名称);
        设置插入点(*构建器, 正常块);
        return 结果;
    }
    return 构建器->CreateCall(函数, 参数, 名称);
}

LLVM值* 代码生成器::创建调用无返回(LLVM函数* 函数, llvm::ArrayRef<LLVM值*> 参数) {
    if (当前异常处理块 && 函数) {
        基本块* 正常块 = llvm::BasicBlock::Create(上下文, "invoke_ok", 获取插入块(*构建器)->getParent());
        auto 结果 = 构建器->CreateInvoke(函数->getFunctionType(), 函数, 正常块, 当前异常处理块, 参数);
        设置插入点(*构建器, 正常块);
        return 结果;
    }
    return 构建器->CreateCall(函数, 参数);
}

LLVM值* 代码生成器::创建调用无返回(函数被调用者 函数, llvm::ArrayRef<LLVM值*> 参数) {
    if (当前异常处理块) {
        基本块* 正常块 = llvm::BasicBlock::Create(上下文, "invoke_ok", 获取插入块(*构建器)->getParent());
        auto 结果 = 构建器->CreateInvoke(函数, 正常块, 当前异常处理块, 参数);
        设置插入点(*构建器, 正常块);
        return 结果;
    }
    return 构建器->CreateCall(函数, 参数);
}

LLVM值* 代码生成器::创建间接调用(函数类型* 类型, LLVM值* 指针, llvm::ArrayRef<LLVM值*> 参数, const std::string& 名称) {
    if (当前异常处理块) {
        基本块* 正常块 = llvm::BasicBlock::Create(上下文, "invoke_ok", 获取插入块(*构建器)->getParent());
        auto 结果 = 构建器->CreateInvoke(类型, 指针, 正常块, 当前异常处理块, 参数, 名称);
        设置插入点(*构建器, 正常块);
        return 结果;
    }
    return 构建器->CreateCall(类型, 指针, 参数, 名称);
}

void 代码生成器::生成语句(const 语句& 语句) {
    if (获取插入块(*构建器)->getTerminatorOrNull() != nullptr) return;

    if (语句.行号 > 0) {
        LLVM函数* 记录函数 = 获取模块函数(*模块, "记录行执行");
        if (记录函数) {
            创建调用无返回(记录函数, {获取整数常量(上下文, 语句.行号, 32)});
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
                int 元素数量 = static_cast<int>(数组.元素列表.size());
                bool 含字符串 = false;
                for (const auto& 元素 : 数组.元素列表) {
                    if (元素->类型 == 表达式类型::字符串) { 含字符串 = true; break; }
                }
                if (含字符串) {
                    LLVM数组类型* 数组类型 = llvm::ArrayType::get(获取指针类型(上下文), 元素数量);
                    分配指令* 分配 = 创建分配(*构建器, 数组类型, 0, 名称);
                    for (int j = 0; j < 元素数量; j++) {
                        LLVM值* 索引 = 获取整数常量(上下文, j, 32);
                        LLVM值* 元素地址 = 创建边界内GEP(*构建器, 数组类型, 分配, {获取整数常量(上下文, 0, 32), 索引}, "元素");
                        创建存储(*构建器, 生成表达式(*数组.元素列表[j]), 元素地址);
                    }
                    符号表实例.声明数组(名称, 分配, 元素数量);
                    符号表实例.设置字符串数组(名称);
                } else {
                    LLVM数组类型* 数组类型 = llvm::ArrayType::get(整型32(上下文), 元素数量);
                    分配指令* 分配 = 创建分配(*构建器, 数组类型, 0, 名称);
                    for (int j = 0; j < 元素数量; j++) {
                        LLVM值* 索引 = 获取整数常量(上下文, j, 32);
                        LLVM值* 元素地址 = 创建边界内GEP(*构建器, 数组类型, 分配, {获取整数常量(上下文, 0, 32), 索引}, "元素");
                        创建存储(*构建器, 生成表达式(*数组.元素列表[j]), 元素地址);
                    }
                    符号表实例.声明数组(名称, 分配, 元素数量);
                }
            } else if (初始值表达式->类型 == 表达式类型::匿名函数) {
                LLVM值* 函数值 = 生成表达式(*初始值表达式);
                LLVM函数* 匿名函数 = 类型转换<LLVM函数>(函数值);
                const 闭包信息* 旧信息 = 符号表实例.获取闭包信息(匿名函数->getName().str());
                if (旧信息) 符号表实例.声明函数指针(名称, 匿名函数, 旧信息->捕获变量名, 旧信息->捕获变量地址);
                else 符号表实例.声明函数指针(名称, 匿名函数);
            } else if (初始值表达式->类型 == 表达式类型::结构体实例) {
                LLVM值* 初始值 = 生成表达式(*初始值表达式);
                符号表实例.声明变量(名称, 初始值);
                符号表实例.设置结构体变量(名称, static_cast<const 结构体实例表达式&>(*初始值表达式).结构体名);
            } else if (初始值表达式->类型 == 表达式类型::映射字面量) {
                LLVM值* 初始值 = 生成表达式(*初始值表达式);
                符号表实例.声明变量(名称, 初始值);
                符号表实例.设置映射变量(名称);
            } else if (初始值表达式->类型 == 表达式类型::元组字面量) {
                LLVM值* 初始值 = 生成表达式(*初始值表达式);
                符号表实例.声明变量(名称, 初始值);
                if (auto* alloca = LLVM动态转换<分配指令>(初始值)) {
                    元组类型映射[名称] = 类型转换<LLVM结构体类型>(alloca->getAllocatedType());
                }
            } else {
                LLVM值* 初始值 = 生成表达式(*初始值表达式);
                if (!初始值 || 是获取空类型(初始值->getType())) {
                    初始值 = 获取整数常量(上下文, 0, 32);
                }
                bool 是浮点 = 是双精度类型(初始值->getType());
                bool 是指针 = 是指针类型(初始值->getType());

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
                LLVM值* 已有地址 = 符号表实例.获取变量值(名称);
                if (已有地址) {
                    // 变量已存在，更新值
                    创建存储(*构建器, 初始值, 已有地址);
                } else {
                    // 变量不存在，创建新变量
                    类型* 变量类型;
                    if (!结构体类型名.empty()) 变量类型 = 获取指针类型(上下文);
                    else if (是浮点) 变量类型 = 双精度浮点(上下文);
                    else if (是指针) 变量类型 = 获取指针类型(上下文);
                    else 变量类型 = 整型32(上下文);
                    分配指令* 分配 = 创建分配(*构建器, 变量类型, 0, 名称);
                    创建存储(*构建器, 初始值, 分配);
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
                LLVM值* 成员地址 = 查找结构体成员地址(*构建器, 上下文, 对象名, 成员名, 符号表实例, 结构体类型映射, 结构体成员映射);
                创建存储(*构建器, 生成表达式(*赋值.值表达式), 成员地址);
            } else {
                LLVM值* 变量地址 = 符号表实例.获取变量值(赋值.变量名);
                if (!变量地址) throw std::runtime_error("未定义的变量: " + 赋值.变量名 + "（行 " + std::to_string(语句.行号) + "）");
                LLVM值* 赋值结果 = 生成表达式(*赋值.值表达式);
                if (!赋值结果 || 是获取空类型(赋值结果->getType())) {
                    赋值结果 = 获取整数常量(上下文, 0, 32);
                }
                创建存储(*构建器, 赋值结果, 变量地址);
            }
            break;
        }
        case 语句类型::复合赋值: {
            const auto& 复合赋值 = static_cast<const 复合赋值语句&>(语句);
            LLVM值* 变量地址 = 符号表实例.获取变量值(复合赋值.变量名);
            if (!变量地址) throw std::runtime_error("未定义的变量: " + 复合赋值.变量名 + "（行 " + std::to_string(语句.行号) + "）");
            LLVM值* 当前值 = 创建加载(*构建器, 整型32(上下文), 变量地址, 复合赋值.变量名);
            LLVM值* 增量值 = 生成表达式(*复合赋值.值表达式);
            LLVM值* 新值;
            switch (复合赋值.操作) {
                case 二元操作符::加法: 新值 = 创建加(*构建器, 当前值, 增量值); break;
                case 二元操作符::减法: 新值 = 创建减(*构建器, 当前值, 增量值); break;
                case 二元操作符::乘法: 新值 = 创建乘(*构建器, 当前值, 增量值); break;
                case 二元操作符::除法: 新值 = 创建有符号除法(*构建器, 当前值, 增量值); break;
                case 二元操作符::取模: 新值 = 创建有符号取余数(*构建器, 当前值, 增量值); break;
                default: throw std::runtime_error("不支持的复合赋值操作");
            }
            创建存储(*构建器, 新值, 变量地址);
            break;
        }
        case 语句类型::下标赋值语句: {
            const auto& 下标赋值 = static_cast<const 下标赋值语句&>(语句);
            if (符号表实例.是常量(下标赋值.数组名)) {
                throw std::runtime_error("不能对常量 '" + 下标赋值.数组名 + "' 赋值（行 " + std::to_string(语句.行号) + "）");
            }
            LLVM值* 数组地址 = 符号表实例.获取变量值(下标赋值.数组名);
            if (!数组地址) throw std::runtime_error("未定义的数组: " + 下标赋值.数组名 + "（行 " + std::to_string(语句.行号) + "）");
            LLVM值* 索引值 = 生成表达式(*下标赋值.索引);
            LLVM值* 赋值 = 生成表达式(*下标赋值.值表达式);
            // 优先检查是否为本地数组（alloca），使用正确的数组类型
            if (auto* alloca = LLVM动态转换<分配指令>(数组地址)) {
                类型* 分配类型 = alloca->getAllocatedType();
                if (是数组类型(分配类型)) {
                    LLVM数组类型* 数组类型 = 类型转换<LLVM数组类型>(分配类型);
                    LLVM值* 元素地址 = 创建边界内GEP(*构建器, 数组类型, 数组地址, {获取整数常量(上下文, 0, 32), 索引值}, 下标赋值.数组名 + "_元素");
                    创建存储(*构建器, 赋值, 元素地址);
                } else {
                    LLVM值* 元素地址 = 创建边界内GEP(*构建器, 整型32(上下文), 数组地址, 索引值, 下标赋值.数组名 + "_元素");
                    创建存储(*构建器, 赋值, 元素地址);
                }
            } else {
                // 动态数组：先加载指针
                LLVM值* 实际数组地址 = 创建加载(*构建器, 获取指针类型(上下文), 数组地址, 下标赋值.数组名 + "_加载");
                LLVM值* 元素地址 = 创建边界内GEP(*构建器, 整型32(上下文), 实际数组地址, 索引值, 下标赋值.数组名 + "_元素");
                创建存储(*构建器, 赋值, 元素地址);
            }
            break;
        }
        case 语句类型::如果语句: {
            const auto& 如果引用 = static_cast<const 如果语句&>(语句);
            
            // 处理初始化语句（if (初始化; 条件) 形式）
            if (如果引用.初始化) {
                生成语句(*如果引用.初始化);
            }
            
            LLVM值* 条件值 = 生成表达式(*如果引用.条件);
            if (是指针类型(条件值->getType())) {
                条件值 = 创建指针转整数(*构建器, 条件值, 整型64(上下文), "ptrtoint");
                条件值 = 创建截断(*构建器, 条件值, 整型32(上下文), "ptri32");
            }
            条件值 = 创建整数比较不等(*构建器, 条件值, 获取整数常量(上下文, 0, 32), "ifcond");
            LLVM函数* 当前函数 = 获取插入块(*构建器)->getParent();

            std::vector<基本块*> elif块列表;
            for (size_t j = 0; j < 如果引用.否则如果列表.size(); ++j)
                elif块列表.push_back(llvm::BasicBlock::Create(上下文, "elif" + std::to_string(j), 当前函数));
            基本块* else块 = llvm::BasicBlock::Create(上下文, "else", 当前函数);
            基本块* 合并块 = llvm::BasicBlock::Create(上下文, "ifcont", 当前函数);

            基本块* then块 = llvm::BasicBlock::Create(上下文, "then", 当前函数);
            基本块* 否则如果目标 = elif块列表.empty() ? else块 : elif块列表[0];
            创建条件分支(*构建器, 条件值, then块, 否则如果目标);

            设置插入点(*构建器, then块);
            {
                作用域守卫 守卫(符号表实例);
                for (const auto& 子 : 如果引用.then块) 生成语句(*子);
            }
            if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr) 创建分支(*构建器, 合并块);

            for (size_t j = 0; j < 如果引用.否则如果列表.size(); ++j) {
                设置插入点(*构建器, elif块列表[j]);
                LLVM值* elif条件 = 生成表达式(*如果引用.否则如果列表[j].条件);
                if (是指针类型(elif条件->getType())) {
                    elif条件 = 创建指针转整数(*构建器, elif条件, 整型64(上下文), "ptrtoint");
                    elif条件 = 创建截断(*构建器, elif条件, 整型32(上下文), "ptri32");
                }
                elif条件 = 创建整数比较不等(*构建器, elif条件, 获取整数常量(上下文, 0, 32), "elifcond");
                基本块* elif体块 = llvm::BasicBlock::Create(上下文, "elifbody", 当前函数);
                基本块* 下一个 = (j + 1 < elif块列表.size()) ? elif块列表[j + 1] : else块;
                创建条件分支(*构建器, elif条件, elif体块, 下一个);
                设置插入点(*构建器, elif体块);
                {
                    作用域守卫 守卫(符号表实例);
                    for (const auto& 子 : 如果引用.否则如果列表[j].主体) 生成语句(*子);
                }
                if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr) 创建分支(*构建器, 合并块);
            }

            设置插入点(*构建器, else块);
            {
                作用域守卫 守卫(符号表实例);
                for (const auto& 子 : 如果引用.else块) 生成语句(*子);
            }
            if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr) 创建分支(*构建器, 合并块);
            设置插入点(*构建器, 合并块);
            break;
        }
        case 语句类型::循环语句: {
            const auto& 循环引用 = static_cast<const 循环语句&>(语句);
            LLVM函数* 当前函数 = 获取插入块(*构建器)->getParent();

            std::unique_ptr<作用域守卫> 初始化守卫;
            if (循环引用.初始化) {
                初始化守卫 = std::make_unique<作用域守卫>(符号表实例);
                生成语句(*循环引用.初始化);
            }

            基本块* 条件块 = llvm::BasicBlock::Create(上下文, "forcond", 当前函数);
            基本块* 体块 = llvm::BasicBlock::Create(上下文, "forbody", 当前函数);
            基本块* 步进块 = llvm::BasicBlock::Create(上下文, "forstep", 当前函数);
            基本块* 后块 = llvm::BasicBlock::Create(上下文, "forend", 当前函数);

            循环继续栈.push(步进块);
            循环退出栈.push(后块);

            创建分支(*构建器, 条件块);

            设置插入点(*构建器, 条件块);
            if (循环引用.条件) {
                LLVM值* 条件值 = 生成表达式(*循环引用.条件);
                if (是指针类型(条件值->getType())) {
                    条件值 = 创建指针转整数(*构建器, 条件值, 整型64(上下文), "ptrtoint");
                    条件值 = 创建截断(*构建器, 条件值, 整型32(上下文), "ptri32");
                }
                条件值 = 创建整数比较不等(*构建器, 条件值, 获取整数常量(上下文, 0, 32), "forcond");
                创建条件分支(*构建器, 条件值, 体块, 后块);
            } else {
                创建分支(*构建器, 体块);
            }

            设置插入点(*构建器, 体块);
            {
                作用域守卫 守卫(符号表实例);
                for (const auto& 子 : 循环引用.主体) 生成语句(*子);
            }
            if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr) 创建分支(*构建器, 步进块);

            设置插入点(*构建器, 步进块);
            if (循环引用.步进) {
                生成语句(*循环引用.步进);
            }
            创建分支(*构建器, 条件块);

            循环继续栈.pop();
            循环退出栈.pop();
            设置插入点(*构建器, 后块);

            初始化守卫.reset();
            break;
        }
        case 语句类型::当循环语句: {
            const auto& 当循环 = static_cast<const 当循环语句&>(语句);
            LLVM函数* 当前函数 = 获取插入块(*构建器)->getParent();
            基本块* 条件块 = llvm::BasicBlock::Create(上下文, "whilecond", 当前函数);
            基本块* 体块 = llvm::BasicBlock::Create(上下文, "whilebody", 当前函数);
            基本块* 后块 = llvm::BasicBlock::Create(上下文, "whileend", 当前函数);
            循环继续栈.push(条件块);
            循环退出栈.push(后块);
            创建分支(*构建器, 条件块);
            设置插入点(*构建器, 条件块);
            LLVM值* 条件值 = 生成表达式(*当循环.条件);
            if (是指针类型(条件值->getType())) {
                条件值 = 创建指针转整数(*构建器, 条件值, 整型64(上下文), "ptrtoint");
                条件值 = 创建截断(*构建器, 条件值, 整型32(上下文), "ptri32");
            }
            条件值 = 创建整数比较不等(*构建器, 条件值, 获取整数常量(上下文, 0, 32), "whilecond");
            创建条件分支(*构建器, 条件值, 体块, 后块);
            设置插入点(*构建器, 体块);
            {
                作用域守卫 守卫(符号表实例);
                for (const auto& 子 : 当循环.主体) 生成语句(*子);
            }
            if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr) 创建分支(*构建器, 条件块);
            循环继续栈.pop();
            循环退出栈.pop();
            设置插入点(*构建器, 后块);
            break;
        }
        case 语句类型::做循环语句: {
            const auto& 做循环 = static_cast<const 做循环语句&>(语句);
            LLVM函数* 当前函数 = 获取插入块(*构建器)->getParent();
            基本块* 体块 = llvm::BasicBlock::Create(上下文, "do_body", 当前函数);
            基本块* 条件块 = llvm::BasicBlock::Create(上下文, "do_cond", 当前函数);
            基本块* 后块 = llvm::BasicBlock::Create(上下文, "do_end");

            创建分支(*构建器, 体块);
            设置插入点(*构建器, 体块);
            {
                作用域守卫 守卫(符号表实例);
                循环继续栈.push(条件块);
                循环退出栈.push(后块);
                for (const auto& 子语句 : 做循环.主体) 生成语句(*子语句);
            }
            if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr) 创建分支(*构建器, 条件块);

            设置插入点(*构建器, 条件块);
            LLVM值* 条件值 = 生成表达式(*做循环.条件);
            if (是指针类型(条件值->getType())) {
                条件值 = 创建指针转整数(*构建器, 条件值, 整型64(上下文), "ptrtoint");
                条件值 = 创建截断(*构建器, 条件值, 整型32(上下文), "ptri32");
            }
            LLVM值* 条件结果 = 创建整数比较不等(*构建器, 条件值, 获取整数常量(上下文, 0, 32), "do_cond");
            if (做循环.是当循环) {
                创建条件分支(*构建器, 条件结果, 体块, 后块);
            } else {
                创建条件分支(*构建器, 条件结果, 后块, 体块);
            }

            循环继续栈.pop();
            循环退出栈.pop();
            当前函数->insert(当前函数->end(), 后块);
            设置插入点(*构建器, 后块);
            break;
        }
        case 语句类型::中断语句: {
            if (循环退出栈.empty()) throw std::runtime_error("'中断'必须在循环内使用（行 " + std::to_string(语句.行号) + "）");
            创建分支(*构建器, 循环退出栈.top());
            基本块* 新块 = llvm::BasicBlock::Create(上下文, "afterbreak", 获取插入块(*构建器)->getParent());
            设置插入点(*构建器, 新块);
            break;
        }
        case 语句类型::继续语句: {
            if (循环继续栈.empty()) throw std::runtime_error("'继续'必须在循环内使用（行 " + std::to_string(语句.行号) + "）");
            创建分支(*构建器, 循环继续栈.top());
            基本块* 新块 = llvm::BasicBlock::Create(上下文, "aftercontinue", 获取插入块(*构建器)->getParent());
            设置插入点(*构建器, 新块);
            break;
        }
        case 语句类型::返回语句: {
            throw std::runtime_error("不允许使用'返回'关键字，请直接赋值返回值变量（行 " + std::to_string(语句.行号) + "）");
        }
        case 语句类型::打印语句: {
            const auto& 打印 = static_cast<const 打印语句&>(语句);
            LLVM值* 打印值 = 生成表达式(*打印.值表达式);
            函数类型* printf类型 = llvm::FunctionType::get(整型32(上下文), {获取指针类型(上下文)}, true);
            获取或插入函数(*模块, "printf", printf类型);
            if (打印.值表达式->类型 == 表达式类型::字符串) {
                创建调用无返回(获取模块函数(*模块, "printf"), {创建全局字符串(*构建器, "%s\n"), 打印值});
            } else if (是双精度类型(打印值->getType()) || 是浮点类型(打印值->getType())) {
                创建调用无返回(获取模块函数(*模块, "printf"), {创建全局字符串(*构建器, "%.6g\n"), 打印值});
            } else if (是指针类型(打印值->getType())) {
                创建调用无返回(获取模块函数(*模块, "printf"), {创建全局字符串(*构建器, "%s\n"), 打印值});
            } else if (打印.值表达式->类型 == 表达式类型::下标访问 && 符号表实例.是映射变量(static_cast<const 下标访问表达式&>(*打印.值表达式).数组名)) {
                创建调用无返回(获取模块函数(*模块, "printf"), {创建全局字符串(*构建器, "%s\n"), 打印值});
            } else {
                创建调用无返回(获取模块函数(*模块, "printf"), {创建全局字符串(*构建器, "%d\n"), 打印值});
            }
            break;
        }
        case 语句类型::表达式语句:
            生成表达式(*static_cast<const 表达式语句&>(语句).值表达式);
            break;
        case 语句类型::解构赋值: {
            const auto& 解构 = static_cast<const 解构赋值语句&>(语句);
            LLVM值* 值 = 生成表达式(*解构.值表达式);
            
            // 检查是否为结构体类型（多返回值）
            if (是结构体类型(值->getType())) {
                auto* 结构体类型 = 类型转换<LLVM结构体类型>(值->getType());
                for (size_t i = 0; i < 解构.变量名列表.size() && i < 结构体类型->getNumElements(); i++) {
                    LLVM值* 元素值 = 创建提取值(*构建器, 值, {static_cast<unsigned>(i)});
                    std::string 变量名 = 解构.变量名列表[i];
                    
                    // 检查变量是否已存在
                    LLVM值* 已有地址 = 符号表实例.获取变量值(变量名);
                    if (已有地址) {
                        创建存储(*构建器, 元素值, 已有地址);
                    } else {
                        类型* 元素类型 = 结构体类型->getElementType(i);
                        分配指令* 分配 = 创建分配(*构建器, 元素类型, 0, 变量名);
                        创建存储(*构建器, 元素值, 分配);
                        符号表实例.声明变量(变量名, 分配);
                        if (是双精度类型(元素类型)) 符号表实例.设置浮点变量(变量名);
                        else if (是指针类型(元素类型)) 符号表实例.设置指针变量(变量名);
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
            LLVM值* 匹配值 = 生成表达式(*匹配.匹配值);
            LLVM函数* 当前函数 = 获取插入块(*构建器)->getParent();
            基本块* 合并块 = llvm::BasicBlock::Create(上下文, "matchend", 当前函数);

            std::vector<基本块*> 分支块;
            for (size_t i = 0; i < 匹配.分支列表.size(); i++) {
                分支块.push_back(llvm::BasicBlock::Create(上下文, "match_" + std::to_string(i), 当前函数));
            }

            创建分支(*构建器, 分支块[0]);

            for (size_t i = 0; i < 匹配.分支列表.size(); i++) {
                设置插入点(*构建器, 分支块[i]);
                const auto& 分支 = 匹配.分支列表[i];

                if (分支.条件) {
                    LLVM值* 分支值 = 生成表达式(*分支.条件);
                    LLVM值* 比较结果;
                    if (是指针类型(匹配值->getType()) && 是指针类型(分支值->getType())) {
                        // 字符串比较：使用 strcmp
                        LLVM函数* strcmp函数 = 获取模块函数(*模块, "strcmp");
                        LLVM值* cmp结果 = 创建调用(strcmp函数, {匹配值, 分支值}, "strcmp结果");
                        比较结果 = 创建整数比较等(*构建器, cmp结果, 获取整数常量(上下文, 0, 32), "streq");
                    } else {
                        比较结果 = 创建整数比较等(*构建器, 匹配值, 分支值, "matchcmp");
                    }

                    基本块* 下一个块 = (i + 1 < 分支块.size()) ? 分支块[i + 1] : 合并块;
                    基本块* 执行块 = llvm::BasicBlock::Create(上下文, "match_body_" + std::to_string(i), 当前函数);
                    创建条件分支(*构建器, 比较结果, 执行块, 下一个块);

                    设置插入点(*构建器, 执行块);
                    {
                        作用域守卫 守卫(符号表实例);
                        for (const auto& 子 : 分支.主体) 生成语句(*子);
                    }
                    if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr) 创建分支(*构建器, 合并块);
                } else {
                    作用域守卫 守卫(符号表实例);
                    for (const auto& 子 : 分支.主体) 生成语句(*子);
                    if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr) 创建分支(*构建器, 合并块);
                }
            }

            设置插入点(*构建器, 合并块);
            break;
        }
        case 语句类型::遍历语句: {
            const auto& 遍历 = static_cast<const 遍历语句&>(语句);
            LLVM函数* 当前函数 = 获取插入块(*构建器)->getParent();

            LLVM值* 数组地址 = 生成表达式(*遍历.数组表达式);
            int 数组大小值 = 100;
            if (遍历.数组表达式->类型 == 表达式类型::变量) {
                const auto& 变量名 = static_cast<const 变量表达式&>(*遍历.数组表达式).名称;
                int 元素数量 = 符号表实例.获取数组大小(变量名);
                if (元素数量 > 0) 数组大小值 = 元素数量;
            }
            LLVM值* 数组大小 = 获取整数常量(上下文, 数组大小值, 32);

            分配指令* 索引变量 = 创建分配(*构建器, 整型32(上下文), 0, "遍历索引");
            创建存储(*构建器, 获取整数常量(上下文, 0, 32), 索引变量);

            基本块* 条件块 = llvm::BasicBlock::Create(上下文, "forin_cond", 当前函数);
            基本块* 体块 = llvm::BasicBlock::Create(上下文, "forin_body", 当前函数);
            基本块* 步进块 = llvm::BasicBlock::Create(上下文, "forin_step", 当前函数);
            基本块* 后块 = llvm::BasicBlock::Create(上下文, "forin_end", 当前函数);

            循环继续栈.push(步进块);
            循环退出栈.push(后块);

            创建分支(*构建器, 条件块);

            设置插入点(*构建器, 条件块);
            LLVM值* 当前索引 = 创建加载(*构建器, 整型32(上下文), 索引变量, "当前索引");
            LLVM值* 条件 = 创建整数比较小于(*构建器, 当前索引, 数组大小, "遍历条件");
            创建条件分支(*构建器, 条件, 体块, 后块);

            设置插入点(*构建器, 体块);
            {
                作用域守卫 守卫(符号表实例);

                // 优先检查本地数组类型
                if (auto* alloca = LLVM动态转换<分配指令>(数组地址)) {
                    类型* 分配类型 = alloca->getAllocatedType();
                    if (是数组类型(分配类型)) {
LLVM数组类型* 数组类型 = 类型转换<LLVM数组类型>(分配类型);
                        LLVM值* 元素地址 = 创建边界内GEP(*构建器, 数组类型, 数组地址, {获取整数常量(上下文, 0, 32), 当前索引}, "元素地址");
                        LLVM值* 元素值 = 创建加载(*构建器, 数组类型->getElementType(), 元素地址, "元素值");
                        分配指令* 循环变量 = 创建分配(*构建器, 数组类型->getElementType(), 0, 遍历.变量名);
                        创建存储(*构建器, 元素值, 循环变量);
                        符号表实例.声明变量(遍历.变量名, 循环变量);
                        if (是双精度类型(数组类型->getElementType())) 符号表实例.设置浮点变量(遍历.变量名);
                        else if (是指针类型(数组类型->getElementType())) 符号表实例.设置指针变量(遍历.变量名);
                    } else {
                        LLVM值* 元素地址 = 创建边界内GEP(*构建器, 整型32(上下文), 数组地址, 当前索引, "元素地址");
                        LLVM值* 元素值 = 创建加载(*构建器, 整型32(上下文), 元素地址, "元素值");
                        分配指令* 循环变量 = 创建分配(*构建器, 整型32(上下文), 0, 遍历.变量名);
                        创建存储(*构建器, 元素值, 循环变量);
                        符号表实例.声明变量(遍历.变量名, 循环变量);
                    }
                } else {
                    LLVM值* 元素地址 = 创建边界内GEP(*构建器, 整型32(上下文), 数组地址, 当前索引, "元素地址");
                    LLVM值* 元素值 = 创建加载(*构建器, 整型32(上下文), 元素地址, "元素值");
                    分配指令* 循环变量 = 创建分配(*构建器, 整型32(上下文), 0, 遍历.变量名);
                    创建存储(*构建器, 元素值, 循环变量);
                    符号表实例.声明变量(遍历.变量名, 循环变量);
                }

                for (const auto& 子 : 遍历.主体) 生成语句(*子);
            }
            if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr) 创建分支(*构建器, 步进块);

            设置插入点(*构建器, 步进块);
            LLVM值* 新索引 = 创建加(*构建器, 当前索引, 获取整数常量(上下文, 1, 32), "新索引");
            创建存储(*构建器, 新索引, 索引变量);
            创建分支(*构建器, 条件块);

            循环继续栈.pop();
            循环退出栈.pop();
            设置插入点(*构建器, 后块);
            break;
        }
        case 语句类型::结构体定义: {
            const auto& 结构体引用 = static_cast<const 结构体定义语句&>(语句);
            std::vector<类型*> 成员类型;
            std::vector<std::string> 成员名;
            std::vector<std::string> 成员类型名;
            for (const auto& 成员 : 结构体引用.成员列表) {
                成员类型.push_back(类型名到LLVM类型(成员.类型));
                成员名.push_back(成员.名称);
                成员类型名.push_back(成员.类型);
            }
            LLVM结构体类型* 结构体类型 = llvm::StructType::create(上下文, 成员类型, 结构体引用.名称);
            结构体类型映射[结构体引用.名称] = 结构体类型;
            结构体成员映射[结构体引用.名称] = 成员名;
            结构体成员类型名映射[结构体引用.名称] = 成员类型名;
            调试打印("[代码生成器] 定义结构体: " << 结构体引用.名称 << " 成员数: " << 成员名.size());
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
            LLVM值* 让出值 = 生成表达式(*让出.值表达式);
            if (!返回值变量列表.empty()) {
                创建存储(*构建器, 让出值, 返回值变量列表[0]);
            }

            // LLVM coro.suspend intrinsic
            auto* coro_suspend = llvm::Intrinsic::getOrInsertDeclaration(模块.get(), llvm::Intrinsic::coro_suspend);
            auto* token = llvm::ConstantTokenNone::get(上下文);
            auto* suspend结果 = llvm中文::创建调用(*构建器, coro_suspend, {token, llvm::ConstantInt::getFalse(上下文)}, "yield_suspend");

            LLVM函数* 当前函数体 = 获取插入块(*构建器)->getParent();
            auto* resumeBB = llvm::BasicBlock::Create(上下文, "coro.resume", 当前函数体);
            auto* suspendBB = llvm::BasicBlock::Create(上下文, "coro.suspend", 当前函数体);
            auto* destroyBB = llvm::BasicBlock::Create(上下文, "coro.destroy", 当前函数体);

            auto* sw = 创建开关指令(*构建器, suspend结果, suspendBB, 2);
            sw->addCase(获取整数常量(上下文, 0, 8), resumeBB);
            sw->addCase(获取整数常量(上下文, 1, 8), destroyBB);

            // resume: continue execution
            设置插入点(*构建器, resumeBB);

            // destroy: cleanup and return
            设置插入点(*构建器, destroyBB);
            auto* coro_free = llvm::Intrinsic::getOrInsertDeclaration(模块.get(), llvm::Intrinsic::coro_free);
            auto* coro_end = llvm::Intrinsic::getOrInsertDeclaration(模块.get(), llvm::Intrinsic::coro_end);
            if (coro_free && 协程句柄) {
                auto* free_mem = llvm中文::创建调用(*构建器, coro_free, {token, 协程句柄}, "coro_free");
                auto* free函数 = 获取模块函数(*模块, "free");
                if (free函数) llvm中文::创建调用(*构建器, free函数, {free_mem});
            }
            创建分支(*构建器, suspendBB);

            // suspend: yield value and return
            设置插入点(*构建器, suspendBB);
            if (coro_end && 协程句柄) {
                llvm中文::创建调用(*构建器, coro_end, {协程句柄, llvm::ConstantInt::getFalse(上下文), token});
            }
            if (返回值变量列表.size() == 1) {
                auto* ret = 创建加载(*构建器, 返回值变量列表[0]->getAllocatedType(), 返回值变量列表[0], "yield_val");
                创建返回指令(*构建器, ret);
            } else {
                创建空返回(*构建器);
            }

            // resume block continues here
            设置插入点(*构建器, resumeBB);
            break;
        }

        case 语句类型::抛出语句: {
            const auto& 抛出引用 = static_cast<const 抛出语句&>(语句);
            LLVM值* 异常值 = 生成表达式(*抛出引用.异常值);
            LLVM值* 异常对象 = 创建调用(获取模块函数(*模块, "__cxa_allocate_exception"),
                                                        {获取整数常量(上下文, 8, 64)}, "exc_alloc");
            LLVM值* 存储值 = 异常值;
            if (是整型(异常值->getType())) {
                LLVM值* i64值 = 创建符号扩展(*构建器, 异常值, 整型64(上下文));
                存储值 = 创建整数转指针(*构建器, i64值, 获取指针类型(上下文));
                存储值 = 创建指针转整数(*构建器, 存储值, 整型64(上下文));
            } else if (是指针类型(异常值->getType())) {
                存储值 = 创建指针转整数(*构建器, 异常值, 整型64(上下文));
            }
            创建存储(*构建器, 存储值, 创建位转换(*构建器, 异常对象, 获取指针类型(上下文)));
            LLVM函数* throw函数 = 获取模块函数(*模块, "__cxa_throw");
            基本块* 正常块 = llvm::BasicBlock::Create(上下文, "throw_ok", 获取插入块(*构建器)->getParent());
            基本块* 异常块 = 当前异常处理块 ? 当前异常处理块 : llvm::BasicBlock::Create(上下文, "throw_unwind", 获取插入块(*构建器)->getParent());
            std::vector<LLVM值*> throw参数 = {异常对象, 获取空值(获取指针类型(上下文)),
                                                     获取空值(获取指针类型(上下文))};
            构建器->CreateInvoke(throw函数->getFunctionType(), throw函数, 正常块, 异常块, throw参数);
            设置插入点(*构建器, 正常块);
            创建不可达(*构建器);
            break;
        }
        case 语句类型::尝试语句: {
            const auto& 尝试引用 = static_cast<const 尝试语句&>(语句);
            LLVM函数* 当前函数 = 获取插入块(*构建器)->getParent();

LLVM函数* 人格函数 = 获取模块函数(*模块, "__gxx_personality_seh0");
LLVM函数* 开始捕获函数 = 获取模块函数(*模块, "__cxa_begin_catch");
LLVM函数* 结束捕获函数 = 获取模块函数(*模块, "__cxa_end_catch");
LLVM函数* 恢复函数 = 获取模块函数(*模块, "_Unwind_Resume");

            基本块* 合并块 = llvm::BasicBlock::Create(上下文, "try_merge", 当前函数);
            基本块* 清理块 = llvm::BasicBlock::Create(上下文, "try_cleanup", 当前函数);
            基本块* 捕获块 = nullptr;

            基本块* 最终正常块 = nullptr;
            基本块* 最终异常块 = nullptr;
            if (!尝试引用.最终主体.empty()) {
                最终正常块 = llvm::BasicBlock::Create(上下文, "finally_ok", 当前函数);
                最终异常块 = llvm::BasicBlock::Create(上下文, "finally_ex", 当前函数);
            }

            if (!尝试引用.捕获主体.empty()) {
                捕获块 = llvm::BasicBlock::Create(上下文, "catch", 当前函数);
            }

            基本块* 旧异常处理块 = 当前异常处理块;
            当前异常处理块 = 清理块;

            for (const auto& 子 : 尝试引用.尝试主体) 生成语句(*子);

            当前异常处理块 = 旧异常处理块;

            if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr) {
                if (最终正常块) {
                    创建分支(*构建器, 最终正常块);
                } else {
                    创建分支(*构建器, 合并块);
                }
            }

            设置插入点(*构建器, 清理块);
            着陆垫指令* 着陆垫 = 创建着陆垫(*构建器,
                llvm::StructType::get(获取指针类型(上下文), 整型32(上下文)), 1, "landingpad");
            着陆垫->addClause(获取空值(获取指针类型(上下文)));
            LLVM值* 异常指针 = 创建提取值(*构建器, 着陆垫, 0, "exc_ptr");
            LLVM值* 选择器 = 创建提取值(*构建器, 着陆垫, 1, "sel");

            if (捕获块) {
                创建分支(*构建器, 捕获块);

                设置插入点(*构建器, 捕获块);
                LLVM值* 捕获值 = llvm中文::创建调用(*构建器, 开始捕获函数, {异常指针}, "catch_val");
                LLVM值* 异常i64 = 创建加载(*构建器, 整型64(上下文),
                    创建位转换(*构建器, 捕获值, 获取指针类型(上下文)), "exc_i64");
                LLVM值* exc_ptr = 创建整数转指针(*构建器, 异常i64, 获取指针类型(上下文), "exc_ptr");
                {
                    作用域守卫 守卫(符号表实例);
                    分配指令* 变量地址 = 创建分配(*构建器, 获取指针类型(上下文), 0);
                    符号表实例.声明变量(尝试引用.异常变量名, 变量地址);
                    符号表实例.设置指针变量(尝试引用.异常变量名);
                    创建存储(*构建器, exc_ptr, 变量地址);
                    for (const auto& 子 : 尝试引用.捕获主体) 生成语句(*子);
                }
                if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr) {
                    llvm中文::创建调用(*构建器, 结束捕获函数, {});
                    if (最终正常块) {
                        创建分支(*构建器, 最终正常块);
                    } else {
                        创建分支(*构建器, 合并块);
                    }
                }
            } else {
                if (!尝试引用.最终主体.empty()) {
                    创建分支(*构建器, 最终异常块);
                } else {
                    llvm中文::创建调用(*构建器, 恢复函数, {异常指针});
                    创建不可达(*构建器);
                }
            }

            if (最终正常块) {
                设置插入点(*构建器, 最终正常块);
                for (const auto& 子 : 尝试引用.最终主体) 生成语句(*子);
                if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr)
                    创建分支(*构建器, 合并块);
            }

            if (最终异常块) {
                设置插入点(*构建器, 最终异常块);
                for (const auto& 子 : 尝试引用.最终主体) 生成语句(*子);
                if (获取插入块(*构建器)->getTerminatorOrNull() == nullptr) {
                    llvm中文::创建调用(*构建器, 恢复函数, {异常指针});
                    创建不可达(*构建器);
                }
            }

            设置插入点(*构建器, 合并块);
            break;
        }
    }
}
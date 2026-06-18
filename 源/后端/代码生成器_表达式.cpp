#include "代码生成器_内部.h"

llvm::Value* 代码生成器::生成表达式(const 表达式& 表达式) {
    switch (表达式.类型) {
        case 表达式类型::整数:
            return llvm::ConstantInt::get(上下文, llvm::APInt(32, static_cast<const 整数表达式&>(表达式).值));
        case 表达式类型::字符:
            return llvm::ConstantInt::get(上下文, llvm::APInt(32, static_cast<const 字符表达式&>(表达式).值));
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
            // 函数指针直接返回，不加载
            if (llvm::isa<llvm::Function>(地址)) return 地址;
            // 从 alloca 获取实际类型，而不是依赖符号表的类型标记
            llvm::Type* 类型;
            if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(地址)) {
                类型 = alloca->getAllocatedType();
            } else if (auto* arg = llvm::dyn_cast<llvm::Argument>(地址)) {
                // 捕获变量参数是指针，需要解引用获取实际类型
                if (arg->getType()->isPointerTy()) {
                    // 按符号表标记推断实际类型
                    if (符号表实例.是浮点变量(变量.名称)) 类型 = llvm::Type::getDoubleTy(上下文);
                    else if (符号表实例.是指针变量(变量.名称)) 类型 = llvm::PointerType::get(上下文, 0);
                    else 类型 = llvm::Type::getInt32Ty(上下文);
                } else {
                    类型 = arg->getType();
                }
            } else if (地址->getType()->isPointerTy()) {
                // 默认：按符号表标记推断
                if (符号表实例.是浮点变量(变量.名称)) 类型 = llvm::Type::getDoubleTy(上下文);
                else if (符号表实例.是指针变量(变量.名称)) 类型 = llvm::PointerType::get(上下文, 0);
                else 类型 = llvm::Type::getInt32Ty(上下文);
            } else {
                类型 = 地址->getType();
            }
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

            // 浮点运算
            bool 是浮点 = (左->getType()->isDoubleTy() || 右->getType()->isDoubleTy());
            if (是浮点) {
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
                case 二元操作符::左移: return 构建器->CreateShl(左, 右, "shltmp");
                case 二元操作符::右移: return 构建器->CreateAShr(左, 右, "shrtmp");
                case 二元操作符::位与: return 构建器->CreateAnd(左, 右, "andtmp");
                case 二元操作符::位或: return 构建器->CreateOr(左, 右, "ortmp");
                case 二元操作符::位异或: return 构建器->CreateXor(左, 右, "xortmp");
                default: throw std::runtime_error("未知二元运算符");
            }
        }
        case 表达式类型::函数调用: {
            const auto& 调用 = static_cast<const 函数调用表达式&>(表达式);

            // 文件IO内置函数
            if (调用.函数名 == "打开") {
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
                llvm::Value* 当前计数 = 构建器->CreateLoad(llvm::Type::getInt32Ty(上下文), 文件句柄计数器, "当前计数");
                llvm::Value* 槽地址 = 构建器->CreateInBoundsGEP(文件表类型, 文件表指针, {零, 当前计数}, "槽地址");
                构建器->CreateStore(文件指针, 槽地址);
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
            // 反射函数：类型名
            if (调用.函数名 == "类型名") {
                生成表达式(*调用.参数列表[0]);
                std::string 类型名 = "整数";
                if (调用.参数列表[0]->类型 == 表达式类型::浮点数) 类型名 = "浮点数";
                else if (调用.参数列表[0]->类型 == 表达式类型::字符串) 类型名 = "字符串";
                else if (调用.参数列表[0]->类型 == 表达式类型::布尔值) 类型名 = "布尔";
                else if (调用.参数列表[0]->类型 == 表达式类型::字符) 类型名 = "字符";
                else if (调用.参数列表[0]->类型 == 表达式类型::变量) {
                    const auto& 变量名 = static_cast<const 变量表达式&>(*调用.参数列表[0]).名称;
                    if (符号表实例.是浮点变量(变量名)) 类型名 = "浮点数";
                    else if (符号表实例.是指针变量(变量名)) 类型名 = "字符串";
                    else if (符号表实例.是数组(变量名)) 类型名 = "数组";
                }
                return 构建器->CreateGlobalString(类型名);
            }
            // 反射函数：类型检查
            if (调用.函数名 == "是整数" || 调用.函数名 == "是浮点" || 调用.函数名 == "是字符串" ||
                调用.函数名 == "是布尔" || 调用.函数名 == "是字符" || 调用.函数名 == "是数组") {
                bool 结果 = false;
                if (调用.参数列表[0]->类型 == 表达式类型::变量) {
                    const auto& 变量名 = static_cast<const 变量表达式&>(*调用.参数列表[0]).名称;
                    if (调用.函数名 == "是整数") 结果 = !符号表实例.是浮点变量(变量名) && !符号表实例.是指针变量(变量名) && !符号表实例.是数组(变量名);
                    else if (调用.函数名 == "是浮点") 结果 = 符号表实例.是浮点变量(变量名);
                    else if (调用.函数名 == "是字符串") 结果 = 符号表实例.是指针变量(变量名) && !符号表实例.是数组(变量名);
                    else if (调用.函数名 == "是布尔") 结果 = false; // 布尔存储为整数
                    else if (调用.函数名 == "是字符") 结果 = false; // 字符存储为整数
                    else if (调用.函数名 == "是数组") 结果 = 符号表实例.是数组(变量名);
                } else {
                    if (调用.函数名 == "是整数") 结果 = 调用.参数列表[0]->类型 == 表达式类型::整数;
                    else if (调用.函数名 == "是浮点") 结果 = 调用.参数列表[0]->类型 == 表达式类型::浮点数;
                    else if (调用.函数名 == "是字符串") 结果 = 调用.参数列表[0]->类型 == 表达式类型::字符串;
                    else if (调用.函数名 == "是布尔") 结果 = 调用.参数列表[0]->类型 == 表达式类型::布尔值;
                    else if (调用.函数名 == "是字符") 结果 = 调用.参数列表[0]->类型 == 表达式类型::字符;
                }
                return llvm::ConstantInt::get(上下文, llvm::APInt(32, 结果 ? 1 : 0));
            }
            if (调用.函数名 == "查找") {
                llvm::FunctionType* strstr类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("strstr", strstr类型);
                llvm::Value* haystack = 生成表达式(*调用.参数列表[0]);
                llvm::Value* needle = 生成表达式(*调用.参数列表[1]);
                llvm::Value* result = 构建器->CreateCall(模块->getFunction("strstr"), {haystack, needle}, "查找结果");
                llvm::Value* nullPtr = llvm::ConstantPointerNull::get(llvm::PointerType::get(上下文, 0));
                llvm::Value* isNull = 构建器->CreateICmpEQ(result, nullPtr, "isNull");
                llvm::Value* resultInt = 构建器->CreatePtrToInt(result, llvm::Type::getInt64Ty(上下文), "resultInt");
                llvm::Value* haystackInt = 构建器->CreatePtrToInt(haystack, llvm::Type::getInt64Ty(上下文), "haystackInt");
                llvm::Value* offset = 构建器->CreateSub(resultInt, haystackInt, "偏移");
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

                llvm::Value* byteStart = 构建器->CreateCall(模块->getFunction("字符位置到字节位置"), {src, charStart}, "字节起始");
                llvm::Value* byteEnd = 构建器->CreateCall(模块->getFunction("字符位置到字节位置"),
                    {src, 构建器->CreateAdd(charStart, charLen, "endChar")}, "字节结束");
                llvm::Value* byteLen = 构建器->CreateSub(byteEnd, byteStart, "字节长度");
                llvm::Value* lenNonNeg = 构建器->CreateICmpSGE(byteLen, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)));
                byteLen = 构建器->CreateSelect(lenNonNeg, byteLen, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), "安全字节长度");

                llvm::Value* srcPtr = 构建器->CreateGEP(llvm::Type::getInt8Ty(上下文), src, byteStart, "srcPtr");
                llvm::Value* 缓冲区 = 构建器->CreateCall(模块->getFunction("malloc"),
                    {构建器->CreateZExt(构建器->CreateAdd(byteLen, llvm::ConstantInt::get(上下文, llvm::APInt(32, 1)), "bufsz"),
                        llvm::Type::getInt64Ty(上下文))}, "子串缓冲区");
                构建器->CreateCall(模块->getFunction("strncpy"), {缓冲区, srcPtr, 构建器->CreateZExt(byteLen, llvm::Type::getInt64Ty(上下文))});
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

            // 输入函数
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

            // 数组高阶函数
            if (调用.函数名 == "排序") {
                llvm::Value* 数组地址 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 大小 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 排序类型 = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("数组排序", 排序类型);
                构建器->CreateCall(模块->getFunction("数组排序"), {数组地址, 大小});
                return llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
            }
            if (调用.函数名 == "反转") {
                llvm::Value* 数组地址 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 大小 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 反转类型 = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("数组反转", 反转类型);
                构建器->CreateCall(模块->getFunction("数组反转"), {数组地址, 大小});
                return llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
            }
            if (调用.函数名 == "包含") {
                llvm::Value* 数组地址 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 大小 = 生成表达式(*调用.参数列表[1]);
                llvm::Value* 元素 = 生成表达式(*调用.参数列表[2]);
                llvm::FunctionType* 包含类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("数组包含", 包含类型);
                return 构建器->CreateCall(模块->getFunction("数组包含"), {数组地址, 大小, 元素});
            }

            // 字符串高级操作
            if (调用.函数名 == "分割") {
                llvm::Value* 字符串 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 分隔符 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 分割类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(llvm::PointerType::get(上下文, 0), 0),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("字符串分割", 分割类型);
                // 需要一个输出参数来接收数量
                llvm::AllocaInst* 数量地址 = 构建器->CreateAlloca(llvm::Type::getInt32Ty(上下文), nullptr, "分割数量");
                llvm::Value* 结果 = 构建器->CreateCall(模块->getFunction("字符串分割"), {字符串, 分隔符, 数量地址});
                return 结果;
            }
            if (调用.函数名 == "连接") {
                llvm::Value* 数组地址 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 大小 = 生成表达式(*调用.参数列表[1]);
                llvm::Value* 分隔符 = 生成表达式(*调用.参数列表[2]);
                llvm::FunctionType* 连接类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(llvm::PointerType::get(上下文, 0), 0), llvm::Type::getInt32Ty(上下文), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("字符串连接", 连接类型);
                return 构建器->CreateCall(模块->getFunction("字符串连接"), {数组地址, 大小, 分隔符});
            }
            if (调用.函数名 == "格式化") {
                llvm::Value* 模板 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 参数1 = 调用.参数列表.size() > 1 ? 生成表达式(*调用.参数列表[1]) : llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                llvm::Value* 参数2 = 调用.参数列表.size() > 2 ? 生成表达式(*调用.参数列表[2]) : llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                llvm::Value* 参数3 = 调用.参数列表.size() > 3 ? 生成表达式(*调用.参数列表[3]) : llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                llvm::FunctionType* 格式化类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文), llvm::Type::getInt32Ty(上下文), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("字符串格式化", 格式化类型);
                return 构建器->CreateCall(模块->getFunction("字符串格式化"), {模板, 参数1, 参数2, 参数3});
            }

            // 日期时间函数
            if (调用.函数名 == "获取时间戳") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt64Ty(上下文), {}, false);
                模块->getOrInsertFunction("获取时间戳", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取时间戳"), {});
            }
            if (调用.函数名 == "获取年份") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文), {}, false);
                模块->getOrInsertFunction("获取年份", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取年份"), {});
            }
            if (调用.函数名 == "获取月份") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文), {}, false);
                模块->getOrInsertFunction("获取月份", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取月份"), {});
            }
            if (调用.函数名 == "获取日") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文), {}, false);
                模块->getOrInsertFunction("获取日", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取日"), {});
            }
            if (调用.函数名 == "获取小时") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文), {}, false);
                模块->getOrInsertFunction("获取小时", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取小时"), {});
            }
            if (调用.函数名 == "获取分钟") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文), {}, false);
                模块->getOrInsertFunction("获取分钟", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取分钟"), {});
            }
            if (调用.函数名 == "获取秒") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文), {}, false);
                模块->getOrInsertFunction("获取秒", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取秒"), {});
            }
            if (调用.函数名 == "获取星期") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文), {}, false);
                模块->getOrInsertFunction("获取星期", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取星期"), {});
            }
            if (调用.函数名 == "获取日期时间") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0), {}, false);
                模块->getOrInsertFunction("获取日期时间", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取日期时间"), {});
            }
            if (调用.函数名 == "获取日期") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0), {}, false);
                模块->getOrInsertFunction("获取日期", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取日期"), {});
            }
            if (调用.函数名 == "获取时间") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0), {}, false);
                模块->getOrInsertFunction("获取时间", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取时间"), {});
            }

            // JSON函数
            if (调用.函数名 == "JSON获取字符串") {
                llvm::Value* JSON = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 键 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("JSON获取字符串", 函数类型);
                return 构建器->CreateCall(模块->getFunction("JSON获取字符串"), {JSON, 键});
            }
            if (调用.函数名 == "JSON获取整数") {
                llvm::Value* JSON = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 键 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("JSON获取整数", 函数类型);
                return 构建器->CreateCall(模块->getFunction("JSON获取整数"), {JSON, 键});
            }
            if (调用.函数名 == "JSON获取浮点") {
                llvm::Value* JSON = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 键 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("JSON获取浮点", 函数类型);
                return 构建器->CreateCall(模块->getFunction("JSON获取浮点"), {JSON, 键});
            }
            if (调用.函数名 == "JSON获取布尔") {
                llvm::Value* JSON = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 键 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("JSON获取布尔", 函数类型);
                return 构建器->CreateCall(模块->getFunction("JSON获取布尔"), {JSON, 键});
            }
            if (调用.函数名 == "JSON存在") {
                llvm::Value* JSON = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 键 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("JSON存在", 函数类型);
                return 构建器->CreateCall(模块->getFunction("JSON存在"), {JSON, 键});
            }
            if (调用.函数名 == "JSON创建字符串") {
                llvm::Value* 键 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 值 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("JSON创建字符串", 函数类型);
                return 构建器->CreateCall(模块->getFunction("JSON创建字符串"), {键, 值});
            }
            if (调用.函数名 == "JSON创建整数") {
                llvm::Value* 键 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 值 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("JSON创建整数", 函数类型);
                return 构建器->CreateCall(模块->getFunction("JSON创建整数"), {键, 值});
            }

            // 加密函数
            if (调用.函数名 == "简单哈希") {
                llvm::Value* 输入 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt64Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("简单哈希", 函数类型);
                return 构建器->CreateCall(模块->getFunction("简单哈希"), {输入});
            }
            if (调用.函数名 == "简单加密") {
                llvm::Value* 输入 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 密钥 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("简单加密", 函数类型);
                return 构建器->CreateCall(模块->getFunction("简单加密"), {输入, 密钥});
            }
            if (调用.函数名 == "简单解密") {
                llvm::Value* 输入 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 密钥 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("简单解密", 函数类型);
                return 构建器->CreateCall(模块->getFunction("简单解密"), {输入, 密钥});
            }
            if (调用.函数名 == "随机数") {
                llvm::Value* 最小值 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 最大值 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::Type::getInt32Ty(上下文), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("随机数", 函数类型);
                return 构建器->CreateCall(模块->getFunction("随机数"), {最小值, 最大值});
            }
            if (调用.函数名 == "随机浮点") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文), {}, false);
                模块->getOrInsertFunction("随机浮点", 函数类型);
                return 构建器->CreateCall(模块->getFunction("随机浮点"), {});
            }

            // 计时函数
            if (调用.函数名 == "开始计时") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(上下文), {}, false);
                模块->getOrInsertFunction("开始计时", 函数类型);
                return 构建器->CreateCall(模块->getFunction("开始计时"), {});
            }
            if (调用.函数名 == "结束计时") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文), {}, false);
                模块->getOrInsertFunction("结束计时", 函数类型);
                return 构建器->CreateCall(模块->getFunction("结束计时"), {});
            }

            // 系统函数
            if (调用.函数名 == "获取当前目录") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0), {}, false);
                模块->getOrInsertFunction("获取当前目录", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取当前目录"), {});
            }
            if (调用.函数名 == "文件存在") {
                llvm::Value* 路径 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("文件存在", 函数类型);
                return 构建器->CreateCall(模块->getFunction("文件存在"), {路径});
            }
            if (调用.函数名 == "获取参数数量") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文), {}, false);
                模块->getOrInsertFunction("获取参数数量", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取参数数量"), {});
            }
            if (调用.函数名 == "获取参数") {
                llvm::Value* 索引 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("获取参数", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取参数"), {索引});
            }
            if (调用.函数名 == "输出错误") {
                llvm::Value* 消息 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("输出错误", 函数类型);
                return 构建器->CreateCall(模块->getFunction("输出错误"), {消息});
            }
            if (调用.函数名 == "创建目录") {
                llvm::Value* 路径 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("创建目录", 函数类型);
                return 构建器->CreateCall(模块->getFunction("创建目录"), {路径});
            }
            if (调用.函数名 == "目录存在") {
                llvm::Value* 路径 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("目录存在", 函数类型);
                return 构建器->CreateCall(模块->getFunction("目录存在"), {路径});
            }
            if (调用.函数名 == "删除文件") {
                llvm::Value* 路径 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("删除文件", 函数类型);
                return 构建器->CreateCall(模块->getFunction("删除文件"), {路径});
            }
            if (调用.函数名 == "列出目录") {
                llvm::Value* 路径 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("列出目录", 函数类型);
                return 构建器->CreateCall(模块->getFunction("列出目录"), {路径});
            }
            if (调用.函数名 == "获取目录项名称") {
                llvm::Value* 索引 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("获取目录项名称", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取目录项名称"), {索引});
            }
            if (调用.函数名 == "获取目录项是否目录") {
                llvm::Value* 索引 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("获取目录项是否目录", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取目录项是否目录"), {索引});
            }
            if (调用.函数名 == "获取目录项大小") {
                llvm::Value* 索引 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt64Ty(上下文),
                    {llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("获取目录项大小", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取目录项大小"), {索引});
            }
            if (调用.函数名 == "递归遍历目录") {
                llvm::Value* 路径 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 深度 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("递归遍历目录", 函数类型);
                return 构建器->CreateCall(模块->getFunction("递归遍历目录"), {路径, 深度});
            }
            if (调用.函数名 == "删除目录") {
                llvm::Value* 路径 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("删除目录", 函数类型);
                return 构建器->CreateCall(模块->getFunction("删除目录"), {路径});
            }
            if (调用.函数名 == "复制文件") {
                llvm::Value* 源路径 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 目标路径 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("复制文件", 函数类型);
                return 构建器->CreateCall(模块->getFunction("复制文件"), {源路径, 目标路径});
            }
            if (调用.函数名 == "获取文件扩展名") {
                llvm::Value* 路径 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("获取文件扩展名", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取文件扩展名"), {路径});
            }
            if (调用.函数名 == "获取文件名") {
                llvm::Value* 路径 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("获取文件名", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取文件名"), {路径});
            }
            if (调用.函数名 == "获取目录路径") {
                llvm::Value* 路径 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("获取目录路径", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取目录路径"), {路径});
            }
            if (调用.函数名 == "连接路径") {
                llvm::Value* 路径1 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 路径2 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("连接路径", 函数类型);
                return 构建器->CreateCall(模块->getFunction("连接路径"), {路径1, 路径2});
            }
            if (调用.函数名 == "获取文件大小") {
                llvm::Value* 路径 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt64Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("获取文件大小", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取文件大小"), {路径});
            }
            if (调用.函数名 == "重命名文件") {
                llvm::Value* 旧路径 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 新路径 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("重命名文件", 函数类型);
                return 构建器->CreateCall(模块->getFunction("重命名文件"), {旧路径, 新路径});
            }
            if (调用.函数名 == "分割行数") {
                llvm::Value* 文本 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("分割行数", 函数类型);
                return 构建器->CreateCall(模块->getFunction("分割行数"), {文本});
            }
            if (调用.函数名 == "获取行") {
                llvm::Value* 文本 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 行号 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("获取行", 函数类型);
                return 构建器->CreateCall(模块->getFunction("获取行"), {文本, 行号});
            }
            if (调用.函数名 == "去除空白") {
                llvm::Value* 文本 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("去除空白", 函数类型);
                return 构建器->CreateCall(模块->getFunction("去除空白"), {文本});
            }
            if (调用.函数名 == "转小写") {
                llvm::Value* 文本 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("转小写", 函数类型);
                return 构建器->CreateCall(模块->getFunction("转小写"), {文本});
            }
            if (调用.函数名 == "转大写") {
                llvm::Value* 文本 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("转大写", 函数类型);
                return 构建器->CreateCall(模块->getFunction("转大写"), {文本});
            }
            if (调用.函数名 == "字符串开头") {
                llvm::Value* 文本 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 前缀 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("字符串开头", 函数类型);
                return 构建器->CreateCall(模块->getFunction("字符串开头"), {文本, 前缀});
            }
            if (调用.函数名 == "字符串结尾") {
                llvm::Value* 文本 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 后缀 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("字符串结尾", 函数类型);
                return 构建器->CreateCall(模块->getFunction("字符串结尾"), {文本, 后缀});
            }
            if (调用.函数名 == "字符码") {
                llvm::Value* 文本 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 位置 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("字符码", 函数类型);
                return 构建器->CreateCall(模块->getFunction("字符码"), {文本, 位置});
            }
            if (调用.函数名 == "字符") {
                llvm::Value* 码点 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0),
                    {llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("字符", 函数类型);
                return 构建器->CreateCall(模块->getFunction("字符"), {码点});
            }

            // 动态数组函数
            if (调用.函数名 == "创建动态数组") {
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::PointerType::get(上下文, 0), {}, false);
                模块->getOrInsertFunction("创建动态数组", 函数类型);
                return 构建器->CreateCall(模块->getFunction("创建动态数组"), {});
            }
            if (调用.函数名 == "动态数组添加") {
                llvm::Value* 数组 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 值 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("动态数组添加", 函数类型);
                return 构建器->CreateCall(模块->getFunction("动态数组添加"), {数组, 值});
            }
            if (调用.函数名 == "动态数组获取") {
                llvm::Value* 数组 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 索引 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("动态数组获取", 函数类型);
                return 构建器->CreateCall(模块->getFunction("动态数组获取"), {数组, 索引});
            }
            if (调用.函数名 == "动态数组设置") {
                llvm::Value* 数组 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 索引 = 生成表达式(*调用.参数列表[1]);
                llvm::Value* 值 = 生成表达式(*调用.参数列表[2]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("动态数组设置", 函数类型);
                return 构建器->CreateCall(模块->getFunction("动态数组设置"), {数组, 索引, 值});
            }
            if (调用.函数名 == "动态数组大小" || 调用.函数名 == "求大小") {
                llvm::Value* 数组 = 生成表达式(*调用.参数列表[0]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文),
                    {llvm::PointerType::get(上下文, 0)}, false);
                模块->getOrInsertFunction("动态数组大小", 函数类型);
                return 构建器->CreateCall(模块->getFunction("动态数组大小"), {数组});
            }
            if (调用.函数名 == "动态数组删除") {
                llvm::Value* 数组 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 索引 = 生成表达式(*调用.参数列表[1]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("动态数组删除", 函数类型);
                return 构建器->CreateCall(模块->getFunction("动态数组删除"), {数组, 索引});
            }
            if (调用.函数名 == "动态数组插入") {
                llvm::Value* 数组 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 索引 = 生成表达式(*调用.参数列表[1]);
                llvm::Value* 值 = 生成表达式(*调用.参数列表[2]);
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(上下文),
                    {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文), llvm::Type::getInt32Ty(上下文)}, false);
                模块->getOrInsertFunction("动态数组插入", 函数类型);
                return 构建器->CreateCall(模块->getFunction("动态数组插入"), {数组, 索引, 值});
            }

            // 数学扩展函数
            if (调用.函数名 == "幂运算") {
                llvm::Value* 底数 = 生成表达式(*调用.参数列表[0]);
                llvm::Value* 指数 = 生成表达式(*调用.参数列表[1]);
                if (底数->getType()->isIntegerTy()) 底数 = 构建器->CreateSIToFP(底数, llvm::Type::getDoubleTy(上下文));
                if (指数->getType()->isIntegerTy()) 指数 = 构建器->CreateSIToFP(指数, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文),
                    {llvm::Type::getDoubleTy(上下文), llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("幂运算", 函数类型);
                return 构建器->CreateCall(模块->getFunction("幂运算"), {底数, 指数});
            }
            if (调用.函数名 == "平方根") {
                llvm::Value* 数 = 生成表达式(*调用.参数列表[0]);
                if (数->getType()->isIntegerTy()) 数 = 构建器->CreateSIToFP(数, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文),
                    {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("平方根", 函数类型);
                return 构建器->CreateCall(模块->getFunction("平方根"), {数});
            }
            if (调用.函数名 == "正弦") {
                llvm::Value* 弧度 = 生成表达式(*调用.参数列表[0]);
                if (弧度->getType()->isIntegerTy()) 弧度 = 构建器->CreateSIToFP(弧度, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文),
                    {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("正弦", 函数类型);
                return 构建器->CreateCall(模块->getFunction("正弦"), {弧度});
            }
            if (调用.函数名 == "余弦") {
                llvm::Value* 弧度 = 生成表达式(*调用.参数列表[0]);
                if (弧度->getType()->isIntegerTy()) 弧度 = 构建器->CreateSIToFP(弧度, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文),
                    {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("余弦", 函数类型);
                return 构建器->CreateCall(模块->getFunction("余弦"), {弧度});
            }
            if (调用.函数名 == "正切") {
                llvm::Value* 弧度 = 生成表达式(*调用.参数列表[0]);
                if (弧度->getType()->isIntegerTy()) 弧度 = 构建器->CreateSIToFP(弧度, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文),
                    {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("正切", 函数类型);
                return 构建器->CreateCall(模块->getFunction("正切"), {弧度});
            }
            if (调用.函数名 == "角度转弧度") {
                llvm::Value* 角度 = 生成表达式(*调用.参数列表[0]);
                if (角度->getType()->isIntegerTy()) 角度 = 构建器->CreateSIToFP(角度, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文),
                    {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("角度转弧度", 函数类型);
                return 构建器->CreateCall(模块->getFunction("角度转弧度"), {角度});
            }
            if (调用.函数名 == "弧度转角度") {
                llvm::Value* 弧度 = 生成表达式(*调用.参数列表[0]);
                if (弧度->getType()->isIntegerTy()) 弧度 = 构建器->CreateSIToFP(弧度, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文),
                    {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("弧度转角度", 函数类型);
                return 构建器->CreateCall(模块->getFunction("弧度转角度"), {弧度});
            }
            if (调用.函数名 == "自然对数") {
                llvm::Value* 数 = 生成表达式(*调用.参数列表[0]);
                if (数->getType()->isIntegerTy()) 数 = 构建器->CreateSIToFP(数, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文),
                    {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("自然对数", 函数类型);
                return 构建器->CreateCall(模块->getFunction("自然对数"), {数});
            }
            if (调用.函数名 == "常用对数") {
                llvm::Value* 数 = 生成表达式(*调用.参数列表[0]);
                if (数->getType()->isIntegerTy()) 数 = 构建器->CreateSIToFP(数, llvm::Type::getDoubleTy(上下文));
                llvm::FunctionType* 函数类型 = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(上下文),
                    {llvm::Type::getDoubleTy(上下文)}, false);
                模块->getOrInsertFunction("常用对数", 函数类型);
                return 构建器->CreateCall(模块->getFunction("常用对数"), {数});
            }

            // 内置函数：打印
            if (调用.函数名 == "打印") {
                llvm::FunctionType* printf类型 = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(上下文), {llvm::PointerType::get(上下文, 0)}, true);
                模块->getOrInsertFunction("printf", printf类型);
                if (调用.参数列表.empty()) {
                    return 构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("\n")});
                }
                llvm::Value* 值 = 生成表达式(*调用.参数列表[0]);
                if (值->getType()->isDoubleTy() || 值->getType()->isFloatTy()) {
                    return 构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("%.6g\n"), 值});
                } else if (值->getType()->isPointerTy()) {
                    return 构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("%s\n"), 值});
                } else {
                    return 构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("%d\n"), 值});
                }
            }

            // 用户自定义函数
            llvm::Function* 目标函数 = 模块->getFunction(调用.函数名);
            const 闭包信息* 闭包 = nullptr;
            if (!目标函数) { 闭包 = 符号表实例.获取闭包信息(调用.函数名); if (闭包) 目标函数 = 闭包->函数; }

            // 查找函数定义以获取默认参数和变长参数信息
            const 函数* 函数定义 = nullptr;
            auto 定义迭代 = 全局函数定义映射.find(调用.函数名);
            if (定义迭代 != 全局函数定义映射.end()) 函数定义 = 定义迭代->second;

            // 处理泛型函数：单态化
            std::string 实际函数名 = 调用.函数名;
            if (函数定义 && !函数定义->类型参数列表.empty()) {
                // 确定具体类型
                std::vector<std::string> 具体类型;
                for (size_t i = 0; i < 调用.参数列表.size() && i < 函数定义->参数列表.size(); ++i) {
                    std::string 类型名 = "整数";
                    if (调用.参数列表[i]->类型 == 表达式类型::浮点数) 类型名 = "浮点数";
                    else if (调用.参数列表[i]->类型 == 表达式类型::字符串) 类型名 = "字符串";
                    else if (调用.参数列表[i]->类型 == 表达式类型::变量) {
                        const auto& 变量名 = static_cast<const 变量表达式&>(*调用.参数列表[i]).名称;
                        if (符号表实例.是浮点变量(变量名)) 类型名 = "浮点数";
                        else if (符号表实例.是指针变量(变量名)) 类型名 = "字符串";
                    }
                    具体类型.push_back(类型名);
                }
                // 生成特化函数名
                实际函数名 = 调用.函数名 + "_";
                for (const auto& 类型 : 具体类型) {
                    if (类型 == "整数") 实际函数名 += "I";
                    else if (类型 == "浮点数") 实际函数名 += "F";
                    else if (类型 == "字符串") 实际函数名 += "S";
                    else 实际函数名 += "X";
                }
                // 检查是否已生成特化版本
                目标函数 = 模块->getFunction(实际函数名);
                if (!目标函数) {
                    // 生成特化版本：替换类型参数
                    std::unordered_map<std::string, std::string> 类型替换;
                    for (size_t i = 0; i < 函数定义->类型参数列表.size() && i < 具体类型.size(); ++i) {
                        类型替换[函数定义->类型参数列表[i]] = 具体类型[i];
                    }
                    // 创建特化函数签名
                    std::vector<llvm::Type*> 参数类型;
                    for (size_t i = 0; i < 函数定义->参数列表.size(); ++i) {
                        std::string 类型 = 函数定义->参数列表[i].类型;
                        auto 替换迭代 = 类型替换.find(类型);
                        if (替换迭代 != 类型替换.end()) 类型 = 替换迭代->second;
                        if (函数定义->参数列表[i].是否变长) {
                            参数类型.push_back(llvm::PointerType::get(上下文, 0));
                        } else {
                            参数类型.push_back(类型名到LLVM类型(类型));
                        }
                    }
                    // 计算返回类型（支持多返回值）
                    llvm::Type* 返回LLVM类型 = llvm::Type::getVoidTy(上下文);
                    if (函数定义->返回值列表.size() == 1 && !函数定义->返回值列表[0].类型.empty() && 函数定义->返回值列表[0].类型 != "空") {
                        std::string 返回类型 = 函数定义->返回值列表[0].类型;
                        auto 返回替换 = 类型替换.find(返回类型);
                        if (返回替换 != 类型替换.end()) 返回类型 = 返回替换->second;
                        返回LLVM类型 = 类型名到LLVM类型(返回类型);
                    } else if (函数定义->返回值列表.size() > 1) {
                        std::vector<llvm::Type*> 返回类型列表;
                        for (const auto& 返回值 : 函数定义->返回值列表) {
                            if (!返回值.类型.empty() && 返回值.类型 != "空") {
                                std::string 返回类型 = 返回值.类型;
                                auto 返回替换 = 类型替换.find(返回类型);
                                if (返回替换 != 类型替换.end()) 返回类型 = 返回替换->second;
                                返回类型列表.push_back(类型名到LLVM类型(返回类型));
                            }
                        }
                        if (!返回类型列表.empty()) {
                            返回LLVM类型 = llvm::StructType::get(上下文, 返回类型列表);
                        }
                    }
                    llvm::FunctionType* 函数类型 = llvm::FunctionType::get(返回LLVM类型, 参数类型, false);
                    目标函数 = llvm::Function::Create(函数类型, llvm::Function::ExternalLinkage, 实际函数名, *模块);
                    // 生成特化函数体
                    llvm::BasicBlock* 入口块 = llvm::BasicBlock::Create(上下文, "entry", 目标函数);
                    构建器->SetInsertPoint(入口块);
                    符号表实例.重置为全局作用域();
                    size_t 参数索引 = 0;
                    for (auto& 参数 : 目标函数->args()) {
                        参数.setName(函数定义->参数列表[参数索引].名称);
                        符号表实例.声明变量(函数定义->参数列表[参数索引].名称, &参数);
                        std::string 类型 = 函数定义->参数列表[参数索引].类型;
                        auto 替换迭代 = 类型替换.find(类型);
                        if (替换迭代 != 类型替换.end()) 类型 = 替换迭代->second;
                        if (类型 == "浮点" || 类型 == "浮点数") {
                            符号表实例.设置浮点变量(函数定义->参数列表[参数索引].名称);
                        } else if (类型 == "字符串") {
                            符号表实例.设置指针变量(函数定义->参数列表[参数索引].名称);
                        }
                        参数索引++;
                    }
                    for (const auto& 语句 : 函数定义->主体) 生成语句(*语句);
                    if (构建器->GetInsertBlock()->getTerminator() == nullptr) {
                        if (函数定义->返回值列表.size() == 1 && !函数定义->返回值列表[0].类型.empty() && 函数定义->返回值列表[0].类型 != "空") {
                            构建器->CreateRet(llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)));
                        } else if (函数定义->返回值列表.size() > 1) {
                            // 多返回值：创建默认结构体
                            std::vector<llvm::Type*> 返回类型列表;
                            for (const auto& 返回值 : 函数定义->返回值列表) {
                                if (!返回值.类型.empty() && 返回值.类型 != "空") {
                                    返回类型列表.push_back(类型名到LLVM类型(返回值.类型));
                                }
                            }
                            llvm::StructType* 返回结构体类型 = llvm::StructType::get(上下文, 返回类型列表);
                            llvm::Value* 返回结构体 = llvm::UndefValue::get(返回结构体类型);
                            构建器->CreateRet(返回结构体);
                        } else {
                            构建器->CreateRetVoid();
                        }
                    }
                }
            }

            // 检查是否为函数参数（变量中存储的函数指针）
            llvm::Value* 函数指针变量 = nullptr;
            if (!目标函数) {
                llvm::Value* 变量地址 = 符号表实例.获取变量值(调用.函数名);
                if (变量地址) {
                    // 检查是否为函数指针（直接是 Function*）
                    if (auto* func = llvm::dyn_cast<llvm::Function>(变量地址)) {
                        目标函数 = func;
                    } else {
                        // 加载函数指针
                        函数指针变量 = 构建器->CreateLoad(llvm::PointerType::get(上下文, 0), 变量地址, 调用.函数名 + "_ptr");
                    }
                }
            }

            if (!目标函数 && !函数指针变量) throw std::runtime_error("未定义的函数: " + 调用.函数名);

            // 检查是否有变长参数
            bool 有变长参数 = false;
            size_t 固定参数数量 = 函数定义 ? 函数定义->参数列表.size() : 0;
            if (函数定义 && !函数定义->参数列表.empty() && 函数定义->参数列表.back().是否变长) {
                有变长参数 = true;
                固定参数数量 = 函数定义->参数列表.size() - 1;
            }

            std::vector<llvm::Value*> 参数值;
            size_t 参数索引 = 0;
            size_t 固定参数计数 = 0;
            for (const auto& 参数 : 调用.参数列表) {
                llvm::Value* 值 = 生成表达式(*参数);
                if (有变长参数 && 固定参数计数 >= 固定参数数量) {
                    // 变长参数部分，先收集到临时列表
                    break;
                }
                参数值.push_back(值);
                参数索引++;
                固定参数计数++;
            }

            // 填充默认参数值
            if (函数定义 && !有变长参数 && 调用.参数列表.size() < 函数定义->参数列表.size()) {
                for (size_t i = 调用.参数列表.size(); i < 函数定义->参数列表.size(); ++i) {
                    const auto& 参数定义 = 函数定义->参数列表[i];
                    if (!参数定义.默认值) {
                        throw std::runtime_error("参数 '" + 参数定义.名称 + "' 缺少值且无默认值");
                    }
                    llvm::Value* 默认值 = 生成表达式(*参数定义.默认值);
                    参数值.push_back(默认值);
                    参数索引++;
                }
            }

            // 处理变长参数：收集多余参数为数组
            if (有变长参数) {
                size_t 变长参数数量 = 调用.参数列表.size() > 固定参数数量 ? 调用.参数列表.size() - 固定参数数量 : 0;
                llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::Type::getInt32Ty(上下文), 变长参数数量 > 0 ? 变长参数数量 : 1);
                llvm::AllocaInst* 数组 = 构建器->CreateAlloca(数组类型, nullptr, "变长参数数组");
                for (size_t i = 0; i < 变长参数数量; ++i) {
                    llvm::Value* 值 = 生成表达式(*调用.参数列表[固定参数数量 + i]);
                    llvm::Value* 索引 = llvm::ConstantInt::get(上下文, llvm::APInt(32, static_cast<int64_t>(i)));
                    llvm::Value* 指针 = 构建器->CreateInBoundsGEP(数组类型, 数组, {llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 索引});
                    构建器->CreateStore(值, 指针);
                }
                参数值.push_back(数组);
                // 存储变长参数数量到符号表（供函数内部使用）
                符号表实例.声明变量(函数定义->参数列表.back().名称 + "_数量", llvm::ConstantInt::get(上下文, llvm::APInt(32, static_cast<int64_t>(变长参数数量))));
            }
            if (闭包) for (const auto& 地址 : 闭包->捕获变量地址) 参数值.push_back(地址);
            // 调用函数（支持命名函数和函数指针）
            if (函数指针变量) {
                // 调用函数指针变量 - 创建通用函数类型
                std::vector<llvm::Type*> 参数类型;
                for (const auto& 值 : 参数值) 参数类型.push_back(值->getType());
                llvm::FunctionType* 通用函数类型 = llvm::FunctionType::get(llvm::Type::getInt32Ty(上下文), 参数类型, false);
                return 构建器->CreateCall(通用函数类型, 函数指针变量, 参数值, 调用.函数名 + "_结果");
            }
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
            // 计算返回类型
            llvm::Type* 返回LLVM类型 = llvm::Type::getVoidTy(上下文);
            if (匿名.返回值列表.size() == 1 && !匿名.返回值列表[0].类型.empty() && 匿名.返回值列表[0].类型 != "空") {
                返回LLVM类型 = 类型名到LLVM类型(匿名.返回值列表[0].类型);
            } else if (匿名.返回值列表.size() > 1) {
                // 多返回值：创建结构体类型
                std::vector<llvm::Type*> 返回类型列表;
                for (const auto& 返回值 : 匿名.返回值列表) {
                    if (!返回值.类型.empty() && 返回值.类型 != "空") {
                        返回类型列表.push_back(类型名到LLVM类型(返回值.类型));
                    }
                }
                if (!返回类型列表.empty()) {
                    返回LLVM类型 = llvm::StructType::get(上下文, 返回类型列表);
                }
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
                // 为参数创建 alloca（与普通函数一致）
                llvm::Type* 参数类型 = 类型名到LLVM类型(匿名.参数列表[i].类型);
                llvm::AllocaInst* 分配 = 匿名构建器.CreateAlloca(参数类型, nullptr, 匿名.参数列表[i].名称);
                匿名构建器.CreateStore(&*参数迭代器, 分配);
                符号表实例.声明变量(匿名.参数列表[i].名称, 分配);
                if (匿名.参数列表[i].类型 == "浮点" || 匿名.参数列表[i].类型 == "浮点数") {
                    符号表实例.设置浮点变量(匿名.参数列表[i].名称);
                } else if (匿名.参数列表[i].类型 == "字符串" || 匿名.参数列表[i].类型 == "函数") {
                    符号表实例.设置指针变量(匿名.参数列表[i].名称);
                }
            }
            for (size_t i = 0; i < 捕获变量名.size(); ++i, ++参数迭代器) {
                参数迭代器->setName(捕获变量名[i] + "_ptr");
                符号表实例.声明变量(捕获变量名[i], &*参数迭代器);
            }
            auto 保存构建器 = std::move(构建器);
            构建器 = std::make_unique<llvm::IRBuilder<>>(入口块);

            // 为返回值创建 alloca（支持多返回值）
            std::vector<llvm::AllocaInst*> 返回值分配列表;
            for (const auto& 返回值 : 匿名.返回值列表) {
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

            for (const auto& 子 : 匿名.主体) 生成语句(*子);
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
            llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::PointerType::get(上下文, 0), 256);
            llvm::StructType* 映射结构体类型 = llvm::StructType::create(上下文, {数组类型, 数组类型}, "映射类型");
            static int 映射计数 = 0;
            std::string 前缀 = ".映射" + std::to_string(映射计数++);
            auto* 映射全局 = new llvm::GlobalVariable(*模块, 映射结构体类型, false, llvm::GlobalValue::InternalLinkage,
                llvm::ConstantAggregateZero::get(映射结构体类型), 前缀);
            llvm::Value* 零 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
            llvm::Value* 键数组地址 = 构建器->CreateInBoundsGEP(映射结构体类型, 映射全局, {零, 零}, "键数组地址");
            llvm::Value* 值数组地址 = 构建器->CreateInBoundsGEP(映射结构体类型, 映射全局, {零, llvm::ConstantInt::get(上下文, llvm::APInt(32, 1))}, "值数组地址");
            llvm::AllocaInst* 大小变量 = 构建器->CreateAlloca(llvm::Type::getInt32Ty(上下文), nullptr, "映射大小");
            构建器->CreateStore(llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 大小变量);
            for (int i = 0; i < 大小; i++) {
                llvm::Value* 键 = 生成表达式(*映射.键值对列表[i].first);
                llvm::Value* 值 = 生成表达式(*映射.键值对列表[i].second);
                llvm::Value* 当前大小 = 构建器->CreateLoad(llvm::Type::getInt32Ty(上下文), 大小变量);
                llvm::Value* 新大小 = 构建器->CreateCall(模块->getFunction("映射设置"),
                    {键数组地址, 值数组地址, 当前大小, 键, 值});
                构建器->CreateStore(新大小, 大小变量);
            }
            return 映射全局;
        }
        case 表达式类型::下标访问: {
            const auto& 下标 = static_cast<const 下标访问表达式&>(表达式);
            llvm::Value* 数组地址 = 符号表实例.获取变量值(下标.数组名);
            if (!数组地址) throw std::runtime_error("未定义的变量: " + 下标.数组名);
            if (符号表实例.是映射变量(下标.数组名)) {
                llvm::Value* 键 = 生成表达式(*下标.索引);
                llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::PointerType::get(上下文, 0), 256);
                llvm::StructType* 映射结构体类型 = llvm::StructType::get(上下文, {数组类型, 数组类型});
                llvm::Value* 零 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                llvm::Value* 键数组地址 = 构建器->CreateInBoundsGEP(映射结构体类型, 数组地址, {零, 零}, "键数组地址");
                llvm::Value* 值数组地址 = 构建器->CreateInBoundsGEP(映射结构体类型, 数组地址, {零, llvm::ConstantInt::get(上下文, llvm::APInt(32, 1))}, "值数组地址");
                llvm::Value* 大小 = llvm::ConstantInt::get(上下文, llvm::APInt(32, 256));
                return 构建器->CreateCall(模块->getFunction("映射获取"), {键数组地址, 值数组地址, 大小, 键});
            }
            llvm::Value* 索引值 = 生成表达式(*下标.索引);
            // 检查是否是 alloca（本地数组）还是指针（动态数组）
            if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(数组地址)) {
                llvm::Type* 分配类型 = alloca->getAllocatedType();
                if (分配类型->isArrayTy()) {
                    // 本地数组：直接使用 GEP
                    llvm::ArrayType* 数组类型 = llvm::cast<llvm::ArrayType>(分配类型);
                    llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(数组类型, 数组地址, {llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 索引值}, 下标.数组名 + "_元素");
                    return 构建器->CreateLoad(数组类型->getElementType(), 元素地址, 下标.数组名 + "_值");
                }
            }
            // 动态数组：先加载指针
            llvm::Value* 实际数组地址 = 构建器->CreateLoad(llvm::PointerType::get(上下文, 0), 数组地址, 下标.数组名 + "_加载");
            if (符号表实例.是字符串数组(下标.数组名)) {
                llvm::ArrayType* 数组类型 = llvm::ArrayType::get(llvm::PointerType::get(上下文, 0), 符号表实例.获取数组大小(下标.数组名));
                llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(数组类型, 实际数组地址, {llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)), 索引值}, 下标.数组名 + "_元素");
                return 构建器->CreateLoad(llvm::PointerType::get(上下文, 0), 元素地址, 下标.数组名 + "_值");
            }
            llvm::Value* 元素地址 = 构建器->CreateInBoundsGEP(llvm::Type::getInt32Ty(上下文), 实际数组地址, 索引值, 下标.数组名 + "_元素");
            return 构建器->CreateLoad(llvm::Type::getInt32Ty(上下文), 元素地址, 下标.数组名 + "_值");
        }
        case 表达式类型::分片访问: {
            const auto& 分片 = static_cast<const 分片访问表达式&>(表达式);
            llvm::Value* 数组地址 = 符号表实例.获取变量值(分片.数组名);
            if (!数组地址) throw std::runtime_error("未定义的变量: " + 分片.数组名);
            // 变量存储的是指向数组的指针，需要先加载
            llvm::Value* 实际数组地址 = 构建器->CreateLoad(llvm::PointerType::get(上下文, 0), 数组地址, 分片.数组名 + "_加载");
            llvm::Value* 开始值 = 分片.开始索引 ? 生成表达式(*分片.开始索引) : llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
            // 返回指向分片起始位置的指针
            llvm::Value* 分片地址 = 构建器->CreateInBoundsGEP(llvm::Type::getInt32Ty(上下文), 实际数组地址, 开始值, 分片.数组名 + "_分片");
            return 分片地址;
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
        case 表达式类型::自增自减: {
            const auto& 自增自减 = static_cast<const 自增自减表达式&>(表达式);
            llvm::Value* 地址 = 符号表实例.获取变量值(自增自减.变量名);
            if (!地址) throw std::runtime_error("未定义的变量: " + 自增自减.变量名);
            llvm::Value* 当前值 = 构建器->CreateLoad(llvm::Type::getInt32Ty(上下文), 地址, 自增自减.变量名);
            llvm::Value* 新值;
            if (自增自减.是自增) {
                新值 = 构建器->CreateAdd(当前值, llvm::ConstantInt::get(上下文, llvm::APInt(32, 1)), "自增");
            } else {
                新值 = 构建器->CreateSub(当前值, llvm::ConstantInt::get(上下文, llvm::APInt(32, 1)), "自减");
            }
            构建器->CreateStore(新值, 地址);
            return 新值;
        }
        case 表达式类型::管道调用: {
            const auto& 管道 = static_cast<const 管道调用表达式&>(表达式);

            // 1. 先执行左边的表达式（通常是函数调用）
            llvm::Value* 左结果 = 生成表达式(*管道.左表达式);

            // 2. 查找右边的函数
            llvm::Function* 右函数 = 模块->getFunction(管道.右函数名);

            // 内置函数：打印
            if (管道.右函数名 == "打印") {
                llvm::FunctionType* printf类型 = llvm::FunctionType::get(llvm::Type::getInt32Ty(上下文), {llvm::PointerType::get(上下文, 0)}, true);
                模块->getOrInsertFunction("printf", printf类型);
                if (左结果->getType()->isDoubleTy() || 左结果->getType()->isFloatTy()) {
                    return 构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("%.6g\n"), 左结果});
                } else if (左结果->getType()->isPointerTy()) {
                    return 构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("%s\n"), 左结果});
                } else {
                    return 构建器->CreateCall(模块->getFunction("printf"), {构建器->CreateGlobalString("%d\n"), 左结果});
                }
            }

            if (!右函数) throw std::runtime_error("管道右侧未定义的函数: " + 管道.右函数名);

            // 3. 查找函数定义以获取参数信息
            const 函数* 右函数定义 = nullptr;
            auto 定义迭代 = 全局函数定义映射.find(管道.右函数名);
            if (定义迭代 != 全局函数定义映射.end()) 右函数定义 = 定义迭代->second;

            // 4. 构建参数列表
            std::vector<llvm::Value*> 参数值;

            if (!管道.参数映射.empty() && 右函数定义) {
                // 有映射：按照映射关系放置参数
                // 初始化所有参数为默认值或零
                参数值.resize(右函数定义->参数列表.size(), nullptr);

                // 找到映射目标参数的索引
                for (const auto& [左名, 右名] : 管道.参数映射) {
                    for (size_t i = 0; i < 右函数定义->参数列表.size(); i++) {
                        if (右函数定义->参数列表[i].名称 == 右名) {
                            参数值[i] = 左结果;
                            break;
                        }
                    }
                }

                // 填充额外参数到未映射的位置
                size_t 额外参数索引 = 0;
                for (size_t i = 0; i < 参数值.size() && 额外参数索引 < 管道.右参数列表.size(); i++) {
                    if (!参数值[i]) {
                        参数值[i] = 生成表达式(*管道.右参数列表[额外参数索引]);
                        额外参数索引++;
                    }
                }

                // 填充未映射的参数为默认值或零
                for (size_t i = 0; i < 参数值.size(); i++) {
                    if (!参数值[i]) {
                        if (右函数定义->参数列表[i].默认值) {
                            参数值[i] = 生成表达式(*右函数定义->参数列表[i].默认值);
                        } else {
                            llvm::Type* 参数类型 = 右函数->getArg(i)->getType();
                            if (参数类型->isDoubleTy()) {
                                参数值[i] = llvm::ConstantFP::get(上下文, llvm::APFloat(0.0));
                            } else if (参数类型->isPointerTy()) {
                                参数值[i] = llvm::ConstantPointerNull::get(llvm::PointerType::get(上下文, 0));
                            } else {
                                参数值[i] = llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
                            }
                        }
                    }
                }
            } else {
                // 无映射：左结果作为第一个参数
                参数值.push_back(左结果);
                // 添加额外参数
                for (const auto& 参数 : 管道.右参数列表) {
                    参数值.push_back(生成表达式(*参数));
                }
                // 填充默认参数
                if (右函数定义) {
                    while (参数值.size() < 右函数定义->参数列表.size()) {
                        size_t i = 参数值.size();
                        if (右函数定义->参数列表[i].默认值) {
                            参数值.push_back(生成表达式(*右函数定义->参数列表[i].默认值));
                        } else {
                            llvm::Type* 参数类型 = 右函数->getArg(i)->getType();
                            if (参数类型->isDoubleTy()) {
                                参数值.push_back(llvm::ConstantFP::get(上下文, llvm::APFloat(0.0)));
                            } else if (参数类型->isPointerTy()) {
                                参数值.push_back(llvm::ConstantPointerNull::get(llvm::PointerType::get(上下文, 0)));
                            } else {
                                参数值.push_back(llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)));
                            }
                        }
                    }
                }
            }

            // 5. 调用右边的函数
            if (右函数->getReturnType()->isVoidTy()) {
                构建器->CreateCall(右函数, 参数值);
                return llvm::ConstantInt::get(上下文, llvm::APInt(32, 0));
            }
            return 构建器->CreateCall(右函数, 参数值, 管道.右函数名 + "_管道结果");
        }
        case 表达式类型::条件表达式: {
            const auto& 条件表达 = static_cast<const 条件表达式&>(表达式);
            llvm::Value* 条件值 = 生成表达式(*条件表达.条件);
            
            // 转换条件为布尔值
            llvm::Value* 条件布尔 = 条件值;
            if (条件值->getType()->isIntegerTy()) {
                条件布尔 = 构建器->CreateICmpNE(条件值, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)));
            }
            
            // 创建基本块
            llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();
            llvm::BasicBlock* 真值块 = llvm::BasicBlock::Create(上下文, "三元真", 当前函数);
            llvm::BasicBlock* 假值块 = llvm::BasicBlock::Create(上下文, "三元假");
            llvm::BasicBlock* 合并块 = llvm::BasicBlock::Create(上下文, "三元合并");
            
            构建器->CreateCondBr(条件布尔, 真值块, 假值块);
            
            // 生成真值
            构建器->SetInsertPoint(真值块);
            llvm::Value* 真值 = 生成表达式(*条件表达.真值);
            构建器->CreateBr(合并块);
            真值块 = 构建器->GetInsertBlock();
            
            // 生成假值
            当前函数->insert(当前函数->end(), 假值块);
            构建器->SetInsertPoint(假值块);
            llvm::Value* 假值 = 生成表达式(*条件表达.假值);
            构建器->CreateBr(合并块);
            假值块 = 构建器->GetInsertBlock();
            
            // 合并
            当前函数->insert(当前函数->end(), 合并块);
            构建器->SetInsertPoint(合并块);
            
            // 创建 PHI 节点
            llvm::PHINode* phi = 构建器->CreatePHI(真值->getType(), 2);
            phi->addIncoming(真值, 真值块);
            phi->addIncoming(假值, 假值块);
            return phi;
        }
        case 表达式类型::空值合并: {
            const auto& 空值合并表达 = static_cast<const 空值合并表达式&>(表达式);
            llvm::Value* 左值 = 生成表达式(*空值合并表达.左表达式);
            llvm::Value* 右值 = 生成表达式(*空值合并表达.右表达式);
            
            // 对于整数类型，检查左值是否为0（空值）
            if (左值->getType()->isIntegerTy() && 右值->getType()->isIntegerTy()) {
                // 创建基本块
                llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();
                llvm::BasicBlock* 检查块 = 构建器->GetInsertBlock();
                llvm::BasicBlock* 使用默认块 = llvm::BasicBlock::Create(上下文, "空值使用默认", 当前函数);
                llvm::BasicBlock* 合并块 = llvm::BasicBlock::Create(上下文, "空值合并", 当前函数);
                
                // 检查左值是否为0
                llvm::Value* 是空 = 构建器->CreateICmpEQ(左值, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)));
                构建器->CreateCondBr(是空, 使用默认块, 合并块);
                
                // 使用默认值
                构建器->SetInsertPoint(使用默认块);
                构建器->CreateBr(合并块);
                使用默认块 = 构建器->GetInsertBlock();
                
                // 合并
                构建器->SetInsertPoint(合并块);
                
                llvm::PHINode* phi = 构建器->CreatePHI(左值->getType(), 2);
                phi->addIncoming(左值, 检查块);
                phi->addIncoming(右值, 使用默认块);
                return phi;
            }
            
            // 对于指针类型，检查左值是否为null
            if (左值->getType()->isPointerTy() && 右值->getType()->isPointerTy()) {
                llvm::Function* 当前函数 = 构建器->GetInsertBlock()->getParent();
                llvm::BasicBlock* 检查块 = 构建器->GetInsertBlock();
                llvm::BasicBlock* 使用默认块 = llvm::BasicBlock::Create(上下文, "空值使用默认", 当前函数);
                llvm::BasicBlock* 合并块 = llvm::BasicBlock::Create(上下文, "空值合并", 当前函数);
                
                llvm::Value* 是空 = 构建器->CreateICmpEQ(左值, llvm::ConstantPointerNull::get(llvm::PointerType::get(上下文, 0)));
                构建器->CreateCondBr(是空, 使用默认块, 合并块);
                
                构建器->SetInsertPoint(使用默认块);
                构建器->CreateBr(合并块);
                使用默认块 = 构建器->GetInsertBlock();
                
                构建器->SetInsertPoint(合并块);
                
                llvm::PHINode* phi = 构建器->CreatePHI(左值->getType(), 2);
                phi->addIncoming(左值, 检查块);
                phi->addIncoming(右值, 使用默认块);
                return phi;
            }
            
            // 默认返回左值
            return 左值;
        }
        default:
            throw std::runtime_error("未知表达式类型");
    }
}

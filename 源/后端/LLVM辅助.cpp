#include "LLVM辅助.h"
#include "符号表.h"

llvm::Value* 确保浮点(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Value* 值) {
    if (值->getType()->isIntegerTy()) {
        return 构建器.CreateSIToFP(值, llvm::Type::getDoubleTy(上下文));
    }
    return 值;
}

llvm::Value* 拼接字符串(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    llvm::Value* 左, llvm::Value* 右) {
    llvm::Type* i8Ptr = llvm::PointerType::get(上下文, 0);
    llvm::FunctionType* sprintf类型 = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(上下文), {i8Ptr, i8Ptr}, true);
    llvm::FunctionType* malloc类型 = llvm::FunctionType::get(i8Ptr, {llvm::Type::getInt64Ty(上下文)}, false);
    llvm::FunctionType* strlen类型 = llvm::FunctionType::get(llvm::Type::getInt64Ty(上下文), {i8Ptr}, false);
    模块->getOrInsertFunction("sprintf", sprintf类型);
    模块->getOrInsertFunction("malloc", malloc类型);
    模块->getOrInsertFunction("strlen", strlen类型);

    llvm::Value* 左长度 = 构建器.CreateCall(模块->getFunction("strlen"), {左}, "左长度");
    llvm::Value* 右长度 = 构建器.CreateCall(模块->getFunction("strlen"), {右}, "右长度");
    llvm::Value* 总长度 = 构建器.CreateAdd(左长度, 右长度, "总长度");
    llvm::Value* 缓冲区大小 = 构建器.CreateAdd(总长度, llvm::ConstantInt::get(上下文, llvm::APInt(64, 1)), "缓冲区大小");
    llvm::Value* 缓冲区 = 构建器.CreateCall(模块->getFunction("malloc"), {缓冲区大小}, "拼接缓冲区");
    构建器.CreateCall(模块->getFunction("sprintf"), {缓冲区, 构建器.CreateGlobalString("%s%s"), 左, 右});
    return 缓冲区;
}

static llvm::Value* 剥离末尾换行(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    llvm::Value* 缓冲区) {
    llvm::FunctionType* strlen类型 = llvm::FunctionType::get(
        llvm::Type::getInt64Ty(上下文), {llvm::PointerType::get(上下文, 0)}, false);
    模块->getOrInsertFunction("strlen", strlen类型);
    llvm::Value* 长度 = 构建器.CreateCall(模块->getFunction("strlen"), {缓冲区}, "缓冲区长度");
    llvm::Value* 长度大于零 = 构建器.CreateICmpUGT(长度, llvm::ConstantInt::get(上下文, llvm::APInt(64, 0)));
    llvm::Value* 最后位置 = 构建器.CreateSub(长度, llvm::ConstantInt::get(上下文, llvm::APInt(64, 1)));
    llvm::Value* 安全位置 = 构建器.CreateSelect(长度大于零, 最后位置, llvm::ConstantInt::get(上下文, llvm::APInt(64, 0)));
    llvm::Value* 最后字符地址 = 构建器.CreateInBoundsGEP(llvm::Type::getInt8Ty(上下文), 缓冲区, 安全位置);
    llvm::Value* 最后字符 = 构建器.CreateLoad(llvm::Type::getInt8Ty(上下文), 最后字符地址);
    llvm::Value* 是换行 = 构建器.CreateICmpEQ(最后字符, llvm::ConstantInt::get(上下文, llvm::APInt(8, '\n')));
    llvm::Value* 应替换 = 构建器.CreateAnd(长度大于零, 是换行);
    llvm::Value* 空终止 = llvm::ConstantInt::get(llvm::Type::getInt8Ty(上下文), 0);
    llvm::Value* 原始字节 = 构建器.CreateLoad(llvm::Type::getInt8Ty(上下文), 最后字符地址, "原始字节");
    llvm::Value* 最终字节 = 构建器.CreateSelect(应替换, 空终止, 原始字节, "最终字节");
    构建器.CreateStore(最终字节, 最后字符地址);
    return 缓冲区;
}

llvm::Value* 读取行并剥离换行(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    llvm::Value* 文件指针, int 缓冲大小) {
    llvm::FunctionType* fgets类型 = llvm::FunctionType::get(
        llvm::PointerType::get(上下文, 0),
        {llvm::PointerType::get(上下文, 0), llvm::Type::getInt32Ty(上下文), llvm::PointerType::get(上下文, 0)}, false);
    llvm::FunctionType* malloc类型 = llvm::FunctionType::get(
        llvm::PointerType::get(上下文, 0), {llvm::Type::getInt64Ty(上下文)}, false);
    模块->getOrInsertFunction("fgets", fgets类型);
    模块->getOrInsertFunction("malloc", malloc类型);

    llvm::Value* 缓冲区 = 构建器.CreateCall(模块->getFunction("malloc"),
        {llvm::ConstantInt::get(上下文, llvm::APInt(64, 缓冲大小))}, "读取缓冲区");
    构建器.CreateCall(模块->getFunction("fgets"), {缓冲区,
        llvm::ConstantInt::get(上下文, llvm::APInt(32, 缓冲大小)), 文件指针});
    return 剥离末尾换行(构建器, 上下文, 模块, 缓冲区);
}

llvm::Value* 读取标准输入行(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    int 缓冲大小) {
    llvm::Value* stdinPtr = 模块->getNamedGlobal("stdin");
    if (!stdinPtr) {
        auto* stdinGV = new llvm::GlobalVariable(*模块, llvm::PointerType::get(上下文, 0),
            false, llvm::GlobalValue::ExternalLinkage, nullptr, "stdin");
        stdinGV->setDLLStorageClass(llvm::GlobalValue::DLLImportStorageClass);
        stdinPtr = stdinGV;
    }
    stdinPtr = 构建器.CreateLoad(llvm::PointerType::get(上下文, 0), stdinPtr, "stdin_val");
    return 读取行并剥离换行(构建器, 上下文, 模块, stdinPtr, 缓冲大小);
}

llvm::Value* 调用外部函数0(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    const std::string& 名称, llvm::Type* 返回类型) {
    llvm::FunctionType* 函数类型 = llvm::FunctionType::get(返回类型, {}, false);
    模块->getOrInsertFunction(名称, 函数类型);
    return 构建器.CreateCall(模块->getFunction(名称), {});
}

llvm::Value* 调用数学函数1(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    const std::string& C名称, const std::string& 显示名称, llvm::Value* 参数) {
    参数 = 确保浮点(构建器, 上下文, 参数);
    llvm::Type* f64 = llvm::Type::getDoubleTy(上下文);
    llvm::FunctionType* 函数类型 = llvm::FunctionType::get(f64, {f64}, false);
    模块->getOrInsertFunction(C名称, 函数类型);
    return 构建器.CreateCall(模块->getFunction(C名称), {参数}, 显示名称 + "结果");
}

llvm::Value* 调用数学函数2(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    const std::string& C名称, const std::string& 显示名称, llvm::Value* 参数1, llvm::Value* 参数2) {
    参数1 = 确保浮点(构建器, 上下文, 参数1);
    参数2 = 确保浮点(构建器, 上下文, 参数2);
    llvm::Type* f64 = llvm::Type::getDoubleTy(上下文);
    llvm::FunctionType* 函数类型 = llvm::FunctionType::get(f64, {f64, f64}, false);
    模块->getOrInsertFunction(C名称, 函数类型);
    return 构建器.CreateCall(模块->getFunction(C名称), {参数1, 参数2}, 显示名称 + "结果");
}

llvm::Value* 生成极值选择(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    llvm::Value* 左, llvm::Value* 右, bool 是最小) {
    if (左->getType()->isDoubleTy() || 右->getType()->isDoubleTy()) {
        左 = 确保浮点(构建器, 上下文, 左);
        右 = 确保浮点(构建器, 上下文, 右);
        llvm::Value* 比较 = 是最小 ? 构建器.CreateFCmpOLT(左, 右, "小于") : 构建器.CreateFCmpOGT(左, 右, "大于");
        return 构建器.CreateSelect(比较, 左, 右, 是最小 ? "最小值" : "最大值");
    }
    llvm::Value* 比较 = 是最小 ? 构建器.CreateICmpSLT(左, 右, "小于") : 构建器.CreateICmpSGT(左, 右, "大于");
    return 构建器.CreateSelect(比较, 左, 右, 是最小 ? "最小值" : "最大值");
}

llvm::Value* 短路逻辑运算(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    llvm::Value* 左值, std::function<llvm::Value*()> 生成右值, bool 是与操作) {
    // 转换为 i32
    if (左值->getType()->isIntegerTy(1)) 左值 = 构建器.CreateZExt(左值, llvm::Type::getInt32Ty(上下文));
    llvm::Value* 左条件 = 构建器.CreateICmpNE(左值, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)),
        是与操作 ? "landtmp" : "lortmp");

    llvm::Function* 当前函数 = 构建器.GetInsertBlock()->getParent();
    llvm::BasicBlock* 右块 = llvm::BasicBlock::Create(上下文, 是与操作 ? "land_rhs" : "lor_rhs", 当前函数);
    llvm::BasicBlock* 合并块 = llvm::BasicBlock::Create(上下文, 是与操作 ? "land_merge" : "lor_merge");

    // AND: 左真→右块, 左假→合并(假)
    // OR:  左真→合并(真), 左假→右块
    if (是与操作) {
        构建器.CreateCondBr(左条件, 右块, 合并块);
    } else {
        构建器.CreateCondBr(左条件, 合并块, 右块);
    }

    llvm::BasicBlock* 左块 = 构建器.GetInsertBlock();
    构建器.SetInsertPoint(右块);
    llvm::Value* 右值 = 生成右值();
    if (右值->getType()->isIntegerTy(1)) 右值 = 构建器.CreateZExt(右值, llvm::Type::getInt32Ty(上下文));
    llvm::Value* 右条件 = 构建器.CreateICmpNE(右值, llvm::ConstantInt::get(上下文, llvm::APInt(32, 0)),
        是与操作 ? "landtmp2" : "lortmp2");
    右块 = 构建器.GetInsertBlock();
    构建器.CreateBr(合并块);
    当前函数->insert(当前函数->end(), 合并块);
    构建器.SetInsertPoint(合并块);

    llvm::PHINode* phi = 构建器.CreatePHI(llvm::Type::getInt1Ty(上下文), 2, 是与操作 ? "land" : "lor");
    phi->addIncoming(llvm::ConstantInt::get(上下文, 是与操作 ? llvm::APInt(1, 0) : llvm::APInt(1, 1)), 左块);
    phi->addIncoming(右条件, 右块);
    return 构建器.CreateZExt(phi, llvm::Type::getInt32Ty(上下文), 是与操作 ? "landext" : "lorext");
}

llvm::Value* 查找结构体成员地址(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    const std::string& 对象名, const std::string& 成员名,
    符号表& 符号表实例,
    std::unordered_map<std::string, llvm::StructType*>& 结构体类型映射,
    std::unordered_map<std::string, std::vector<std::string>>& 结构体成员映射) {
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

    // 检查是否需要加载指针（结构体参数是指针类型）
    llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(对象地址);
    if (alloca && alloca->getAllocatedType()->isPointerTy()) {
        对象地址 = 构建器.CreateLoad(llvm::PointerType::get(上下文, 0), 对象地址, 对象名 + "_加载");
    }

    return 构建器.CreateStructGEP(结构体类型, 对象地址, static_cast<unsigned>(成员索引), 成员名);
}

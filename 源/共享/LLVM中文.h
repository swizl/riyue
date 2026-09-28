#如果未定义 LLVM中文_H
#定义 LLVM中文_H

#包含 "中文C++.h"
#包含 "llvm/IR/Value.h"
#包含 "llvm/IR/Type.h"
#包含 "llvm/IR/Function.h"
#包含 "llvm/IR/BasicBlock.h"
#包含 "llvm/IR/Module.h"
#包含 "llvm/IR/IRBuilder.h"
#包含 "llvm/IR/LLVMContext.h"
#包含 "llvm/IR/Constants.h"
#包含 "llvm/IR/DerivedTypes.h"
#包含 "llvm/IR/Instructions.h"
#包含 "llvm/IR/GlobalVariable.h"
#包含 "llvm/IR/Intrinsics.h"
#包含 "llvm/Support/raw_ostream.h"
#包含 "llvm/Support/CodeGen.h"
#包含 "llvm/Target/TargetMachine.h"
#包含 "llvm/TargetParser/Triple.h"
#包含 "llvm/IR/Verifier.h"

// ============================================================================
// 类型别名（注意：不使用"函数"和"构建器"，与 AST 类名和成员变量名冲突）
// ============================================================================
名域 llvm中文 {
    取用 LLVM值 = llvm::Value;
    取用 类型 = llvm::Type;
    取用 LLVM函数 = llvm::Function;
    取用 函数类型 = llvm::FunctionType;
    取用 基本块 = llvm::BasicBlock;
    取用 指针类型 = llvm::PointerType;
    取用 LLVM数组类型 = llvm::ArrayType;
    取用 LLVM结构体类型 = llvm::StructType;
取用 LLVM常量整数 = llvm::ConstantInt;
取用 LLVM常量浮点 = llvm::ConstantFP;
取用 LLVM常量 = llvm::Constant;
    取用 PHI节点 = llvm::PHINode;
    取用 分配指令 = llvm::AllocaInst;
    取用 全局变量类型 = llvm::GlobalVariable;
    取用 函数被调用者 = llvm::FunctionCallee;
    取用 着陆垫指令 = llvm::LandingPadInst;
    取用 任意精度整数 = llvm::APInt;
    取用 任意精度浮点 = llvm::APFloat;
    取用 全局值 = llvm::GlobalValue;
    取用 目标机器 = llvm::TargetMachine;
    取用 三元组 = llvm::Triple;
    取用 目标选项 = llvm::TargetOptions;
    取用 原始输出流 = llvm::raw_fd_ostream;
    取用 文件类型 = llvm::CodeGenFileType;
    取用 目标 = llvm::Target;
    取用 字符串引用 = llvm::StringRef;
}

名域 llvm中文 {
    名域 重定位 = llvm::Reloc;
}

// ============================================================================
// 类型工厂函数
// ============================================================================
名域 llvm中文 {
    内联 llvm::Type* 整型32(llvm::LLVMContext& 上下文) { 归返 llvm::Type::getInt32Ty(上下文); }
    内联 llvm::Type* 整型64(llvm::LLVMContext& 上下文) { 归返 llvm::Type::getInt64Ty(上下文); }
    内联 llvm::Type* 整型8(llvm::LLVMContext& 上下文) { 归返 llvm::Type::getInt8Ty(上下文); }
    内联 llvm::Type* 整型16(llvm::LLVMContext& 上下文) { 归返 llvm::Type::getInt16Ty(上下文); }
    内联 llvm::Type* 整型1(llvm::LLVMContext& 上下文) { 归返 llvm::Type::getInt1Ty(上下文); }
    内联 llvm::Type* 双精度浮点(llvm::LLVMContext& 上下文) { 归返 llvm::Type::getDoubleTy(上下文); }
    内联 llvm::Type* 获取空类型(llvm::LLVMContext& 上下文) { 归返 llvm::Type::getVoidTy(上下文); }
    内联 llvm::Type* 获取指针类型(llvm::LLVMContext& 上下文, 无符号 地址空间 = 0) { 归返 llvm::PointerType::get(上下文, 地址空间); }

    内联 llvm::ConstantInt* 获取整数常量(llvm::LLVMContext& 上下文, uint64_t 值, 无符号 位宽 = 32) {
        归返 llvm::ConstantInt::get(上下文, llvm::APInt(位宽, 值));
    }
    内联 llvm::ConstantInt* 获取整数常量(llvm::LLVMContext& 上下文, 恒常 llvm::APInt& 值) {
        归返 llvm::ConstantInt::get(上下文, 值);
    }
    内联 llvm::ConstantFP* 获取浮点常量(llvm::LLVMContext& 上下文, double 值) {
        归返 llvm::ConstantFP::get(上下文, llvm::APFloat(值));
    }
    内联 llvm::ConstantFP* 获取浮点常量(llvm::LLVMContext& 上下文, 恒常 llvm::APFloat& 值) {
        归返 llvm::ConstantFP::get(上下文, 值);
    }
    内联 llvm::Constant* 获取空指针值(llvm::LLVMContext& 上下文) {
        归返 llvm::ConstantPointerNull::get(llvm::PointerType::get(上下文, 0));
    }
    内联 llvm::Constant* 获取假值(llvm::LLVMContext& 上下文) {
        归返 llvm::ConstantInt::getFalse(上下文);
    }
}

// ============================================================================
// 模块方法
// ============================================================================
名域 llvm中文 {
    内联 llvm::Function* 获取模块函数(llvm::Module& 模块, 恒常 文本& 名称) {
        归返 模块.getFunction(名称);
    }
    内联 llvm::FunctionCallee 获取或插入函数(llvm::Module& 模块, 恒常 文本& 名称, llvm::FunctionType* 类型) {
        归返 模块.getOrInsertFunction(名称, 类型);
    }
}

// ============================================================================
// RTTI 辅助
// ============================================================================
名域 llvm中文 {
    范型<类型名_ T, 类型名_ U>
    内联 T* LLVM动态转换(U* 值) { 归返 llvm::dyn_cast<T>(值); }
    范型<类型名_ T, 类型名_ U>
    内联 真假型 是类型(U* 值) { 归返 llvm::isa<T>(值); }
    范型<类型名_ T, 类型名_ U>
    内联 T* 类型转换(U* 值) { 归返 llvm::cast<T>(值); }
}

// ============================================================================
// IRBuilder 方法包装
// ============================================================================
名域 llvm中文 {
    // 内存操作
    内联 llvm::Value* 创建加载(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, llvm::Value* 指针, 恒常 llvm::Twine& 名称 = "") {
        归返 构建器.CreateLoad(类型, 指针, 名称);
    }
    内联 llvm::Value* 创建存储(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Value* 指针) {
        归返 构建器.CreateStore(值, 指针);
    }
    内联 llvm::AllocaInst* 创建分配(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, llvm::Value* 数组大小 = 空针, 恒常 llvm::Twine& 名称 = "") {
        归返 构建器.CreateAlloca(类型, 数组大小, 名称);
    }
    内联 llvm::Value* 创建GEP(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, llvm::Value* 指针, llvm::ArrayRef<llvm::Value*> 索引, 恒常 llvm::Twine& 名称 = "") {
        归返 构建器.CreateGEP(类型, 指针, 索引, 名称);
    }
    内联 llvm::Value* 创建边界内GEP(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, llvm::Value* 指针, llvm::ArrayRef<llvm::Value*> 索引, 恒常 llvm::Twine& 名称 = "") {
        归返 构建器.CreateInBoundsGEP(类型, 指针, 索引, 名称);
    }
    内联 llvm::Value* 创建结构体GEP(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, llvm::Value* 指针, 无符号 索引, 恒常 llvm::Twine& 名称 = "") {
        归返 构建器.CreateStructGEP(类型, 指针, 索引, 名称);
    }

    // 整数运算
    内联 llvm::Value* 创建加(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateAdd(左, 右, 名称); }
    内联 llvm::Value* 创建减(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateSub(左, 右, 名称); }
    内联 llvm::Value* 创建乘(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateMul(左, 右, 名称); }
内联 llvm::Value* 创建有符号除法(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateSDiv(左, 右, 名称); }
内联 llvm::Value* 创建有符号取余数(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateSRem(左, 右, 名称); }
    内联 llvm::Value* 创建取负(llvm::IRBuilder<>& 构建器, llvm::Value* 值, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateNeg(值, 名称); }
    内联 llvm::Value* 创建浮点取负(llvm::IRBuilder<>& 构建器, llvm::Value* 值, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateFNeg(值, 名称); }
    内联 llvm::Value* 创建左移(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateShl(左, 右, 名称); }
    内联 llvm::Value* 创建算术右移(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateAShr(左, 右, 名称); }
    内联 llvm::Value* 创建与(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateAnd(左, 右, 名称); }
    内联 llvm::Value* 创建或(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateOr(左, 右, 名称); }
    内联 llvm::Value* 创建异或(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateXor(左, 右, 名称); }

    // 浮点运算
    内联 llvm::Value* 创建浮点加(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateFAdd(左, 右, 名称); }
    内联 llvm::Value* 创建浮点减(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateFSub(左, 右, 名称); }
    内联 llvm::Value* 创建浮点乘(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateFMul(左, 右, 名称); }
    内联 llvm::Value* 创建浮点除(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateFDiv(左, 右, 名称); }

    // 类型转换
    内联 llvm::Value* 创建零扩展(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateZExt(值, 目标类型, 名称); }
    内联 llvm::Value* 创建符号扩展(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateSExt(值, 目标类型, 名称); }
    内联 llvm::Value* 创建截断(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateTrunc(值, 目标类型, 名称); }
    内联 llvm::Value* 创建整数转浮点(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateSIToFP(值, 目标类型, 名称); }
    内联 llvm::Value* 创建浮点转整数(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateFPToSI(值, 目标类型, 名称); }
    内联 llvm::Value* 创建指针转整数(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreatePtrToInt(值, 目标类型, 名称); }
    内联 llvm::Value* 创建整数转指针(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateIntToPtr(值, 目标类型, 名称); }
    内联 llvm::Value* 创建位转换(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateBitCast(值, 目标类型, 名称); }

    // 整数比较
    内联 llvm::Value* 创建整数比较等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateICmpEQ(左, 右, 名称); }
    内联 llvm::Value* 创建整数比较不等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateICmpNE(左, 右, 名称); }
    内联 llvm::Value* 创建整数比较小于(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateICmpSLT(左, 右, 名称); }
    内联 llvm::Value* 创建整数比较大于(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateICmpSGT(左, 右, 名称); }
    内联 llvm::Value* 创建整数比较小于等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateICmpSLE(左, 右, 名称); }
    内联 llvm::Value* 创建整数比较大于等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateICmpSGE(左, 右, 名称); }
内联 llvm::Value* 创建无符号整型比较大于(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateICmpUGT(左, 右, 名称); }
内联 llvm::Value* 创建无符号整型比较小于(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateICmpULT(左, 右, 名称); }

    // 浮点比较
    内联 llvm::Value* 创建浮点比较等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateFCmpOEQ(左, 右, 名称); }
    内联 llvm::Value* 创建浮点比较不等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateFCmpONE(左, 右, 名称); }
    内联 llvm::Value* 创建浮点比较小于(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateFCmpOLT(左, 右, 名称); }
    内联 llvm::Value* 创建浮点比较大于(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateFCmpOGT(左, 右, 名称); }
    内联 llvm::Value* 创建浮点比较小于等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateFCmpOLE(左, 右, 名称); }
    内联 llvm::Value* 创建浮点比较大于等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateFCmpOGE(左, 右, 名称); }

    // 控制流
    内联 llvm::Instruction* 创建分支(llvm::IRBuilder<>& 构建器, llvm::BasicBlock* 目标) { 归返 构建器.CreateBr(目标); }
    内联 llvm::Instruction* 创建条件分支(llvm::IRBuilder<>& 构建器, llvm::Value* 条件, llvm::BasicBlock* 真分支, llvm::BasicBlock* 假分支) { 归返 构建器.CreateCondBr(条件, 真分支, 假分支); }
    内联 llvm::SwitchInst* 创建开关指令(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::BasicBlock* 默认分支, 无符号 分支数 = 10) { 归返 构建器.CreateSwitch(值, 默认分支, 分支数); }
    内联 llvm::ReturnInst* 创建返回指令(llvm::IRBuilder<>& 构建器, llvm::Value* 值 = 空针) { 归返 构建器.CreateRet(值); }
    内联 llvm::ReturnInst* 创建空返回(llvm::IRBuilder<>& 构建器) { 归返 构建器.CreateRetVoid(); }
    内联 llvm::UnreachableInst* 创建不可达(llvm::IRBuilder<>& 构建器) { 归返 构建器.CreateUnreachable(); }

    // 调用和全局字符串
    内联 llvm::CallInst* 创建调用(llvm::IRBuilder<>& 构建器, llvm::Function* 函数, llvm::ArrayRef<llvm::Value*> 参数, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateCall(函数, 参数, 名称); }
    内联 llvm::CallInst* 创建调用(llvm::IRBuilder<>& 构建器, llvm::FunctionCallee 被调用者, llvm::ArrayRef<llvm::Value*> 参数, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateCall(被调用者, 参数, 名称); }
    内联 llvm::InvokeInst* 创建调用带异常(llvm::IRBuilder<>& 构建器, llvm::Function* 函数, llvm::ArrayRef<llvm::Value*> 参数, llvm::BasicBlock* 正常继续, llvm::BasicBlock* 异常分支, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateInvoke(函数, 正常继续, 异常分支, 参数, 名称); }
    内联 llvm::Value* 创建全局字符串(llvm::IRBuilder<>& 构建器, llvm::StringRef 字符串) { 归返 构建器.CreateGlobalString(字符串); }

    // PHI 和选择
    内联 llvm::PHINode* 创建PHI(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, 无符号 流入数量, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreatePHI(类型, 流入数量, 名称); }
    内联 llvm::Value* 创建选择(llvm::IRBuilder<>& 构建器, llvm::Value* 条件, llvm::Value* 真值_, llvm::Value* 假值_, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateSelect(条件, 真值_, 假值_, 名称); }

    // 结构体操作
内联 llvm::Value* 创建提取值(llvm::IRBuilder<>& 构建器, llvm::Value* 结构体值, llvm::ArrayRef<无符号> 索引, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateExtractValue(结构体值, 索引, 名称); }
内联 llvm::Value* 创建插入值(llvm::IRBuilder<>& 构建器, llvm::Value* 结构体值, llvm::Value* 值, llvm::ArrayRef<无符号> 索引, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateInsertValue(结构体值, 值, 索引, 名称); }

    // 异常处理
    内联 llvm::LandingPadInst* 创建着陆垫(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, 无符号 子句数量, 恒常 llvm::Twine& 名称 = "") { 归返 构建器.CreateLandingPad(类型, 子句数量, 名称); }

    // Builder 状态
    内联 虚空型 设置插入点(llvm::IRBuilder<>& 构建器, llvm::BasicBlock* 块) { 构建器.SetInsertPoint(块); }
    内联 llvm::BasicBlock* 获取插入块(llvm::IRBuilder<>& 构建器) { 归返 构建器.GetInsertBlock(); }
}

// ============================================================================
// 类型查询方法包装
// ============================================================================
名域 llvm中文 {
    内联 真假型 是指针类型(恒常 llvm::Type* 类型) { 归返 类型->isPointerTy(); }
    内联 真假型 是整型(恒常 llvm::Type* 类型) { 归返 类型->isIntegerTy(); }
    内联 真假型 是双精度类型(恒常 llvm::Type* 类型) { 归返 类型->isDoubleTy(); }
    内联 真假型 是获取空类型(恒常 llvm::Type* 类型) { 归返 类型->isVoidTy(); }
    内联 真假型 是结构体类型(恒常 llvm::Type* 类型) { 归返 类型->isStructTy(); }
    内联 真假型 是数组类型(恒常 llvm::Type* 类型) { 归返 类型->isArrayTy(); }
    内联 真假型 是浮点类型(恒常 llvm::Type* 类型) { 归返 类型->isFloatTy(); }
}

// ============================================================================
// 常量表达式
// ============================================================================
名域 llvm中文 {
    内联 llvm::Constant* 获取位转换常量(llvm::Constant* 值, llvm::Type* 类型) {
        归返 llvm::ConstantExpr::getBitCast(值, 类型);
    }
    内联 llvm::Constant* 获取空值(llvm::Type* 类型) {
        归返 llvm::Constant::getNullValue(类型);
    }
}

// ============================================================================
// 类型工厂函数（纯中文）
// ============================================================================
名域 llvm中文 {
    内联 llvm::BasicBlock* 创建基本块(llvm::LLVMContext& 上下文, 恒常 llvm::Twine& 名称 = "", llvm::Function* 父函数 = 空针) {
        归返 llvm::BasicBlock::Create(上下文, 名称, 父函数);
    }
    内联 llvm::ArrayType* 获取数组类型(llvm::Type* 元素类型, uint64_t 元素数量) {
        归返 llvm::ArrayType::get(元素类型, 元素数量);
    }
    内联 llvm::StructType* 获取结构体类型(llvm::LLVMContext& 上下文, llvm::ArrayRef<llvm::Type*> 元素类型, 真假型 是否紧凑 = 假值) {
        归返 llvm::StructType::get(上下文, 元素类型, 是否紧凑);
    }
    内联 llvm::StructType* 创建结构体类型(llvm::LLVMContext& 上下文, llvm::ArrayRef<llvm::Type*> 元素类型, 恒常 llvm::Twine& 名称, 真假型 是否紧凑 = 假值) {
        归返 llvm::StructType::create(上下文, 元素类型, 名称.str(), 是否紧凑);
    }
    内联 llvm::StructType* 创建结构体类型(llvm::LLVMContext& 上下文, 恒常 llvm::Twine& 名称) {
        归返 llvm::StructType::create(上下文, 名称.str());
    }
    内联 llvm::FunctionType* 获取函数类型(llvm::Type* 返回类型, llvm::ArrayRef<llvm::Type*> 参数类型, 真假型 是否变长 = 假值) {
        归返 llvm::FunctionType::get(返回类型, 参数类型, 是否变长);
    }
    内联 llvm::Function* 创建函数(llvm::FunctionType* 类型, llvm::GlobalValue::LinkageTypes 链接类型, 恒常 llvm::Twine& 名称, llvm::Module& 模块) {
        归返 llvm::Function::Create(类型, 链接类型, 名称, 模块);
    }
    内联 llvm::GlobalVariable* 创建全局变量(llvm::Module& 模块, llvm::Type* 类型, 真假型 是否常量, llvm::GlobalValue::LinkageTypes 链接类型, llvm::Constant* 初始值, 恒常 llvm::Twine& 名称 = "") {
        归返 new llvm::GlobalVariable(模块, 类型, 是否常量, 链接类型, 初始值, 名称);
    }
    内联 llvm::GlobalVariable* 创建全局变量(llvm::Module& 模块, llvm::Type* 类型, 真假型 是否常量, llvm::GlobalValue::LinkageTypes 链接类型, 恒常 llvm::Twine& 名称 = "") {
        归返 new llvm::GlobalVariable(模块, 类型, 是否常量, 链接类型, 空针, 名称);
    }
    内联 llvm::UndefValue* 获取未定义值(llvm::Type* 类型) {
        归返 llvm::UndefValue::get(类型);
    }
    内联 llvm::Constant* 获取常量聚合零(llvm::Type* 类型) {
        归返 llvm::ConstantAggregateZero::get(类型);
    }
    内联 llvm::Constant* 获取常量字符串(llvm::LLVMContext& 上下文, llvm::StringRef 字符串, 真假型 空终止 = 真值) {
        归返 llvm::ConstantDataArray::getString(上下文, 字符串, 空终止);
    }
    内联 llvm::Constant* 获取常量数组(llvm::ArrayType* 类型, llvm::ArrayRef<llvm::Constant*> 值) {
        归返 llvm::ConstantArray::get(类型, 值);
    }
    内联 llvm::Constant* 获取常量令牌空(llvm::LLVMContext& 上下文) {
        归返 llvm::ConstantTokenNone::get(上下文);
    }
    内联 llvm::Function* 获取内建函数声明(llvm::Module& 模块, llvm::Intrinsic::ID 内建ID, llvm::ArrayRef<llvm::Type*> 类型参数 = {}) {
        归返 llvm::Intrinsic::getOrInsertDeclaration(&模块, 内建ID, 类型参数);
    }
    内联 真假型 验证函数(llvm::Function& 函数, llvm::raw_ostream* 输出 = 空针) {
        归返 llvm::verifyFunction(函数, 输出);
    }
}

#结束
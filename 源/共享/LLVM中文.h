#ifndef LLVM中文_H
#define LLVM中文_H

#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/MC/TargetRegistry.h"

// ============================================================================
// 类型别名（注意：不使用"函数"和"构建器"，与 AST 类名和成员变量名冲突）
// ============================================================================
namespace llvm中文 {
    using LLVM值 = llvm::Value;
    using 类型 = llvm::Type;
    using LLVM函数 = llvm::Function;
    using 函数类型 = llvm::FunctionType;
    using 基本块 = llvm::BasicBlock;
    using 指针类型 = llvm::PointerType;
    using LLVM数组类型 = llvm::ArrayType;
    using LLVM结构体类型 = llvm::StructType;
using LLVM常量整数 = llvm::ConstantInt;
using LLVM常量浮点 = llvm::ConstantFP;
using LLVM常量 = llvm::Constant;
    using PHI节点 = llvm::PHINode;
    using 分配指令 = llvm::AllocaInst;
    using 全局变量类型 = llvm::GlobalVariable;
    using 函数被调用者 = llvm::FunctionCallee;
    using 着陆垫指令 = llvm::LandingPadInst;
    using 任意精度整数 = llvm::APInt;
    using 任意精度浮点 = llvm::APFloat;
    using 全局值 = llvm::GlobalValue;
    using 目标机器 = llvm::TargetMachine;
    using 三元组 = llvm::Triple;
    using 目标选项 = llvm::TargetOptions;
    using 原始输出流 = llvm::raw_fd_ostream;
    using 文件类型 = llvm::CodeGenFileType;
    using 目标 = llvm::Target;
    using 字符串引用 = llvm::StringRef;
}

namespace llvm中文 {
    namespace 重定位 = llvm::Reloc;
}

// ============================================================================
// 类型工厂函数
// ============================================================================
namespace llvm中文 {
    inline llvm::Type* 整型32(llvm::LLVMContext& 上下文) { return llvm::Type::getInt32Ty(上下文); }
    inline llvm::Type* 整型64(llvm::LLVMContext& 上下文) { return llvm::Type::getInt64Ty(上下文); }
    inline llvm::Type* 整型8(llvm::LLVMContext& 上下文) { return llvm::Type::getInt8Ty(上下文); }
    inline llvm::Type* 整型16(llvm::LLVMContext& 上下文) { return llvm::Type::getInt16Ty(上下文); }
    inline llvm::Type* 整型1(llvm::LLVMContext& 上下文) { return llvm::Type::getInt1Ty(上下文); }
    inline llvm::Type* 双精度浮点(llvm::LLVMContext& 上下文) { return llvm::Type::getDoubleTy(上下文); }
    inline llvm::Type* 获取空类型(llvm::LLVMContext& 上下文) { return llvm::Type::getVoidTy(上下文); }
    inline llvm::Type* 获取指针类型(llvm::LLVMContext& 上下文, unsigned 地址空间 = 0) { return llvm::PointerType::get(上下文, 地址空间); }

    inline llvm::ConstantInt* 获取整数常量(llvm::LLVMContext& 上下文, uint64_t 值, unsigned 位宽 = 32) {
        return llvm::ConstantInt::get(上下文, llvm::APInt(位宽, 值));
    }
    inline llvm::ConstantInt* 获取整数常量(llvm::LLVMContext& 上下文, const llvm::APInt& 值) {
        return llvm::ConstantInt::get(上下文, 值);
    }
    inline llvm::ConstantFP* 获取浮点常量(llvm::LLVMContext& 上下文, double 值) {
        return llvm::ConstantFP::get(上下文, llvm::APFloat(值));
    }
    inline llvm::ConstantFP* 获取浮点常量(llvm::LLVMContext& 上下文, const llvm::APFloat& 值) {
        return llvm::ConstantFP::get(上下文, 值);
    }
    inline llvm::Constant* 获取空指针值(llvm::LLVMContext& 上下文) {
        return llvm::ConstantPointerNull::get(llvm::PointerType::get(上下文, 0));
    }
    inline llvm::Constant* 获取假值(llvm::LLVMContext& 上下文) {
        return llvm::ConstantInt::getFalse(上下文);
    }
}

// ============================================================================
// 模块方法
// ============================================================================
namespace llvm中文 {
    inline llvm::Function* 获取模块函数(llvm::Module& 模块, const std::string& 名称) {
        return 模块.getFunction(名称);
    }
    inline llvm::FunctionCallee 获取或插入函数(llvm::Module& 模块, const std::string& 名称, llvm::FunctionType* 类型) {
        return 模块.getOrInsertFunction(名称, 类型);
    }
}

// ============================================================================
// RTTI 辅助
// ============================================================================
namespace llvm中文 {
    template<typename T, typename U>
    inline T* LLVM动态转换(U* 值) { return llvm::dyn_cast<T>(值); }
    template<typename T, typename U>
    inline bool 是类型(U* 值) { return llvm::isa<T>(值); }
    template<typename T, typename U>
    inline T* 类型转换(U* 值) { return llvm::cast<T>(值); }
}

// ============================================================================
// IRBuilder 方法包装
// ============================================================================
namespace llvm中文 {
    // 内存操作
    inline llvm::Value* 创建加载(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, llvm::Value* 指针, const llvm::Twine& 名称 = "") {
        return 构建器.CreateLoad(类型, 指针, 名称);
    }
    inline llvm::Value* 创建存储(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Value* 指针) {
        return 构建器.CreateStore(值, 指针);
    }
    inline llvm::AllocaInst* 创建分配(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, llvm::Value* 数组大小 = nullptr, const llvm::Twine& 名称 = "") {
        return 构建器.CreateAlloca(类型, 数组大小, 名称);
    }
    inline llvm::Value* 创建GEP(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, llvm::Value* 指针, llvm::ArrayRef<llvm::Value*> 索引, const llvm::Twine& 名称 = "") {
        return 构建器.CreateGEP(类型, 指针, 索引, 名称);
    }
    inline llvm::Value* 创建边界内GEP(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, llvm::Value* 指针, llvm::ArrayRef<llvm::Value*> 索引, const llvm::Twine& 名称 = "") {
        return 构建器.CreateInBoundsGEP(类型, 指针, 索引, 名称);
    }
    inline llvm::Value* 创建结构体GEP(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, llvm::Value* 指针, unsigned 索引, const llvm::Twine& 名称 = "") {
        return 构建器.CreateStructGEP(类型, 指针, 索引, 名称);
    }

    // 整数运算
    inline llvm::Value* 创建加(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateAdd(左, 右, 名称); }
    inline llvm::Value* 创建减(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateSub(左, 右, 名称); }
    inline llvm::Value* 创建乘(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateMul(左, 右, 名称); }
inline llvm::Value* 创建有符号除法(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateSDiv(左, 右, 名称); }
inline llvm::Value* 创建有符号取余数(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateSRem(左, 右, 名称); }
    inline llvm::Value* 创建取负(llvm::IRBuilder<>& 构建器, llvm::Value* 值, const llvm::Twine& 名称 = "") { return 构建器.CreateNeg(值, 名称); }
    inline llvm::Value* 创建浮点取负(llvm::IRBuilder<>& 构建器, llvm::Value* 值, const llvm::Twine& 名称 = "") { return 构建器.CreateFNeg(值, 名称); }
    inline llvm::Value* 创建左移(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateShl(左, 右, 名称); }
    inline llvm::Value* 创建算术右移(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateAShr(左, 右, 名称); }
    inline llvm::Value* 创建与(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateAnd(左, 右, 名称); }
    inline llvm::Value* 创建或(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateOr(左, 右, 名称); }
    inline llvm::Value* 创建异或(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateXor(左, 右, 名称); }

    // 浮点运算
    inline llvm::Value* 创建浮点加(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateFAdd(左, 右, 名称); }
    inline llvm::Value* 创建浮点减(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateFSub(左, 右, 名称); }
    inline llvm::Value* 创建浮点乘(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateFMul(左, 右, 名称); }
    inline llvm::Value* 创建浮点除(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateFDiv(左, 右, 名称); }

    // 类型转换
    inline llvm::Value* 创建零扩展(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, const llvm::Twine& 名称 = "") { return 构建器.CreateZExt(值, 目标类型, 名称); }
    inline llvm::Value* 创建符号扩展(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, const llvm::Twine& 名称 = "") { return 构建器.CreateSExt(值, 目标类型, 名称); }
    inline llvm::Value* 创建截断(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, const llvm::Twine& 名称 = "") { return 构建器.CreateTrunc(值, 目标类型, 名称); }
    inline llvm::Value* 创建整数转浮点(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, const llvm::Twine& 名称 = "") { return 构建器.CreateSIToFP(值, 目标类型, 名称); }
    inline llvm::Value* 创建浮点转整数(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, const llvm::Twine& 名称 = "") { return 构建器.CreateFPToSI(值, 目标类型, 名称); }
    inline llvm::Value* 创建指针转整数(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, const llvm::Twine& 名称 = "") { return 构建器.CreatePtrToInt(值, 目标类型, 名称); }
    inline llvm::Value* 创建整数转指针(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, const llvm::Twine& 名称 = "") { return 构建器.CreateIntToPtr(值, 目标类型, 名称); }
    inline llvm::Value* 创建位转换(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::Type* 目标类型, const llvm::Twine& 名称 = "") { return 构建器.CreateBitCast(值, 目标类型, 名称); }

    // 整数比较
    inline llvm::Value* 创建整数比较等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateICmpEQ(左, 右, 名称); }
    inline llvm::Value* 创建整数比较不等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateICmpNE(左, 右, 名称); }
    inline llvm::Value* 创建整数比较小于(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateICmpSLT(左, 右, 名称); }
    inline llvm::Value* 创建整数比较大于(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateICmpSGT(左, 右, 名称); }
    inline llvm::Value* 创建整数比较小于等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateICmpSLE(左, 右, 名称); }
    inline llvm::Value* 创建整数比较大于等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateICmpSGE(左, 右, 名称); }
inline llvm::Value* 创建无符号整型比较大于(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateICmpUGT(左, 右, 名称); }
inline llvm::Value* 创建无符号整型比较小于(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateICmpULT(左, 右, 名称); }

    // 浮点比较
    inline llvm::Value* 创建浮点比较等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateFCmpOEQ(左, 右, 名称); }
    inline llvm::Value* 创建浮点比较不等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateFCmpONE(左, 右, 名称); }
    inline llvm::Value* 创建浮点比较小于(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateFCmpOLT(左, 右, 名称); }
    inline llvm::Value* 创建浮点比较大于(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateFCmpOGT(左, 右, 名称); }
    inline llvm::Value* 创建浮点比较小于等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateFCmpOLE(左, 右, 名称); }
    inline llvm::Value* 创建浮点比较大于等(llvm::IRBuilder<>& 构建器, llvm::Value* 左, llvm::Value* 右, const llvm::Twine& 名称 = "") { return 构建器.CreateFCmpOGE(左, 右, 名称); }

    // 控制流
    inline llvm::Instruction* 创建分支(llvm::IRBuilder<>& 构建器, llvm::BasicBlock* 目标) { return 构建器.CreateBr(目标); }
    inline llvm::Instruction* 创建条件分支(llvm::IRBuilder<>& 构建器, llvm::Value* 条件, llvm::BasicBlock* 真分支, llvm::BasicBlock* 假分支) { return 构建器.CreateCondBr(条件, 真分支, 假分支); }
    inline llvm::SwitchInst* 创建开关指令(llvm::IRBuilder<>& 构建器, llvm::Value* 值, llvm::BasicBlock* 默认分支, unsigned 分支数 = 10) { return 构建器.CreateSwitch(值, 默认分支, 分支数); }
    inline llvm::ReturnInst* 创建返回指令(llvm::IRBuilder<>& 构建器, llvm::Value* 值 = nullptr) { return 构建器.CreateRet(值); }
    inline llvm::ReturnInst* 创建空返回(llvm::IRBuilder<>& 构建器) { return 构建器.CreateRetVoid(); }
    inline llvm::UnreachableInst* 创建不可达(llvm::IRBuilder<>& 构建器) { return 构建器.CreateUnreachable(); }

    // 调用和全局字符串
    inline llvm::CallInst* 创建调用(llvm::IRBuilder<>& 构建器, llvm::Function* 函数, llvm::ArrayRef<llvm::Value*> 参数, const llvm::Twine& 名称 = "") { return 构建器.CreateCall(函数, 参数, 名称); }
    inline llvm::CallInst* 创建调用(llvm::IRBuilder<>& 构建器, llvm::FunctionCallee 被调用者, llvm::ArrayRef<llvm::Value*> 参数, const llvm::Twine& 名称 = "") { return 构建器.CreateCall(被调用者, 参数, 名称); }
    inline llvm::InvokeInst* 创建调用带异常(llvm::IRBuilder<>& 构建器, llvm::Function* 函数, llvm::ArrayRef<llvm::Value*> 参数, llvm::BasicBlock* 正常继续, llvm::BasicBlock* 异常分支, const llvm::Twine& 名称 = "") { return 构建器.CreateInvoke(函数, 正常继续, 异常分支, 参数, 名称); }
    inline llvm::Value* 创建全局字符串(llvm::IRBuilder<>& 构建器, llvm::StringRef 字符串) { return 构建器.CreateGlobalString(字符串); }

    // PHI 和选择
    inline llvm::PHINode* 创建PHI(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, unsigned 流入数量, const llvm::Twine& 名称 = "") { return 构建器.CreatePHI(类型, 流入数量, 名称); }
    inline llvm::Value* 创建选择(llvm::IRBuilder<>& 构建器, llvm::Value* 条件, llvm::Value* 真值, llvm::Value* 假值, const llvm::Twine& 名称 = "") { return 构建器.CreateSelect(条件, 真值, 假值, 名称); }

    // 结构体操作
inline llvm::Value* 创建提取值(llvm::IRBuilder<>& 构建器, llvm::Value* 结构体值, llvm::ArrayRef<unsigned> 索引, const llvm::Twine& 名称 = "") { return 构建器.CreateExtractValue(结构体值, 索引, 名称); }
inline llvm::Value* 创建插入值(llvm::IRBuilder<>& 构建器, llvm::Value* 结构体值, llvm::Value* 值, llvm::ArrayRef<unsigned> 索引, const llvm::Twine& 名称 = "") { return 构建器.CreateInsertValue(结构体值, 值, 索引, 名称); }

    // 异常处理
    inline llvm::LandingPadInst* 创建着陆垫(llvm::IRBuilder<>& 构建器, llvm::Type* 类型, unsigned 子句数量, const llvm::Twine& 名称 = "") { return 构建器.CreateLandingPad(类型, 子句数量, 名称); }

    // Builder 状态
    inline void 设置插入点(llvm::IRBuilder<>& 构建器, llvm::BasicBlock* 块) { 构建器.SetInsertPoint(块); }
    inline llvm::BasicBlock* 获取插入块(llvm::IRBuilder<>& 构建器) { return 构建器.GetInsertBlock(); }
}

// ============================================================================
// 类型查询方法包装
// ============================================================================
namespace llvm中文 {
    inline bool 是指针类型(const llvm::Type* 类型) { return 类型->isPointerTy(); }
    inline bool 是整型(const llvm::Type* 类型) { return 类型->isIntegerTy(); }
    inline bool 是双精度类型(const llvm::Type* 类型) { return 类型->isDoubleTy(); }
    inline bool 是获取空类型(const llvm::Type* 类型) { return 类型->isVoidTy(); }
    inline bool 是结构体类型(const llvm::Type* 类型) { return 类型->isStructTy(); }
    inline bool 是数组类型(const llvm::Type* 类型) { return 类型->isArrayTy(); }
    inline bool 是浮点类型(const llvm::Type* 类型) { return 类型->isFloatTy(); }
}

// ============================================================================
// 常量表达式
// ============================================================================
namespace llvm中文 {
    inline llvm::Constant* 获取位转换常量(llvm::Constant* 值, llvm::Type* 类型) {
        return llvm::ConstantExpr::getBitCast(值, 类型);
    }
    inline llvm::Constant* 获取空值(llvm::Type* 类型) {
        return llvm::Constant::getNullValue(类型);
    }
}

#endif
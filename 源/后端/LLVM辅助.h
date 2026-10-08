#如果未定义 LLVM辅助_H
#定义 LLVM辅助_H

#包含 "llvm/IR/IRBuilder.h"
#包含 "llvm/IR/Module.h"
#包含 "llvm/IR/LLVMContext.h"
#包含 <string>
#包含 "../共享/LLVM中文.h"

取用 名域 llvm中文;

LLVM值* 确保浮点(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, LLVM值* 值);

LLVM值* 拼接字符串(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    LLVM值* 左, LLVM值* 右);

LLVM值* 读取行并剥离换行(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    LLVM值* 文件指针, 整数型 缓冲大小 = 4096);

LLVM值* 读取标准输入行(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    整数型 缓冲大小 = 4096);

LLVM值* 调用外部函数0(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    恒常 文本& 名称, 类型* 返回类型);

LLVM值* 调用数学函数1(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    恒常 文本& C名称, 恒常 文本& 显示名称, LLVM值* 参数);

LLVM值* 调用数学函数2(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    恒常 文本& C名称, 恒常 文本& 显示名称, LLVM值* 参数1, LLVM值* 参数2);

LLVM值* 生成极值选择(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    LLVM值* 左, LLVM值* 右, 真假型 是最小);

// 短路逻辑运算（逻辑与/逻辑或）
// 是与操作: 真值 = AND, 假值 = OR
LLVM值* 短路逻辑运算(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    LLVM值* 左值, 函数对象<LLVM值*()> 生成右值, 真假型 是与操作);

// 查找结构体成员地址
LLVM值* 查找结构体成员地址(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    恒常 文本& 对象名, 恒常 文本& 成员名,
    类别 符号表& 符号表实例,
    哈希映射<文本, LLVM结构体类型*>& 结构体类型映射,
    哈希映射<文本, 数组向量<文本>>& 结构体成员映射);

范型<类型名_ 返回值列表类型>
类型* 计算返回类型(llvm::LLVMContext& 上下文, 恒常 返回值列表类型& 返回值列表,
    函数对象<类型*(恒常 文本&)> 类型转换) {
    类型* 返回LLVM类型 = 获取空类型(上下文);
    如果 (返回值列表.size() == 1 && !返回值列表[0].类型.empty() && 返回值列表[0].类型 != "空") {
        返回LLVM类型 = 类型转换(返回值列表[0].类型);
    } 否则 如果 (返回值列表.size() > 1) {
        数组向量<类型*> 返回类型列表;
        循环 (恒常 自动& 返回值 : 返回值列表) {
            如果 (!返回值.类型.empty() && 返回值.类型 != "空") {
                返回类型列表.push_back(类型转换(返回值.类型));
            }
        }
        如果 (!返回类型列表.empty()) {
            返回LLVM类型 = llvm::StructType::get(上下文, 返回类型列表);
        }
    }
    归返 返回LLVM类型;
}

// 整数位宽：非整数类型返回 0
内联 整数型 整数位宽(llvm::Type* 类型) {
    如果 (类型->isIntegerTy()) 归返 类型->getIntegerBitWidth();
    归返 0;
}

// 按目标类型转换值：整数族 符号扩展/截断；浮点族 扩展/截断；整数↔浮点 sitofp/fptosi
内联 LLVM值* 转换值(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, LLVM值* 值, 类型* 目标类型) {
    类型* 源类型 = 值->getType();
    如果 (源类型 == 目标类型) 归返 值;
    如果 (源类型->isIntegerTy() && 目标类型->isIntegerTy()) {
        如果 (整数位宽(源类型) < 整数位宽(目标类型)) {
            归返 创建符号扩展(构建器, 值, 目标类型);
        }
        归返 创建截断(构建器, 值, 目标类型);
    }
    如果 (源类型->isFloatingPointTy() && 目标类型->isFloatingPointTy()) {
        如果 (源类型->getPrimitiveSizeInBits() < 目标类型->getPrimitiveSizeInBits()) {
            归返 创建扩展浮点(构建器, 值, 目标类型);
        }
        归返 创建截断浮点(构建器, 值, 目标类型);
    }
    如果 (源类型->isIntegerTy() && 目标类型->isFloatingPointTy()) {
        归返 创建整数转浮点(构建器, 值, 目标类型);
    }
    如果 (源类型->isFloatingPointTy() && 目标类型->isIntegerTy()) {
        归返 创建浮点转整数(构建器, 值, 目标类型);
    }
    归返 值;
}

#结束
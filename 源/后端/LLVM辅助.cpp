#include "LLVM辅助.h"
#include "符号表.h"
#include "../共享/LLVM中文.h"

using namespace llvm中文;

LLVM值* 确保浮点(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, LLVM值* 值) {
    if (值->getType()->isIntegerTy()) {
        return 创建整数转浮点(构建器, 值, 双精度浮点(上下文));
    }
    return 值;
}

LLVM值* 拼接字符串(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    LLVM值* 左, LLVM值* 右) {
    类型* i8Ptr = 获取指针类型(上下文);
    函数类型* sprintf类型 = llvm::FunctionType::get(
        整型32(上下文), {i8Ptr, i8Ptr}, true);
    函数类型* malloc类型 = llvm::FunctionType::get(i8Ptr, {整型64(上下文)}, false);
    函数类型* strlen类型 = llvm::FunctionType::get(整型64(上下文), {i8Ptr}, false);
    获取或插入函数(*模块, "sprintf", sprintf类型);
    获取或插入函数(*模块, "malloc", malloc类型);
    获取或插入函数(*模块, "strlen", strlen类型);

    LLVM值* 左长度 = 创建调用(构建器, 获取模块函数(*模块, "strlen"), {左}, "左长度");
    LLVM值* 右长度 = 创建调用(构建器, 获取模块函数(*模块, "strlen"), {右}, "右长度");
    LLVM值* 总长度 = 创建加(构建器, 左长度, 右长度, "总长度");
    LLVM值* 缓冲区大小 = 创建加(构建器, 总长度, 获取整数常量(上下文, 1, 64), "缓冲区大小");
    LLVM值* 缓冲区 = 创建调用(构建器, 获取模块函数(*模块, "malloc"), {缓冲区大小}, "拼接缓冲区");
    创建调用(构建器, 获取模块函数(*模块, "sprintf"), {缓冲区, 创建全局字符串(构建器, "%s%s"), 左, 右});
    return 缓冲区;
}

static LLVM值* 剥离末尾换行(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    LLVM值* 缓冲区) {
    函数类型* strlen类型 = llvm::FunctionType::get(
        整型64(上下文), {获取指针类型(上下文)}, false);
    获取或插入函数(*模块, "strlen", strlen类型);
    LLVM值* 长度 = 创建调用(构建器, 获取模块函数(*模块, "strlen"), {缓冲区}, "缓冲区长度");
    LLVM值* 长度大于零 = 创建无符号整型比较大于(构建器, 长度, 获取整数常量(上下文, 0, 64));
    LLVM值* 最后位置 = 创建减(构建器, 长度, 获取整数常量(上下文, 1, 64));
    LLVM值* 安全位置 = 创建选择(构建器, 长度大于零, 最后位置, 获取整数常量(上下文, 0, 64));
    LLVM值* 最后字符地址 = 创建边界内GEP(构建器, 整型8(上下文), 缓冲区, 安全位置);
    LLVM值* 最后字符 = 创建加载(构建器, 整型8(上下文), 最后字符地址);
    LLVM值* 是换行 = 创建整数比较等(构建器, 最后字符, 获取整数常量(上下文, '\n', 8));
    LLVM值* 应替换 = 创建与(构建器, 长度大于零, 是换行);
    LLVM值* 空终止 = 获取整数常量(上下文, 0, 8);
    LLVM值* 原始字节 = 创建加载(构建器, 整型8(上下文), 最后字符地址, "原始字节");
    LLVM值* 最终字节 = 创建选择(构建器, 应替换, 空终止, 原始字节, "最终字节");
    创建存储(构建器, 最终字节, 最后字符地址);
    return 缓冲区;
}

LLVM值* 读取行并剥离换行(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    LLVM值* 文件指针, int 缓冲大小) {
    函数类型* fgets类型 = llvm::FunctionType::get(
        获取指针类型(上下文),
        {获取指针类型(上下文), 整型32(上下文), 获取指针类型(上下文)}, false);
    函数类型* malloc类型 = llvm::FunctionType::get(
        获取指针类型(上下文), {整型64(上下文)}, false);
    获取或插入函数(*模块, "fgets", fgets类型);
    获取或插入函数(*模块, "malloc", malloc类型);

    LLVM值* 缓冲区 = 创建调用(构建器, 获取模块函数(*模块, "malloc"),
        {获取整数常量(上下文, 缓冲大小, 64)}, "读取缓冲区");
    创建调用(构建器, 获取模块函数(*模块, "fgets"), {缓冲区,
        获取整数常量(上下文, 缓冲大小, 32), 文件指针});
    return 剥离末尾换行(构建器, 上下文, 模块, 缓冲区);
}

LLVM值* 读取标准输入行(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    int 缓冲大小) {
    LLVM值* stdinPtr = 模块->getNamedGlobal("stdin");
    if (!stdinPtr) {
        auto* stdinGV = new 全局变量类型(*模块, 获取指针类型(上下文),
            false, llvm::GlobalValue::ExternalLinkage, nullptr, "stdin");
        stdinGV->setDLLStorageClass(llvm::GlobalValue::DLLImportStorageClass);
        stdinPtr = stdinGV;
    }
    stdinPtr = 创建加载(构建器, 获取指针类型(上下文), stdinPtr, "stdin_val");
    return 读取行并剥离换行(构建器, 上下文, 模块, stdinPtr, 缓冲大小);
}

LLVM值* 调用外部函数0(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    const std::string& 名称, 类型* 返回类型) {
    函数类型* 函数类型 = llvm::FunctionType::get(返回类型, {}, false);
    获取或插入函数(*模块, 名称, 函数类型);
    return 创建调用(构建器, 获取模块函数(*模块, 名称), {});
}

LLVM值* 调用数学函数1(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    const std::string& C名称, const std::string& 显示名称, LLVM值* 参数) {
    参数 = 确保浮点(构建器, 上下文, 参数);
    类型* f64 = 双精度浮点(上下文);
    函数类型* 函数类型 = llvm::FunctionType::get(f64, {f64}, false);
    获取或插入函数(*模块, C名称, 函数类型);
    return 创建调用(构建器, 获取模块函数(*模块, C名称), {参数}, 显示名称 + "结果");
}

LLVM值* 调用数学函数2(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文, llvm::Module* 模块,
    const std::string& C名称, const std::string& 显示名称, LLVM值* 参数1, LLVM值* 参数2) {
    参数1 = 确保浮点(构建器, 上下文, 参数1);
    参数2 = 确保浮点(构建器, 上下文, 参数2);
    类型* f64 = 双精度浮点(上下文);
    函数类型* 函数类型 = llvm::FunctionType::get(f64, {f64, f64}, false);
    获取或插入函数(*模块, C名称, 函数类型);
    return 创建调用(构建器, 获取模块函数(*模块, C名称), {参数1, 参数2}, 显示名称 + "结果");
}

LLVM值* 生成极值选择(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    LLVM值* 左, LLVM值* 右, bool 是最小) {
    if (左->getType()->isDoubleTy() || 右->getType()->isDoubleTy()) {
        左 = 确保浮点(构建器, 上下文, 左);
        右 = 确保浮点(构建器, 上下文, 右);
        LLVM值* 比较 = 是最小 ? 创建浮点比较小于(构建器, 左, 右, "小于") : 创建浮点比较大于(构建器, 左, 右, "大于");
        return 创建选择(构建器, 比较, 左, 右, 是最小 ? "最小值" : "最大值");
    }
    LLVM值* 比较 = 是最小 ? 创建整数比较小于(构建器, 左, 右, "小于") : 创建整数比较大于(构建器, 左, 右, "大于");
    return 创建选择(构建器, 比较, 左, 右, 是最小 ? "最小值" : "最大值");
}

LLVM值* 短路逻辑运算(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    LLVM值* 左值, std::function<LLVM值*()> 生成右值, bool 是与操作) {
    // 转换为 i32
    if (左值->getType()->isIntegerTy(1)) 左值 = 创建零扩展(构建器, 左值, 整型32(上下文));
    LLVM值* 左条件 = 创建整数比较不等(构建器, 左值, 获取整数常量(上下文, 0, 32),
        是与操作 ? "landtmp" : "lortmp");

    LLVM函数* 当前函数 = 获取插入块(构建器)->getParent();
    基本块* 右块 = llvm::BasicBlock::Create(上下文, 是与操作 ? "land_rhs" : "lor_rhs", 当前函数);
    基本块* 合并块 = llvm::BasicBlock::Create(上下文, 是与操作 ? "land_merge" : "lor_merge");

    // AND: 左真→右块, 左假→合并(假)
    // OR:  左真→合并(真), 左假→右块
    if (是与操作) {
        创建条件分支(构建器, 左条件, 右块, 合并块);
    } else {
        创建条件分支(构建器, 左条件, 合并块, 右块);
    }

    基本块* 左块 = 获取插入块(构建器);
    设置插入点(构建器, 右块);
    LLVM值* 右值 = 生成右值();
    if (右值->getType()->isIntegerTy(1)) 右值 = 创建零扩展(构建器, 右值, 整型32(上下文));
    LLVM值* 右条件 = 创建整数比较不等(构建器, 右值, 获取整数常量(上下文, 0, 32),
        是与操作 ? "landtmp2" : "lortmp2");
    右块 = 获取插入块(构建器);
    创建分支(构建器, 合并块);
    当前函数->insert(当前函数->end(), 合并块);
    设置插入点(构建器, 合并块);

    PHI节点* phi = 创建PHI(构建器, 整型1(上下文), 2, 是与操作 ? "land" : "lor");
    phi->addIncoming(获取整数常量(上下文, 是与操作 ? 0 : 1, 1), 左块);
    phi->addIncoming(右条件, 右块);
    return 创建零扩展(构建器, phi, 整型32(上下文), 是与操作 ? "landext" : "lorext");
}

LLVM值* 查找结构体成员地址(llvm::IRBuilder<>& 构建器, llvm::LLVMContext& 上下文,
    const std::string& 对象名, const std::string& 成员名,
    符号表& 符号表实例,
    std::unordered_map<std::string, LLVM结构体类型*>& 结构体类型映射,
    std::unordered_map<std::string, std::vector<std::string>>& 结构体成员映射) {
    LLVM值* 对象地址 = 符号表实例.获取变量值(对象名);
    if (!对象地址) throw std::runtime_error("未定义的变量: " + 对象名);

    const std::string& 结构体名 = 符号表实例.获取结构体类型(对象名);
    if (结构体名.empty()) throw std::runtime_error("变量不是结构体: " + 对象名);

    auto it = 结构体类型映射.find(结构体名);
    if (it == 结构体类型映射.end()) throw std::runtime_error("未定义的结构体: " + 结构体名);

    LLVM结构体类型* 结构体类型 = it->second;
    const auto& 成员名列表 = 结构体成员映射[结构体名];
    int 成员索引 = -1;
    for (size_t i = 0; i < 成员名列表.size(); i++) {
        if (成员名列表[i] == 成员名) { 成员索引 = static_cast<int>(i); break; }
    }
    if (成员索引 < 0) throw std::runtime_error("未定义的成员: " + 成员名);

    // 检查是否需要加载指针（结构体参数是指针类型）
    分配指令* alloca = LLVM动态转换<分配指令>(对象地址);
    if (alloca && alloca->getAllocatedType()->isPointerTy()) {
        对象地址 = 创建加载(构建器, 获取指针类型(上下文), 对象地址, 对象名 + "_加载");
    }

    return 创建结构体GEP(构建器, 结构体类型, 对象地址, static_cast<unsigned>(成员索引), 成员名);
}
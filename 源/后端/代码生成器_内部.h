#ifndef 代码生成器_内部_H
#define 代码生成器_内部_H

#include "代码生成器.h"
#include "../前端/公共.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include <system_error>
#include <cstdlib>

extern std::unordered_map<std::string, const 函数*> 全局函数定义映射;

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
inline int 执行命令(const std::string& 命令) {
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 命令.c_str(), (int)命令.size(), nullptr, 0);
    std::vector<wchar_t> 宽命令(宽长度 + 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, 命令.c_str(), (int)命令.size(), 宽命令.data(), 宽长度);
    return _wsystem(宽命令.data());
}
#else
inline int 执行命令(const std::string& 命令) { return std::system(命令.c_str()); }
#endif

inline bool 在列表中(const std::string& 名称, const std::vector<std::string>& 列表) {
    for (const auto& 项 : 列表) {
        if (项 == 名称) return true;
    }
    return false;
}

#endif

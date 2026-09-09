#ifndef 代码生成器_内部_H
#define 代码生成器_内部_H

#include "代码生成器.h"
#include "LLVM辅助.h"
#include "C运行时声明.h"
#include "../前端/公共.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/TargetParser/Triple.h"
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
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#include "../共享/LLVM中文.h"

using namespace llvm中文;

extern std::unordered_map<std::string, const 函数*> 全局函数定义映射;
extern LLVM值* 协程句柄;
extern std::vector<分配指令*> 返回值变量列表;

// ============================================================================
// 命令执行
// ============================================================================
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <process.h>
inline int 执行命令(const std::string& 命令) {
    // 用宽字符 API 调用，避免 UTF-8 中文命令在 ANSI 代码页下乱码。
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 命令.c_str(), (int)命令.size(), nullptr, 0);
    std::vector<wchar_t> 宽命令(宽长度 + 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, 命令.c_str(), (int)命令.size(), 宽命令.data(), 宽长度);
    return _wsystem(宽命令.data());
}
#elif defined(__CYGWIN__)
// Cygwin：走 POSIX 系统调用（sh），路径为 cygwin 风格。
#include <unistd.h>
inline int 执行命令(const std::string& 命令) { return std::system(命令.c_str()); }
#else
// Linux / macOS / 其他 POSIX
#include <unistd.h>
inline int 执行命令(const std::string& 命令) { return std::system(命令.c_str()); }
#endif

// ============================================================================
// 运行时辅助对象路径
//
// 链接器需要运行时辅助.o。该文件随编译器分发，位于
//   <可执行文件目录>/源/运行时/运行时辅助.o
// 为跨环境兼容（Windows 原生 / MSYS2 / Cygwin / Linux），我们不直接把中文路径
// 传给 clang++/ld——不同环境对命令行中文路径的编码处理不一致（Windows 窄字符
// API 用 ANSI 代码页，会把 UTF-8 中文解析错）。而是：
//   1) 用宽字符/POSIX API 可靠地定位到真实的 .o 文件；
//   2) 将它拷贝到一个纯 ASCII 的临时文件；
//   3) 返回该 ASCII 临时路径，链接命令行中不再出现任何中文路径。
// ============================================================================
namespace 运行时路径细节 {

#ifdef _WIN32
inline std::wstring 可执行文件目录宽() {
    wchar_t 缓冲[MAX_PATH] = {0};
    DWORD 长度 = GetModuleFileNameW(nullptr, 缓冲, MAX_PATH);
    if (长度 == 0 || 长度 >= MAX_PATH) return L"";
    for (DWORD i = 长度; i-- > 0; ) {
        if (缓冲[i] == L'\\' || 缓冲[i] == L'/') { 缓冲[i] = 0; break; }
    }
    return std::wstring(缓冲);
}
inline bool 文件存在宽(const std::wstring& 路径) {
    return GetFileAttributesW(路径.c_str()) != INVALID_FILE_ATTRIBUTES;
}
inline std::wstring 临时文件宽() {
    wchar_t 临时目录[MAX_PATH] = {0};
    GetTempPathW(MAX_PATH, 临时目录);
    wchar_t 名[MAX_PATH];
    wsprintfW(名, L"%sriyue_rt_%lu.o", 临时目录, GetCurrentProcessId());
    return std::wstring(名);
}
inline bool 拷贝文件宽(const std::wstring& 源, const std::wstring& 目标) {
    return CopyFileW(源.c_str(), 目标.c_str(), FALSE) != 0;
}
inline std::string 宽转UTF8(const std::wstring& 宽) {
    if (宽.empty()) return std::string();
    int 需 = WideCharToMultiByte(CP_UTF8, 0, 宽.c_str(), (int)宽.size(), nullptr, 0, nullptr, nullptr);
    std::string 窄(需, '\0');
    WideCharToMultiByte(CP_UTF8, 0, 宽.c_str(), (int)宽.size(), 窄.data(), 需, nullptr, nullptr);
    return 窄;
}
#else
#include <climits>
#include <sys/stat.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
inline std::string 可执行文件目录() {
    char 缓冲[PATH_MAX] = {0};
    ssize_t n = -1;
#if defined(__linux__) || defined(__CYGWIN__)
    n = readlink("/proc/self/exe", 缓冲, sizeof(缓冲) - 1);
#elif defined(__APPLE__)
    uint32_t 元素数量 = sizeof(缓冲);
    if (_NSGetExecutablePath(缓冲, &元素数量) == 0) n = (ssize_t)strlen(缓冲);
#endif
    if (n <= 0) return std::string();
    std::string 路径(缓冲, n);
    size_t 斜杠 = 路径.find_last_of('/');
    return (斜杠 == std::string::npos) ? std::string() : 路径.substr(0, 斜杠);
}
inline bool 文件存在(const std::string& 路径) {
    struct stat st;
    return stat(路径.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}
inline std::string 临时文件() {
    const char* 目录 = getenv("TMPDIR");
    if (!目录 || !*目录) 目录 = "/tmp";
    return std::string(目录) + "/riyue_rt_" + std::to_string(getpid()) + ".o";
}
inline bool 拷贝文件(const std::string& 源, const std::string& 目标) {
    std::ifstream 入(源, std::ios::binary);
    if (!入) return false;
    std::ofstream 出(目标, std::ios::binary);
    if (!出) return false;
    出 << 入.rdbuf();
    return (bool)出;
}
#endif

} // namespace 运行时路径细节

// 返回链接用的运行时对象路径（保证为纯 ASCII 临时文件路径）。
// 找不到真实 .o 时回退到源码树相对路径（开发场景）。
inline std::string 运行时对象路径() {
#ifdef _WIN32
    using namespace 运行时路径细节;
    std::wstring 目录 = 可执行文件目录宽();
    std::wstring 候选;
    if (!目录.empty()) {
        std::wstring 旁 = 目录 + L"\\源\\运行时\\运行时辅助.o";
        if (文件存在宽(旁)) 候选 = 旁;
    }
    if (候选.empty()) {
        // 开发回退：当前工作目录下的源码树相对路径（宽字符构造）。
        std::wstring 相对 = L"源\\运行时\\运行时辅助.o";
        if (文件存在宽(相对)) 候选 = 相对;
    }
    if (!候选.empty()) {
        std::wstring 临时 = 临时文件宽();
        if (拷贝文件宽(候选, 临时)) return 宽转UTF8(临时);
        // 拷贝失败则直接返回原始路径（可能链接失败，但保留原行为）
        return 宽转UTF8(候选);
    }
    return "源/运行时/运行时辅助.o";
#else
    using namespace 运行时路径细节;
    std::string 目录 = 可执行文件目录();
    std::string 候选;
    if (!目录.empty()) {
        std::string 旁 = 目录 + "/源/运行时/运行时辅助.o";
        if (文件存在(旁)) 候选 = 旁;
    }
    if (候选.empty() && 文件存在("源/运行时/运行时辅助.o"))
        候选 = "源/运行时/运行时辅助.o";
    if (!候选.empty()) {
        std::string 临时 = 临时文件();
        if (拷贝文件(候选, 临时)) return 临时;
        return 候选;
    }
    return "源/运行时/运行时辅助.o";
#endif
}

inline bool 在列表中(const std::string& 名称, const std::vector<std::string>& 列表) {
    for (const auto& 项 : 列表) {
        if (项 == 名称) return true;
    }
    return false;
}

#endif

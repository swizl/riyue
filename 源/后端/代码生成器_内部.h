#如果未定义 代码生成器_内部_H
#定义 代码生成器_内部_H

#包含 "代码生成器.h"
#包含 "LLVM辅助.h"
#包含 "C运行时声明.h"
#包含 "../前端/公共.h"
#包含 "llvm/IR/Verifier.h"
#包含 "llvm/Support/FileSystem.h"
#包含 "llvm/Support/CodeGen.h"
#包含 "llvm/TargetParser/Host.h"
#包含 "llvm/TargetParser/Triple.h"
#包含 "llvm/MC/TargetRegistry.h"
#包含 "llvm/Support/TargetSelect.h"
#包含 "llvm/Target/TargetMachine.h"
#包含 "llvm/IR/LegacyPassManager.h"
#包含 "llvm/Transforms/Scalar.h"
#包含 "llvm/Transforms/Utils.h"
#包含 "llvm/Transforms/InstCombine/InstCombine.h"
#包含 "llvm/Transforms/Scalar/GVN.h"
#包含 <system_error>
#包含 <cstdlib>
#包含 <cstdio>
#包含 <cstring>
#包含 <fstream>
#包含 <string>

#包含 "../共享/LLVM中文.h"

取用 名域 llvm中文;

extern 哈希映射<文本, 恒常 函数*> 全局函数定义映射;
extern LLVM值* 协程句柄;
extern 数组向量<分配指令*> 返回值变量列表;

// ============================================================================
// 命令执行
// ============================================================================
#如果定义 _WIN32
#如果未定义 WIN32_LEAN_AND_MEAN
#定义 WIN32_LEAN_AND_MEAN
#结束
#包含 <windows.h>
#包含 <process.h>
内联 整数型 执行命令(恒常 文本& 命令) {
    // 用宽字符 API 调用，避免 UTF-8 中文命令在 ANSI 代码页下乱码。
    整数型 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 命令.c_str(), (整数型)命令.size(), 空针, 0);
    数组向量<宽字符型> 宽命令(宽长度 + 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, 命令.c_str(), (整数型)命令.size(), 宽命令.data(), 宽长度);
    归返 _wsystem(宽命令.data());
}
#否则如果 已定义(__CYGWIN__)
// Cygwin：走 POSIX 系统调用（sh），路径为 cygwin 风格。
#包含 <unistd.h>
内联 整数型 执行命令(恒常 文本& 命令) { 归返 std::system(命令.c_str()); }
#否则
// Linux / macOS / 其他 POSIX
#包含 <unistd.h>
内联 整数型 执行命令(恒常 文本& 命令) { 归返 std::system(命令.c_str()); }
#结束

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
名域 运行时路径细节 {

#如果定义 _WIN32
内联 std::wstring 可执行文件目录宽() {
    宽字符型 缓冲[MAX_PATH] = {0};
    DWORD 长度 = GetModuleFileNameW(空针, 缓冲, MAX_PATH);
    如果 (长度 == 0 || 长度 >= MAX_PATH) 归返 L"";
    循环 (DWORD i = 长度; i-- > 0; ) {
        如果 (缓冲[i] == L'\\' || 缓冲[i] == L'/') { 缓冲[i] = 0; 中断; }
    }
    归返 std::wstring(缓冲);
}
内联 真假型 文件存在宽(恒常 std::wstring& 路径) {
    归返 GetFileAttributesW(路径.c_str()) != INVALID_FILE_ATTRIBUTES;
}
内联 std::wstring 临时文件宽() {
    宽字符型 临时目录[MAX_PATH] = {0};
    GetTempPathW(MAX_PATH, 临时目录);
    宽字符型 名[MAX_PATH];
    wsprintfW(名, L"%sriyue_rt_%lu.o", 临时目录, GetCurrentProcessId());
    归返 std::wstring(名);
}
内联 真假型 拷贝文件宽(恒常 std::wstring& 源, 恒常 std::wstring& 目标) {
    归返 CopyFileW(源.c_str(), 目标.c_str(), 假值) != 0;
}
内联 文本 宽转UTF8(恒常 std::wstring& 宽) {
    如果 (宽.empty()) 归返 文本();
    整数型 需 = WideCharToMultiByte(CP_UTF8, 0, 宽.c_str(), (整数型)宽.size(), 空针, 0, 空针, 空针);
    文本 窄(需, '\0');
    WideCharToMultiByte(CP_UTF8, 0, 宽.c_str(), (整数型)宽.size(), 窄.data(), 需, 空针, 空针);
    归返 窄;
}
#否则
#包含 <climits>
#包含 <sys/stat.h>
#如果 已定义(__APPLE__)
#包含 <mach-o/dyld.h>
#结束
内联 文本 可执行文件目录() {
    字符型 缓冲[PATH_MAX] = {0};
    s大小类型 n = -1;
#如果 已定义(__linux__) || defined(__CYGWIN__)
    n = readlink("/proc/self/exe", 缓冲, 大小计算(缓冲) - 1);
#否则如果 已定义(__APPLE__)
    uint32_t 元素数量 = 大小计算(缓冲);
    如果 (_NSGetExecutablePath(缓冲, &元素数量) == 0) n = (s大小类型)strlen(缓冲);
#结束
    如果 (n <= 0) 归返 文本();
    文本 路径(缓冲, n);
    大小类型 斜杠 = 路径.find_last_of('/');
    归返 (斜杠 == 文本::npos) ? 文本() : 路径.substr(0, 斜杠);
}
内联 真假型 文件存在(恒常 文本& 路径) {
    构型 stat st;
    归返 stat(路径.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}
内联 文本 临时文件() {
    恒常 字符型* 目录 = getenv("TMPDIR");
    如果 (!目录 || !*目录) 目录 = "/tmp";
    归返 文本(目录) + "/riyue_rt_" + 转为文本(getpid()) + ".o";
}
内联 真假型 拷贝文件(恒常 文本& 源, 恒常 文本& 目标) {
    文件输入流 入(源, 标准IO::binary);
    如果 (!入) 归返 假值;
    文件输出流 出(目标, 标准IO::binary);
    如果 (!出) 归返 假值;
    出 << 入.rdbuf();
    归返 (真假型)出;
}
#结束

} // 名域 运行时路径细节

// 返回链接用的运行时对象路径（保证为纯 ASCII 临时文件路径）。
// 找不到真实 .o 时回退到源码树相对路径（开发场景）。
内联 文本 运行时对象路径() {
#如果定义 _WIN32
    取用 名域 运行时路径细节;
    std::wstring 目录 = 可执行文件目录宽();
    std::wstring 候选;
    如果 (!目录.empty()) {
        std::wstring 旁 = 目录 + L"\\源\\运行时\\运行时辅助.o";
        如果 (文件存在宽(旁)) 候选 = 旁;
    }
    如果 (候选.empty()) {
        // 开发回退：当前工作目录下的源码树相对路径（宽字符构造）。
        std::wstring 相对 = L"源\\运行时\\运行时辅助.o";
        如果 (文件存在宽(相对)) 候选 = 相对;
    }
    如果 (!候选.empty()) {
        std::wstring 临时 = 临时文件宽();
        如果 (拷贝文件宽(候选, 临时)) 归返 宽转UTF8(临时);
        // 拷贝失败则直接返回原始路径（可能链接失败，但保留原行为）
        归返 宽转UTF8(候选);
    }
    归返 "源/运行时/运行时辅助.o";
#否则
    取用 名域 运行时路径细节;
    文本 目录 = 可执行文件目录();
    文本 候选;
    如果 (!目录.empty()) {
        文本 旁 = 目录 + "/源/运行时/运行时辅助.o";
        如果 (文件存在(旁)) 候选 = 旁;
    }
    如果 (候选.empty() && 文件存在("源/运行时/运行时辅助.o"))
        候选 = "源/运行时/运行时辅助.o";
    如果 (!候选.empty()) {
        文本 临时 = 临时文件();
        如果 (拷贝文件(候选, 临时)) 归返 临时;
        归返 候选;
    }
    归返 "源/运行时/运行时辅助.o";
#结束
}

内联 真假型 在列表中(恒常 文本& 名称, 恒常 数组向量<文本>& 列表) {
    循环 (恒常 自动& 项 : 列表) {
        如果 (项 == 名称) 归返 真值;
    }
    归返 假值;
}

#结束

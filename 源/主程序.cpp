#include "前端/语法分析器.h"
#include "后端/代码生成器.h"
#include "虚拟机/字节码编译器.h"
#include "虚拟机/虚拟机.h"
#include "前端/公共.h"
#include "llvm/IR/LLVMContext.h"
#include <iostream>
#include <string>
#include <vector>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>

extern "C" {
    void 设置参数(int argc, const char** argv);
}

bool 调试模式 = false;

static std::string 宽字符转UTF8(const std::wstring& 宽串) {
    if (宽串.empty()) return "";
    int 长度 = WideCharToMultiByte(CP_UTF8, 0, 宽串.c_str(), (int)宽串.size(), nullptr, 0, nullptr, nullptr);
    std::string 结果(长度, 0);
    WideCharToMultiByte(CP_UTF8, 0, 宽串.c_str(), (int)宽串.size(), &结果[0], 长度, nullptr, nullptr);
    return 结果;
}

static std::vector<std::string> 获取UTF8参数() {
    int 参数数量 = 0;
    LPWSTR* 宽参数 = CommandLineToArgvW(GetCommandLineW(), &参数数量);
    std::vector<std::string> 结果;
    for (int i = 0; i < 参数数量; i++) {
        结果.push_back(宽字符转UTF8(宽参数[i]));
    }
    LocalFree(宽参数);
    return 结果;
}

void 显示用法(const std::string& 程序名) {
    std::cerr << "用法:" << std::endl;
    std::cerr << "  " << 程序名 << " [--调试] [--优化] 输入文件.心 输出文件    (编译为原生可执行文件)" << std::endl;
    std::cerr << "  " << 程序名 << " --运行 输入文件.心               (虚拟机直接运行)" << std::endl;
    std::cerr << "  " << 程序名 << " --字节码 输入文件.心 输出.riue   (编译为字节码)" << std::endl;
    std::cerr << "  " << 程序名 << " --执行 输出.riue                (执行字节码文件)" << std::endl;
    std::cerr << "  " << 程序名 << " --交互                         (交互式REPL)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "优化选项:" << std::endl;
    std::cerr << "  --优化    启用编译器优化（尾递归优化、死代码消除、内联优化、循环优化）" << std::endl;
}

static bool 是完整语句(const std::string& 输入) {
    int 花括号深度 = 0;
    int 圆括号深度 = 0;
    int 方括号深度 = 0;
    bool 在字符串中 = false;
    bool 在插值中 = false;

    for (size_t i = 0; i < 输入.size(); i++) {
        char c = 输入[i];
        if (c == '"' && (i == 0 || 输入[i-1] != '\\')) {
            if (!在插值中) {
                在字符串中 = !在字符串中;
            }
            continue;
        }
        if (在字符串中) {
            if (c == '{') { 在插值中 = true; 花括号深度++; }
            else if (c == '}' && 在插值中) { 在插值中 = false; 花括号深度--; }
            continue;
        }
        if (c == '{') 花括号深度++;
        else if (c == '}') 花括号深度--;
        else if (c == '(') 圆括号深度++;
        else if (c == ')') 圆括号深度--;
        else if (c == '[') 方括号深度++;
        else if (c == ']') 方括号深度--;
    }

    if (花括号深度 != 0 || 圆括号深度 != 0 || 方括号深度 != 0) return false;

    size_t pos = 输入.find_last_not_of(" \t\n\r");
    if (pos == std::string::npos) return false;
    char last = 输入[pos];
    if (last == ';' || last == '}') return true;
    if (last == '+' || last == '-' || last == '*' || last == '/' || last == '%' ||
        last == '&' || last == '|' || last == '=' || last == '<' || last == '>' ||
        last == '!' || last == ',' || last == '(' || last == '[' || last == '{') return false;
    return true;
}

static std::unique_ptr<class 语句> 解析单条输入(const std::string& 输入) {
    try {
        语法分析器 分析器(输入.c_str(), 输入.size());
        auto 结果 = 分析器.解析语句();
        return 结果;
    } catch (...) {}

    try {
        语法分析器 分析器(输入.c_str(), 输入.size());
        auto 表达式 = 分析器.解析表达式();
        return std::make_unique<打印语句>(std::move(表达式));
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("解析失败: ") + e.what());
    }
}

int 运行REPL() {
    SetConsoleOutputCP(65001);
    std::cout << "日月交互模式 (输入 '退出' 退出)" << std::endl;

    std::vector<std::string> 输入历史;
    std::string 缓冲区;

    while (true) {
        if (缓冲区.empty()) {
            std::cout << "日月> ";
        } else {
            std::cout << "  .. ";
        }
        std::cout.flush();

        std::string 行;
        if (!std::getline(std::cin, 行)) break;

        if (缓冲区.empty() && (行 == "退出" || 行 == "exit" || 行 == "quit")) break;
        if (行.empty() && 缓冲区.empty()) continue;

        if (!缓冲区.empty()) 缓冲区 += "\n";
        缓冲区 += 行;

        if (!是完整语句(缓冲区)) continue;

        try {
            auto 语句 = 解析单条输入(缓冲区);
            输入历史.push_back(缓冲区);
            缓冲区.clear();

            std::vector<std::unique_ptr<class 语句>> 语句列表;
            for (const auto& 历史 : 输入历史) {
                语句列表.push_back(解析单条输入(历史));
            }

            auto 程序 = std::make_unique<struct 程序>();
            auto 主函数 = std::make_unique<函数>("__repl_main", std::vector<std::string>{}, std::vector<函数参数>{}, std::vector<返回值描述>{}, std::move(语句列表));
            程序->函数列表.push_back(std::move(主函数));

            字节码编译器 编译器;
            auto 字节码结果 = 编译器.编译(*程序);

            虚拟机 VM;
            VM.加载(字节码结果);
            VM.执行();
        } catch (const std::exception& e) {
            std::cerr << "错误: " << e.what() << std::endl;
            缓冲区.clear();
        }
    }

    std::cout << "再见!" << std::endl;
    return 0;
}

int main() {
    auto 参数 = 获取UTF8参数();
    int argc = (int)参数.size();

    // 设置命令行参数供运行时使用
    std::vector<const char*> 参数指针;
    for (const auto& arg : 参数) 参数指针.push_back(arg.c_str());
    设置参数(argc, 参数指针.data());

    if (argc < 2) {
        显示用法(参数[0]);
        return 1;
    }

    std::string 模式 = "编译";
    int 输入文件索引 = 1;
    bool 启用优化 = false;

    for (int i = 1; i < argc; i++) {
        if (参数[i] == "--调试") {
            调试模式 = true;
            输入文件索引 = i + 1;
        } else if (参数[i] == "--优化") {
            启用优化 = true;
            输入文件索引 = i + 1;
        } else if (参数[i] == "--运行") {
            模式 = "运行";
            输入文件索引 = i + 1;
        } else if (参数[i] == "--字节码") {
            模式 = "字节码";
            输入文件索引 = i + 1;
        } else if (参数[i] == "--执行") {
            模式 = "执行";
            输入文件索引 = i + 1;
        } else if (参数[i] == "--交互") {
            return 运行REPL();
        }
    }

    if (输入文件索引 >= argc) {
        显示用法(参数[0]);
        return 1;
    }

    std::string 输入文件 = 参数[输入文件索引];

    try {
        if (模式 == "运行") {
            // VM模式：解析 → 编译字节码 → 虚拟机执行
            语法分析器 分析器(输入文件);
            auto 程序 = 分析器.解析程序();

            字节码编译器 编译器;
            auto 字节码结果 = 编译器.编译(*程序);

            虚拟机 VM;
            VM.加载(字节码结果);
            return VM.执行();

        } else if (模式 == "字节码") {
            // 字节码模式：解析 → 编译字节码 → 输出文件
            if (输入文件索引 + 1 >= argc) {
                std::cerr << "错误: 字节码模式需要输出文件" << std::endl;
                return 1;
            }
            std::string 输出文件 = 参数[输入文件索引 + 1];

            语法分析器 分析器(输入文件);
            auto 程序 = 分析器.解析程序();

            字节码编译器 编译器;
            auto 字节码结果 = 编译器.编译(*程序);
            字节码结果.输出(输出文件);

            std::cout << "字节码已生成: " << 输出文件 << std::endl;
            std::cout << "  常量数量: " << 字节码结果.常量池.size() << std::endl;
            std::cout << "  函数数量: " << 字节码结果.函数表.size() << std::endl;
            for (const auto& 函数 : 字节码结果.函数表) {
                std::cout << "  函数 '" << 函数.名称 << "': 参数=" << static_cast<int>(函数.参数数量)
                          << " 寄存器=" << static_cast<int>(函数.最大寄存器)
                          << " 指令=" << 函数.指令列表.size() << std::endl;
            }
            return 0;

        } else if (模式 == "执行") {
            // 执行字节码文件
            虚拟机 VM;
            VM.加载文件(输入文件);
            return VM.执行();

        } else {
            // LLVM编译模式
            if (输入文件索引 + 1 >= argc) {
                std::cerr << "错误: 编译模式需要输出文件" << std::endl;
                return 1;
            }
            std::string 输出文件 = 参数[输入文件索引 + 1];

            语法分析器 分析器(输入文件);
            auto 程序 = 分析器.解析程序();

            // 处理导入
            auto 导入列表 = 分析器.获取导入列表();
            auto 导入别名映射 = 分析器.获取导入别名映射();
            auto 选择导入映射 = 分析器.获取选择导入映射();
            
            // 获取输入文件所在目录（用于路径解析）
            std::string 输入目录;
            size_t 最后分隔符 = 输入文件.find_last_of("/\\");
            if (最后分隔符 != std::string::npos) {
                输入目录 = 输入文件.substr(0, 最后分隔符 + 1);
            }
            
            for (const auto& 模块名 : 导入列表) {
                std::string 模块路径 = 模块名;
                
                调试打印("[主程序] 原始模块路径: " << 模块路径);
                调试打印("[主程序] 输入目录: " << 输入目录);
                
                // 路径解析：相对路径基于当前文件目录
                if (模块路径.find(":") == std::string::npos && 模块路径[0] != '/') {
                    // 只有当模块路径不以输入目录开头时才添加
                    if (输入目录.empty() || 模块路径.substr(0, 输入目录.size()) != 输入目录) {
                        模块路径 = 输入目录 + 模块路径;
                        调试打印("[主程序] 添加目录后: " << 模块路径);
                    } else {
                        调试打印("[主程序] 路径已包含目录，跳过添加");
                    }
                }
                
                // 尝试添加.心后缀
                if (模块路径.find(".心") == std::string::npos) {
                    模块路径 += ".心";
                }
                调试打印("[主程序] 导入模块: " << 模块路径);
                try {
                    语法分析器 模块分析器(模块路径);
                    auto 模块程序 = 模块分析器.解析程序();
                    
                    // 检查是否有选择性导入
                    auto 选择导入 = 选择导入映射.find(模块名);
                    if (选择导入 != 选择导入映射.end()) {
                        // 只导入指定的函数
                        for (const auto& 函数名 : 选择导入->second) {
                            for (auto& 函数 : 模块程序->函数列表) {
                                if (函数->名称 == 函数名) {
                                    程序->函数列表.insert(程序->函数列表.begin(), std::move(函数));
                                    break;
                                }
                            }
                        }
                    } else {
                        // 合并所有函数（插入到主程序函数之前）
                        size_t 插入位置 = 0;
                        for (auto& 函数 : 模块程序->函数列表) {
                            程序->函数列表.insert(程序->函数列表.begin() + 插入位置, std::move(函数));
                            插入位置++;
                        }
                    }
                    
                    // 合并结构体定义
                    for (auto& 结构体 : 模块程序->结构体定义列表) {
                        程序->结构体定义列表.push_back(std::move(结构体));
                    }
                    // 合并枚举定义
                    for (auto& 枚举 : 模块程序->枚举定义列表) {
                        程序->枚举定义列表.push_back(std::move(枚举));
                    }
                    // 合并全局变量
                    for (auto& 全局变量 : 模块程序->全局变量) {
                        程序->全局变量.push_back(std::move(全局变量));
                    }
                } catch (const std::exception& e) {
                    std::cerr << "导入模块失败: " << 模块路径 << " - " << e.what() << std::endl;
                    return 1;
                }
            }

            llvm::LLVMContext 上下文;
            代码生成器 生成器(上下文, 输出文件, 启用优化);
            生成器.生成(*程序);
            生成器.生成可执行文件();

            std::cout << "编译完成: " << 输出文件;
            if (启用优化) std::cout << " (已优化)";
            std::cout << std::endl;
            return 0;
        }
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#include "前端/语法分析器.h"
#include "前端/诊断.h"
#include "后端/代码生成器.h"
#include "虚拟机/字节码编译器.h"
#include "虚拟机/虚拟机.h"
#include "前端/公共.h"
#include "llvm/IR/LLVMContext.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>

extern "C" {
    void 设置参数(int argc, const char** argv);
    void 设置覆盖率(int 启用);
    void 设置覆盖率文件(const char* 文件名);
    void 输出覆盖率报告();
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
    std::cout << "日月交互模式 v1.0" << std::endl;
    std::cout << "输入 '帮助' 查看可用命令" << std::endl;

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

        // 处理特殊命令
        if (缓冲区.empty()) {
            if (行 == "退出" || 行 == "exit" || 行 == "quit") break;
            
            if (行 == "帮助" || 行 == "help") {
                std::cout << "可用命令:" << std::endl;
                std::cout << "  帮助     - 显示此帮助信息" << std::endl;
                std::cout << "  历史     - 显示输入历史" << std::endl;
                std::cout << "  清屏     - 清除屏幕" << std::endl;
                std::cout << "  变量     - 显示当前变量" << std::endl;
                std::cout << "  退出     - 退出REPL" << std::endl;
                std::cout << std::endl;
                std::cout << "支持的语法:" << std::endl;
                std::cout << "  42 = x;          - 变量声明" << std::endl;
                std::cout << "  (x)打印;         - 打印表达式" << std::endl;
                std::cout << "  函数 名 {}       - 函数定义" << std::endl;
                std::cout << "  如果 (条件) {}   - 条件语句" << std::endl;
                std::cout << "  循环 (i: 0, 10) {} - 循环语句" << std::endl;
                continue;
            }
            
            if (行 == "历史" || 行 == "history") {
                std::cout << "输入历史:" << std::endl;
                for (size_t i = 0; i < 输入历史.size(); i++) {
                    std::cout << "  " << (i + 1) << ": " << 输入历史[i] << std::endl;
                }
                continue;
            }
            
            if (行 == "清屏" || 行 == "clear") {
                #ifdef _WIN32
                system("cls");
                #else
                system("clear");
                #endif
                continue;
            }
            
            if (行 == "变量" || 行 == "vars") {
                std::cout << "变量功能需要在执行后查看" << std::endl;
                continue;
            }
        }

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
            auto 主函数 = std::make_unique<函数>("__repl_main", std::vector<std::string>{}, std::vector<类型约束>{}, std::vector<函数参数>{}, std::vector<返回值描述>{}, std::move(语句列表));
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
        } else if (参数[i] == "--调试器") {
            模式 = "调试器";
            输入文件索引 = i + 1;
        } else if (参数[i] == "--覆盖率") {
            模式 = "覆盖率";
            输入文件索引 = i + 1;
        } else if (参数[i] == "--文档") {
            模式 = "文档";
            输入文件索引 = i + 1;
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

        } else if (模式 == "调试器") {
            // 调试器模式：编译并运行，支持断点
            语法分析器 分析器(输入文件);
            auto 程序 = 分析器.解析程序();

            字节码编译器 编译器;
            auto 字节码结果 = 编译器.编译(*程序);

            虚拟机 VM;
            VM.加载(字节码结果);
            std::cout << "[调试器] 程序已加载，输入'帮助'查看命令" << std::endl;
            return VM.执行();

        } else if (模式 == "覆盖率") {
            // 覆盖率模式：编译并运行，统计代码覆盖率
            语法分析器 分析器(输入文件);
            auto 程序 = 分析器.解析程序();

            字节码编译器 编译器;
            auto 字节码结果 = 编译器.编译(*程序);

            虚拟机 VM;
            VM.加载(字节码结果);
            设置覆盖率(1);
            设置覆盖率文件(输入文件.c_str());
            int 结果 = VM.执行();
            输出覆盖率报告();
            return 结果;

        } else if (模式 == "文档") {
            // 文档生成模式：解析源码并生成文档
            语法分析器 分析器(输入文件);
            auto 程序 = 分析器.解析程序();

            std::string 输出文件 = (输入文件索引 + 1 < argc) ? 参数[输入文件索引 + 1] : 输入文件 + ".md";

            std::ofstream 文档文件(输出文件);
            if (!文档文件.is_open()) {
                std::cerr << "错误: 无法创建文档文件: " << 输出文件 << std::endl;
                return 1;
            }

            文档文件 << "# " << 输入文件 << " API 文档\n\n";
            文档文件 << "> 由日月编译器自动生成\n\n";

            // 生成函数文档
            if (!程序->函数列表.empty()) {
                文档文件 << "## 函数\n\n";
                for (const auto& 函数 : 程序->函数列表) {
                    文档文件 << "### " << 函数->名称 << "\n\n";
                    if (函数->是否协程) {
                        文档文件 << "**类型**: 协程\n\n";
                    }
                    if (!函数->参数列表.empty()) {
                        文档文件 << "**参数**:\n\n";
                        文档文件 << "| 名称 | 类型 | 默认值 |\n";
                        文档文件 << "|------|------|--------|\n";
                        for (const auto& 参数 : 函数->参数列表) {
                            文档文件 << "| " << 参数.名称 << " | " << (参数.类型.empty() ? "整数" : 参数.类型) << " | ";
                            if (参数.默认值) {
                                文档文件 << "有";
                            } else {
                                文档文件 << "-";
                            }
                            文档文件 << " |\n";
                        }
                        文档文件 << "\n";
                    }
                    if (!函数->返回值列表.empty()) {
                        文档文件 << "**返回值**:\n\n";
                        文档文件 << "| 名称 | 类型 |\n";
                        文档文件 << "|------|------|\n";
                        for (const auto& 返回值 : 函数->返回值列表) {
                            文档文件 << "| " << 返回值.名称 << " | " << (返回值.类型.empty() ? "整数" : 返回值.类型) << " |\n";
                        }
                        文档文件 << "\n";
                    }
                }
            }

            // 生成结构体文档
            if (!程序->结构体定义列表.empty()) {
                文档文件 << "## 结构体\n\n";
                for (const auto& [名称, 成员列表, 基类名] : 程序->结构体定义列表) {
                    文档文件 << "### " << 名称 << "\n\n";
                    文档文件 << "| 成员 | 类型 |\n";
                    文档文件 << "|------|------|\n";
                    for (const auto& 成员 : 成员列表) {
                        文档文件 << "| " << 成员.名称 << " | " << 成员.类型 << " |\n";
                    }
                    文档文件 << "\n";
                }
            }

            // 生成枚举文档
            if (!程序->枚举定义列表.empty()) {
                文档文件 << "## 枚举\n\n";
                for (const auto& [名称, 成员列表] : 程序->枚举定义列表) {
                    文档文件 << "### " << 名称 << "\n\n";
                    文档文件 << "| 成员 | 值 |\n";
                    文档文件 << "|------|----|\n";
                    for (const auto& 成员 : 成员列表) {
                        文档文件 << "| " << 成员.名称 << " | " << 成员.值 << " |\n";
                    }
                    文档文件 << "\n";
                }
            }

            文档文件.close();
            std::cout << "文档已生成: " << 输出文件 << std::endl;
            return 0;

        } else {
            // LLVM编译模式
            if (输入文件索引 + 1 >= argc) {
                std::cerr << "错误: 编译模式需要输出文件" << std::endl;
                return 1;
            }
            std::string 输出文件 = 参数[输入文件索引 + 1];

            // 读取源文件用于诊断
            std::ifstream 源文件流(输入文件, std::ios::binary);
            std::string 源码内容;
            if (源文件流.is_open()) {
                源码内容.assign((std::istreambuf_iterator<char>(源文件流)), std::istreambuf_iterator<char>());
            }

            诊断引擎 诊断;
            语法分析器 分析器(输入文件);
            分析器.设置诊断(&诊断);
            if (!源码内容.empty()) 分析器.设置源码(源码内容);
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

            // 获取可执行文件所在目录（用于标准库路径）
            std::string 程序目录;
            char 程序路径[MAX_PATH] = {0};
            GetModuleFileNameA(nullptr, 程序路径, MAX_PATH);
            std::string 程序路径串(程序路径);
            size_t 程序分隔符 = 程序路径串.find_last_of("/\\");
            if (程序分隔符 != std::string::npos) {
                程序目录 = 程序路径串.substr(0, 程序分隔符 + 1);
            }

            // 循环导入检测
            std::unordered_set<std::string> 已导入集合;

            // 模块查找函数
            auto 规范化路径 = [](std::string 路径) -> std::string {
                for (auto& c : 路径) {
                    if (c == '/') c = '\\';
                }
                return 路径;
            };

            auto 查找模块 = [&](const std::string& 原始路径) -> std::string {
                std::string 规范化原始 = 规范化路径(原始路径);
                std::vector<std::string> 候选;
                bool 需要后缀 = (规范化原始.find(".心") == std::string::npos);

                // 获取当前工作目录
                char 工作目录[MAX_PATH] = {0};
                GetCurrentDirectoryA(MAX_PATH, 工作目录);
                std::string 工作目录串(工作目录);
                if (工作目录串.back() != '\\') 工作目录串 += '\\';

                // 检查模块路径是否已经是绝对路径
                bool 是绝对路径 = (规范化原始.size() >= 2 && 规范化原始[1] == ':');

                if (!是绝对路径) {
                    // 1. 相对路径（基于当前工作目录）
                    候选.push_back(工作目录串 + 规范化原始);
                    if (需要后缀) 候选.push_back(工作目录串 + 规范化原始 + ".心");

                    // 2. 相对路径（基于输入文件目录）
                    if (!输入目录.empty()) {
                        std::string 规范化输入目录 = 规范化路径(输入目录);
                        // 检查模块路径是否已经以输入目录开头
                        if (规范化原始.substr(0, 规范化输入目录.size()) != 规范化输入目录) {
                            char 绝对路径[MAX_PATH] = {0};
                            if (GetFullPathNameA(规范化输入目录.c_str(), MAX_PATH, 绝对路径, nullptr)) {
                                std::string 绝对输入目录(绝对路径);
                                if (绝对输入目录.back() != '\\') 绝对输入目录 += '\\';
                                候选.push_back(绝对输入目录 + 规范化原始);
                                if (需要后缀) 候选.push_back(绝对输入目录 + 规范化原始 + ".心");
                            }
                        }
                    }

                    // 3. 标准库路径
                    if (!程序目录.empty()) {
                        候选.push_back(程序目录 + "标准库\\" + 规范化原始);
                        if (需要后缀) 候选.push_back(程序目录 + "标准库\\" + 规范化原始 + ".心");
                    }
                } else {
                    // 绝对路径
                    候选.push_back(规范化原始);
                    if (需要后缀) 候选.push_back(规范化原始 + ".心");
                }

                for (const auto& 路径 : 候选) {
                    // 使用宽字符API检查文件是否存在（支持中文路径）
                    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 路径.c_str(), (int)路径.size(), nullptr, 0);
                    std::vector<wchar_t> 宽路径(宽长度 + 1, 0);
                    MultiByteToWideChar(CP_UTF8, 0, 路径.c_str(), (int)路径.size(), 宽路径.data(), 宽长度);
                    std::ifstream 检查(宽路径.data());
                    if (检查.is_open()) {
                        检查.close();
                        调试打印("[主程序] 找到模块: " << 路径);
                        return 路径;
                    }
                }
                return "";
            };
            
            for (const auto& 模块名 : 导入列表) {
                std::string 模块路径 = 查找模块(模块名);
                
                if (模块路径.empty()) {
                    std::cerr << "导入模块失败: 找不到模块 '" << 模块名 << "'" << std::endl;
                    return 1;
                }

                调试打印("[主程序] 导入模块: " << 模块路径);

                // 循环导入检测
                if (已导入集合.count(模块路径) > 0) {
                    调试打印("[主程序] 跳过已导入模块: " << 模块路径);
                    continue;
                }

                try {
                    语法分析器 模块分析器(模块路径);
                    auto 模块程序 = 模块分析器.解析程序();
                    
                    // 标记为已导入
                    已导入集合.insert(模块路径);

                    // 应用别名重命名
                    auto 别名迭代 = 导入别名映射.find(模块名);
                    std::string 别名前缀;
                    if (别名迭代 != 导入别名映射.end()) {
                        别名前缀 = 别名迭代->second + "_";
                    }

                    // 检查是否有选择性导入
                    auto 选择导入 = 选择导入映射.find(模块名);
                    if (选择导入 != 选择导入映射.end()) {
                        // 只导入指定的函数
                        for (const auto& 函数名 : 选择导入->second) {
                            for (auto& 函数 : 模块程序->函数列表) {
                                if (函数->名称 == 函数名) {
                                    if (!别名前缀.empty()) {
                                        函数->名称 = 别名前缀 + 函数->名称;
                                    }
                                    程序->函数列表.insert(程序->函数列表.begin(), std::move(函数));
                                    break;
                                }
                            }
                        }
                    } else {
                        // 合并所有函数（插入到主程序函数之前）
                        size_t 插入位置 = 0;
                        for (auto& 函数 : 模块程序->函数列表) {
                            if (!别名前缀.empty()) {
                                函数->名称 = 别名前缀 + 函数->名称;
                            }
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

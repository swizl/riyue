// 日月代码格式化工具
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <locale>
#include <codecvt>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>

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
    std::cerr << "日月代码格式化工具" << std::endl;
    std::cerr << "用法:" << std::endl;
    std::cerr << "  " << 程序名 << " [--检查] 输入文件.心 [输出文件.心]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "选项:" << std::endl;
    std::cerr << "  --检查    检查格式是否正确，不修改文件" << std::endl;
    std::cerr << std::endl;
    std::cerr << "示例:" << std::endl;
    std::cerr << "  " << 程序名 << " 输入.心              # 格式化并覆盖原文件" << std::endl;
    std::cerr << "  " << 程序名 << " 输入.心 输出.心      # 格式化并输出到新文件" << std::endl;
    std::cerr << "  " << 程序名 << " --检查 输入.心       # 仅检查格式" << std::endl;
}

// 格式化状态
struct 格式化状态 {
    int 缩进级别 = 0;
    bool 在字符串中 = false;
    bool 在注释中 = false;
    bool 上一行是空行 = false;
    std::string 结果;
};

// 检查是否是空白字符
bool 是空白(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// 检查是否是关键字
bool 是关键字(const std::string& 词) {
    static std::vector<std::string> 关键字列表 = {
        "函数", "变量", "常量", "如果", "否则", "否则如果", "循环", "当", "做",
        "中断", "继续", "返回", "打印", "匹配", "遍历", "导入", "结构体", "枚举",
        "真", "假", "空", "整数", "浮点数", "布尔值", "字符串"
    };
    for (const auto& 关键字 : 关键字列表) {
        if (词 == 关键字) return true;
    }
    return false;
}

// 格式化一行代码
std::string 格式化行(const std::string& 行, int 缩进级别) {
    std::string 结果;
    for (int i = 0; i < 缩进级别; i++) {
        结果 += "    ";  // 4空格缩进
    }

    // 去除行首空白
    size_t 开始位置 = 行.find_first_not_of(" \t");
    if (开始位置 == std::string::npos) return "";

    结果 += 行.substr(开始位置);
    return 结果;
}

// 主格式化函数
std::string 格式化代码(const std::string& 源代码) {
    std::istringstream 输入(源代码);
    std::string 行;
    格式化状态 状态;
    std::vector<std::string> 输出行;
    int 上一行缩进 = 0;

    while (std::getline(输入, 行)) {
        // 跳过空行（但保留一个空行）
        if (行.find_first_not_of(" \t") == std::string::npos) {
            if (!状态.上一行是空行) {
                输出行.push_back("");
                状态.上一行是空行 = true;
            }
            continue;
        }
        状态.上一行是空行 = false;

        // 去除行尾空白
        size_t 结束位置 = 行.find_last_not_of(" \t\r\n");
        if (结束位置 != std::string::npos) {
            行 = 行.substr(0, 结束位置 + 1);
        }

        // 计算缩进变化
        int 缩进变化 = 0;
        for (char c : 行) {
            if (c == '{') 缩进变化++;
            else if (c == '}') 缩进变化--;
        }

        // 如果行以}开头，先减少缩进
        if (行.find_first_not_of(" \t") != std::string::npos) {
            char 首字符 = 行[行.find_first_not_of(" \t")];
            if (首字符 == '}') {
                状态.缩进级别 = std::max(0, 状态.缩进级别 - 1);
            }
        }

        // 格式化当前行
        std::string 格式化后的行 = 格式化行(行, 状态.缩进级别);
        if (!格式化后的行.empty()) {
            输出行.push_back(格式化后的行);
        }

        // 更新缩进级别
        if (缩进变化 > 0) {
            状态.缩进级别 += 缩进变化;
        }

        // 确保缩进级别不为负
        状态.缩进级别 = std::max(0, 状态.缩进级别);
    }

    // 合并输出
    std::string 结果;
    for (size_t i = 0; i < 输出行.size(); i++) {
        结果 += 输出行[i];
        if (i < 输出行.size() - 1) {
            结果 += "\n";
        }
    }
    结果 += "\n";

    return 结果;
}

// 检查格式是否正确
bool 检查格式(const std::string& 源代码) {
    std::string 格式化后 = 格式化代码(源代码);
    return 源代码 == 格式化后;
}

int main() {
    auto 参数 = 获取UTF8参数();
    int argc = (int)参数.size();

    if (argc < 2) {
        显示用法(参数[0]);
        return 1;
    }

    bool 检查模式 = false;
    int 输入文件索引 = 1;

    if (参数[1] == "--检查") {
        检查模式 = true;
        输入文件索引 = 2;
    }

    if (输入文件索引 >= argc) {
        显示用法(参数[0]);
        return 1;
    }

    std::string 输入文件 = 参数[输入文件索引];
    std::string 输出文件;

    if (检查模式) {
        输出文件 = 输入文件;
    } else if (输入文件索引 + 1 < argc) {
        输出文件 = 参数[输入文件索引 + 1];
    } else {
        输出文件 = 输入文件;
    }

    try {
        // 读取输入文件 - 使用宽字符路径处理中文文件名
        int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 输入文件.c_str(), (int)输入文件.size(), nullptr, 0);
        std::wstring 宽路径(宽长度, 0);
        MultiByteToWideChar(CP_UTF8, 0, 输入文件.c_str(), (int)输入文件.size(), &宽路径[0], 宽长度);

        FILE* 文件 = _wfopen(宽路径.c_str(), L"rb");
        if (!文件) {
            std::cerr << "错误: 无法打开文件 " << 输入文件 << std::endl;
            return 1;
        }

        fseek(文件, 0, SEEK_END);
        long 文件大小 = ftell(文件);
        fseek(文件, 0, SEEK_SET);

        std::string 源代码(文件大小, 0);
        fread(&源代码[0], 1, 文件大小, 文件);
        fclose(文件);

        if (检查模式) {
            // 检查模式
            if (检查格式(源代码)) {
                std::cout << 输入文件 << ": 格式正确" << std::endl;
                return 0;
            } else {
                std::cout << 输入文件 << ": 格式不正确" << std::endl;
                return 1;
            }
        } else {
            // 格式化模式
            std::string 格式化后 = 格式化代码(源代码);

            // 写入输出文件 - 使用宽字符路径处理中文文件名
            int 输出宽长度 = MultiByteToWideChar(CP_UTF8, 0, 输出文件.c_str(), (int)输出文件.size(), nullptr, 0);
            std::wstring 输出宽路径(输出宽长度, 0);
            MultiByteToWideChar(CP_UTF8, 0, 输出文件.c_str(), (int)输出文件.size(), &输出宽路径[0], 输出宽长度);

            FILE* 输出文件句柄 = _wfopen(输出宽路径.c_str(), L"wb");
            if (!输出文件句柄) {
                std::cerr << "错误: 无法写入文件 " << 输出文件 << std::endl;
                return 1;
            }

            fwrite(格式化后.c_str(), 1, 格式化后.size(), 输出文件句柄);
            fclose(输出文件句柄);

            std::cout << "格式化完成: " << 输出文件 << std::endl;
            return 0;
        }
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

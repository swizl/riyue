// 日月代码格式化工具
// 用法: 日月.exe --格式化 输入.心 [输出.心]

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

// 简单的代码格式化器
class 格式化器 {
private:
    std::string 源代码;
    size_t 位置;
    int 缩进级别;
    std::string 结果;
    bool 新行;

    char 当前字符() const {
        return 位置 < 源代码.size() ? 源代码[位置] : '\0';
    }

    void 前进() {
        if (位置 < 源代码.size()) 位置++;
    }

    void 跳过空白() {
        while (位置 < 源代码.size() && (源代码[位置] == ' ' || 源代码[位置] == '\t')) {
            前进();
        }
    }

    void 跳过注释() {
        if (位置 + 1 < 源代码.size() && 源代码[位置] == '/' && 源代码[位置 + 1] == '/') {
            while (位置 < 源代码.size() && 源代码[位置] != '\n') {
                结果 += 源代码[位置];
                前进();
            }
        }
    }

    void 添加缩进() {
        for (int i = 0; i < 缩进级别; i++) {
            结果 += "    ";
        }
    }

    void 添加新行() {
        结果 += "\n";
        新行 = true;
    }

    bool 是关键字(const std::string& 标识符) const {
        static std::vector<std::string> 关键字列表 = {
            "函数", "变量", "常量", "如果", "否则", "否则如果",
            "循环", "当", "做", "到", "中断", "继续", "返回",
            "匹配", "遍历", "导入", "结构体", "枚举", "打印",
            "真", "假", "空"
        };
        for (const auto& 关键字 : 关键字列表) {
            if (标识符 == 关键字) return true;
        }
        return false;
    }

public:
    格式化器(const std::string& 源) : 源代码(源), 位置(0), 缩进级别(0), 新行(true) {}

    std::string 格式化() {
        while (位置 < 源代码.size()) {
            char c = 当前字符();

            // 跳过空白
            if (c == ' ' || c == '\t') {
                if (!新行) {
                    结果 += ' ';
                }
                跳过空白();
                continue;
            }

            // 处理换行
            if (c == '\n') {
                // 避免多个连续空行
                if (结果.size() >= 2 && 结果[结果.size()-1] == '\n' && 结果[结果.size()-2] == '\n') {
                    // 跳过多余的空行
                    前进();
                    continue;
                }
                添加新行();
                前进();
                continue;
            }

            // 处理注释
            if (c == '/' && 位置 + 1 < 源代码.size() && 源代码[位置 + 1] == '/') {
                if (新行) 添加缩进();
                跳过注释();
                添加新行();
                continue;
            }

            // 处理左花括号
            if (c == '{') {
                // 移除前一个空格（如果有）
                if (!结果.empty() && 结果[结果.size()-1] == ' ') {
                    结果.pop_back();
                }
                结果 += ' ';
                结果 += '{';
                缩进级别++;
                添加新行();
                前进();
                continue;
            }

            // 处理右花括号
            if (c == '}') {
                缩进级别--;
                // 移除前一个空行（如果有）
                if (结果.size() >= 2 && 结果[结果.size()-1] == '\n' && 结果[结果.size()-2] == '\n') {
                    结果.pop_back();
                }
                添加缩进();
                结果 += '}';
                前进();
                // 检查后面是否跟着换行
                跳过空白();
                if (位置 < 源代码.size() && 源代码[位置] != '\n') {
                    添加新行();
                }
                continue;
            }

            // 处理分号
            if (c == ';') {
                结果 += ';';
                前进();
                跳过空白();
                if (位置 < 源代码.size() && 源代码[位置] != '\n' && 源代码[位置] != '}') {
                    添加新行();
                }
                continue;
            }

            // 在新行开始时添加缩进
            if (新行) {
                添加缩进();
                新行 = false;
            }

            // 处理字符串
            if (c == '"') {
                结果 += c;
                前进();
                while (位置 < 源代码.size() && 源代码[位置] != '"') {
                    if (源代码[位置] == '\\') {
                        结果 += 源代码[位置];
                        前进();
                    }
                    结果 += 源代码[位置];
                    前进();
                }
                if (位置 < 源代码.size()) {
                    结果 += 源代码[位置];
                    前进();
                }
                continue;
            }

            // 处理标识符和关键字
            if (isalpha(c) || c == '_' || (c & 0x80)) {
                std::string 标识符;
                while (位置 < 源代码.size() && (isalnum(源代码[位置]) || 源代码[位置] == '_' || (源代码[位置] & 0x80))) {
                    标识符 += 源代码[位置];
                    前进();
                }
                结果 += 标识符;
                continue;
            }

            // 处理数字
            if (isdigit(c)) {
                while (位置 < 源代码.size() && (isdigit(源代码[位置]) || 源代码[位置] == '.')) {
                    结果 += 源代码[位置];
                    前进();
                }
                continue;
            }

            // 处理其他字符
            结果 += c;
            前进();
        }

        // 移除末尾的多余空行
        while (结果.size() >= 2 && 结果[结果.size()-1] == '\n' && 结果[结果.size()-2] == '\n') {
            结果.pop_back();
        }

        return 结果;
    }
};

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif

    if (argc < 2) {
        std::cerr << "用法: 格式化 输入.心 [输出.心]" << std::endl;
        return 1;
    }

    std::string 输入文件 = argv[1];
    std::string 输出文件 = (argc > 2) ? argv[2] : 输入文件;

    // 读取输入文件
    std::ifstream 文件(输入文件, std::ios::binary);
    if (!文件.is_open()) {
        std::cerr << "无法打开文件: " << 输入文件 << std::endl;
        return 1;
    }
    std::string 源代码((std::istreambuf_iterator<char>(文件)), std::istreambuf_iterator<char>());
    文件.close();

    // 格式化代码
    格式化器 格式化(源代码);
    std::string 结果 = 格式化.格式化();

    // 写入输出文件
    std::ofstream 输出(输出文件, std::ios::binary);
    if (!输出.is_open()) {
        std::cerr << "无法创建文件: " << 输出文件 << std::endl;
        return 1;
    }
    输出 << 结果;
    输出.close();

    std::cout << "格式化完成: " << 输出文件 << std::endl;
    return 0;
}

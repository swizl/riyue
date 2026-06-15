#include "解析器生成器.h"
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#include <fstream>
#include <codecvt>

// 在Windows上创建支持UTF-8路径的ofstream
static std::ofstream 打开文件(const std::string& 路径) {
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 路径.c_str(), (int)路径.size(), nullptr, 0);
    std::vector<wchar_t> 宽路径(宽长度 + 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, 路径.c_str(), (int)路径.size(), 宽路径.data(), 宽长度);
    return std::ofstream(宽路径.data());
}
#else
static std::ofstream 打开文件(const std::string& 路径) {
    return std::ofstream(路径);
}
#endif

bool 解析器生成器::解析(const std::string& 语法文件) {
    std::ifstream 文件(语法文件);
    if (!文件.is_open()) {
        std::cerr << "无法打开语法文件: " << 语法文件 << std::endl;
        return false;
    }

    std::stringstream 缓冲;
    缓冲 << 文件.rdbuf();
    std::string 内容 = 缓冲.str();

    // 分离词法和语法定义
    size_t 语法开始 = 内容.find("# ============ 语法定义 ============");
    if (语法开始 == std::string::npos) {
        std::cerr << "语法文件格式错误：找不到语法定义部分" << std::endl;
        return false;
    }

    std::string 词法部分 = 内容.substr(0, 语法开始);
    std::string 语法部分 = 内容.substr(语法开始);

    解析词法定义(词法部分);
    解析语法定义(语法部分);

    return true;
}

void 解析器生成器::解析词法定义(const std::string& 内容) {
    std::istringstream 流(内容);
    std::string 行;

    while (std::getline(流, 行)) {
        // 去除首尾空白
        行.erase(0, 行.find_first_not_of(" \t\r"));
        行.erase(行.find_last_not_of(" \t\r") + 1);

        // 跳过空行和注释
        if (行.empty() || 行[0] == '#') continue;

        if (行.find("%skip") == 0) {
            // %skip 名称 模式
            std::istringstream 行流(行.substr(5));
            标记定义 标记;
            标记.跳过 = true;
            行流 >> 标记.名称;
            std::getline(行流, 标记.模式);
            标记.模式.erase(0, 标记.模式.find_first_not_of(" \t"));
            标记列表.push_back(标记);
        }
        else if (行.find("%token") == 0) {
            // %token 名称 模式
            std::istringstream 行流(行.substr(6));
            标记定义 标记;
            行流 >> 标记.名称;
            std::getline(行流, 标记.模式);
            标记.模式.erase(0, 标记.模式.find_first_not_of(" \t"));
            标记列表.push_back(标记);
        }
        else if (行.find("%keyword") == 0) {
            // %keyword 关键字1 关键字2 ...
            std::istringstream 行流(行.substr(8));
            std::string 关键字;
            while (行流 >> 关键字) {
                关键字集合.insert(关键字);
                标记定义 标记;
                标记.名称 = 关键字;
                标记.模式 = "'" + 关键字 + "'";
                标记.是关键字 = true;
                标记列表.push_back(标记);
            }
        }
    }
}

void 解析器生成器::解析语法定义(const std::string& 内容) {
    std::istringstream 流(内容);
    std::string 行;
    规则定义* 当前规则 = nullptr;

    while (std::getline(流, 行)) {
        // 去除首尾空白
        行.erase(0, 行.find_first_not_of(" \t\r"));
        行.erase(行.find_last_not_of(" \t\r") + 1);

        // 跳过空行和注释
        if (行.empty() || 行[0] == '#') continue;

        // 检查是否是规则定义（名称: 产生式）
        size_t 冒号位置 = 行.find(':');
        if (冒号位置 != std::string::npos && 冒号位置 > 0 &&
            行.find("'") == std::string::npos && 行.find("//") == std::string::npos) {
            // 新规则
            规则定义 新规则;
            新规则.名称 = 行.substr(0, 冒号位置);
            新规则.名称.erase(新规则.名称.find_last_not_of(" \t") + 1);
            规则列表.push_back(新规则);
            当前规则 = &规则列表.back();

            std::string 剩余 = 行.substr(冒号位置 + 1);
            剩余.erase(0, 剩余.find_first_not_of(" \t"));
            if (!剩余.empty() && 剩余 != ";") {
                产生式 新产生式;
                std::istringstream 元素流(剩余);
                std::string 元素;
                while (元素流 >> 元素) {
                    if (元素 == ";") break;
                    新产生式.元素.push_back(元素);
                }
                if (!新产生式.元素.empty()) {
                    当前规则->备选.push_back(新产生式);
                }
            }
        }
        else if (行[0] == '|' && 当前规则) {
            // 备选产生式
            std::string 剩余 = 行.substr(1);
            剩余.erase(0, 剩余.find_first_not_of(" \t"));
            if (!剩余.empty() && 剩余 != ";") {
                产生式 新产生式;
                std::istringstream 元素流(剩余);
                std::string 元素;
                while (元素流 >> 元素) {
                    if (元素 == ";") break;
                    新产生式.元素.push_back(元素);
                }
                if (!新产生式.元素.empty()) {
                    当前规则->备选.push_back(新产生式);
                }
            }
        }
        else if (当前规则 && !行.empty() && 行 != ";") {
            // 续行产生式
            产生式 新产生式;
            std::istringstream 元素流(行);
            std::string 元素;
            while (元素流 >> 元素) {
                if (元素 == ";") break;
                新产生式.元素.push_back(元素);
            }
            if (!新产生式.元素.empty()) {
                当前规则->备选.push_back(新产生式);
            }
        }
    }
}

void 解析器生成器::生成(const std::string& 输出目录) {
    std::filesystem::create_directories(输出目录);

    std::string 头文件路径 = 输出目录 + "/词法分析器.h";
    std::string 源文件路径 = 输出目录 + "/词法分析器.cpp";
    std::string 语法头文件路径 = 输出目录 + "/语法分析器.h";
    std::string 语法源文件路径 = 输出目录 + "/语法分析器.cpp";

    std::cout << "生成: " << 头文件路径 << std::endl;
    生成词法分析器头文件(输出目录);

    std::cout << "生成: " << 源文件路径 << std::endl;
    生成词法分析器源文件(输出目录);

    std::cout << "生成: " << 语法头文件路径 << std::endl;
    生成语法分析器头文件(输出目录);

    std::cout << "生成: " << 语法源文件路径 << std::endl;
    生成语法分析器源文件(输出目录);

    std::cout << "已生成词法/语法分析器到: " << 输出目录 << std::endl;
}

void 解析器生成器::生成词法分析器头文件(const std::string& 输出目录) {
    std::string 文件路径 = 输出目录 + "/词法分析器.h";
    auto 文件 = 打开文件(文件路径);
    if (!文件.is_open()) {
        std::cerr << "无法创建文件: " << 文件路径 << std::endl;
        return;
    }

    文件 << "#ifndef 词法分析器_H" << std::endl;
    文件 << "#define 词法分析器_H" << std::endl;
    文件 << std::endl;
    文件 << "#include <string>" << std::endl;
    文件 << "#include <cstdint>" << std::endl;
    文件 << std::endl;
    文件 << "enum class 标记类型 {" << std::endl;
    文件 << "    未知," << std::endl;
    文件 << "    文件结束," << std::endl;

    // 生成标记类型枚举
    std::set<std::string> 已生成;
    for (const auto& 标记 : 标记列表) {
        if (标记.跳过) continue;
        if (已生成.count(标记.名称)) continue;
        已生成.insert(标记.名称);
        文件 << "    " << 标记.名称 << "," << std::endl;
    }

    文件 << "};" << std::endl;
    文件 << std::endl;
    文件 << "struct 标记 {" << std::endl;
    文件 << "    标记类型 类型;" << std::endl;
    文件 << "    std::string 值;" << std::endl;
    文件 << "    int 行号;" << std::endl;
    文件 << "    标记(标记类型 t, std::string v, int ln) : 类型(t), 值(v), 行号(ln) {}" << std::endl;
    文件 << "};" << std::endl;
    文件 << std::endl;
    文件 << "std::string 标记类型转字符串(标记类型 类型);" << std::endl;
    文件 << std::endl;
    文件 << "class 词法分析器 {" << std::endl;
    文件 << "private:" << std::endl;
    文件 << "    std::string 源代码;" << std::endl;
    文件 << "    size_t 位置 = 0;" << std::endl;
    文件 << "    int 当前行 = 1;" << std::endl;
    文件 << std::endl;
    文件 << "    char 字节(size_t 偏移 = 0) const;" << std::endl;
    文件 << "    void 前进字节(size_t 步长);" << std::endl;
    文件 << "    bool 是空白字节(uint8_t 字节) const;" << std::endl;
    文件 << "    void 跳过空白和注释();" << std::endl;
    文件 << "    int 多字节长度(uint8_t 首字节) const;" << std::endl;
    文件 << "    std::string 读取多字节字符();" << std::endl;
    文件 << "    bool 是多字节首字节(uint8_t 字节) const { return (字节 & 0xC0) == 0xC0; }" << std::endl;
    文件 << std::endl;
    文件 << "public:" << std::endl;
    文件 << "    词法分析器(const std::string& 文件名);" << std::endl;
    文件 << "    bool 已到文件尾() const { return 位置 >= 源代码.size(); }" << std::endl;
    文件 << "    标记 下一个标记();" << std::endl;
    文件 << "};" << std::endl;
    文件 << std::endl;
    文件 << "#endif" << std::endl;
}

void 解析器生成器::生成词法分析器源文件(const std::string& 输出目录) {
    std::string 文件路径 = 输出目录 + "/词法分析器.cpp";
    auto 文件 = 打开文件(文件路径);
    if (!文件.is_open()) {
        std::cerr << "无法创建文件: " << 文件路径 << std::endl;
        return;
    }

    文件 << "#include \"词法分析器.h\"" << std::endl;
    文件 << "#include <fstream>" << std::endl;
    文件 << "#include <cctype>" << std::endl;
    文件 << "#include <unordered_map>" << std::endl;
    文件 << "#include <iostream>" << std::endl;
    文件 << "#include <vector>" << std::endl;
    文件 << "#ifdef _WIN32" << std::endl;
    文件 << "#include <windows.h>" << std::endl;
    文件 << "#endif" << std::endl;
    文件 << std::endl;

    // 关键字映射表
    文件 << "static std::unordered_map<std::string, 标记类型> 关键字 = {" << std::endl;
    bool 第一个 = true;
    for (const auto& 关键字 : 关键字集合) {
        if (!第一个) 文件 << "," << std::endl;
        文件 << "    {\"" << 关键字 << "\", 标记类型::" << 关键字 << "}";
        第一个 = false;
    }
    文件 << std::endl << "};" << std::endl;
    文件 << std::endl;

    // 标记类型转字符串
    文件 << "std::string 标记类型转字符串(标记类型 类型) {" << std::endl;
    文件 << "    switch (类型) {" << std::endl;
    文件 << "        case 标记类型::文件结束: return \"文件结束\";" << std::endl;
    std::set<std::string> 已生成;
    for (const auto& 标记 : 标记列表) {
        if (标记.跳过) continue;
        if (已生成.count(标记.名称)) continue;
        已生成.insert(标记.名称);
        文件 << "        case 标记类型::" << 标记.名称 << ": return \"" << 标记.名称 << "\";" << std::endl;
    }
    文件 << "        default: return \"未知\";" << std::endl;
    文件 << "    }" << std::endl;
    文件 << "}" << std::endl;
    文件 << std::endl;

    // 词法分析器实现
    文件 << "#ifdef _WIN32" << std::endl;
    文件 << "词法分析器::词法分析器(const std::string& 文件名) : 位置(0), 当前行(1) {" << std::endl;
    文件 << "    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 文件名.c_str(), (int)文件名.size(), nullptr, 0);" << std::endl;
    文件 << "    std::vector<wchar_t> 宽文件名(宽长度 + 1, 0);" << std::endl;
    文件 << "    MultiByteToWideChar(CP_UTF8, 0, 文件名.c_str(), (int)文件名.size(), 宽文件名.data(), 宽长度);" << std::endl;
    文件 << "    std::ifstream 文件(宽文件名.data(), std::ios::binary);" << std::endl;
    文件 << "#else" << std::endl;
    文件 << "词法分析器::词法分析器(const std::string& 文件名) : 位置(0), 当前行(1) {" << std::endl;
    文件 << "    std::ifstream 文件(文件名, std::ios::binary);" << std::endl;
    文件 << "#endif" << std::endl;
    文件 << "    if (!文件.is_open()) {" << std::endl;
    文件 << "        std::cerr << \"无法打开文件: \" << 文件名 << std::endl;" << std::endl;
    文件 << "        exit(1);" << std::endl;
    文件 << "    }" << std::endl;
    文件 << "    源代码.assign((std::istreambuf_iterator<char>(文件)), std::istreambuf_iterator<char>());" << std::endl;
    文件 << "}" << std::endl;
    文件 << std::endl;

    // 基本辅助函数
    文件 << "char 词法分析器::字节(size_t 偏移) const {" << std::endl;
    文件 << "    return (位置 + 偏移 < 源代码.size()) ? 源代码[位置 + 偏移] : '\\0';" << std::endl;
    文件 << "}" << std::endl;
    文件 << std::endl;

    文件 << "void 词法分析器::前进字节(size_t 步长) {" << std::endl;
    文件 << "    for (size_t i = 0; i < 步长; ++i) {" << std::endl;
    文件 << "        if (已到文件尾()) break;" << std::endl;
    文件 << "        if (字节() == '\\n') 当前行++;" << std::endl;
    文件 << "        位置++;" << std::endl;
    文件 << "    }" << std::endl;
    文件 << "}" << std::endl;
    文件 << std::endl;

    文件 << "bool 词法分析器::是空白字节(uint8_t 字节) const {" << std::endl;
    文件 << "    return isspace(static_cast<unsigned char>(字节)) != 0;" << std::endl;
    文件 << "}" << std::endl;
    文件 << std::endl;

    文件 << "int 词法分析器::多字节长度(uint8_t 首字节) const {" << std::endl;
    文件 << "    if ((首字节 & 0x80) == 0x00) return 1;" << std::endl;
    文件 << "    if ((首字节 & 0xE0) == 0xC0) return 2;" << std::endl;
    文件 << "    if ((首字节 & 0xF0) == 0xE0) return 3;" << std::endl;
    文件 << "    if ((首字节 & 0xF8) == 0xF0) return 4;" << std::endl;
    文件 << "    return 1;" << std::endl;
    文件 << "}" << std::endl;
    文件 << std::endl;

    文件 << "std::string 词法分析器::读取多字节字符() {" << std::endl;
    文件 << "    if (已到文件尾()) return \"\";" << std::endl;
    文件 << "    uint8_t 首字节 = static_cast<uint8_t>(字节());" << std::endl;
    文件 << "    int 长度 = 多字节长度(首字节);" << std::endl;
    文件 << "    if (位置 + 长度 > 源代码.size()) 长度 = 源代码.size() - 位置;" << std::endl;
    文件 << "    std::string 字符(源代码.substr(位置, 长度));" << std::endl;
    文件 << "    前进字节(长度);" << std::endl;
    文件 << "    return 字符;" << std::endl;
    文件 << "}" << std::endl;
    文件 << std::endl;

    // 跳过空白和注释
    文件 << "void 词法分析器::跳过空白和注释() {" << std::endl;
    文件 << "    while (!已到文件尾()) {" << std::endl;
    文件 << "        uint8_t 当前字节 = static_cast<uint8_t>(字节());" << std::endl;
    文件 << "        if (是空白字节(当前字节)) {" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "        } else if (当前字节 == '/' && 位置 + 1 < 源代码.size() && 字节(1) == '/') {" << std::endl;
    文件 << "            while (!已到文件尾() && 字节() != '\\n') 前进字节(1);" << std::endl;
    文件 << "        } else if (当前字节 == '/' && 位置 + 1 < 源代码.size() && 字节(1) == '*') {" << std::endl;
    文件 << "            前进字节(2);" << std::endl;
    文件 << "            while (!已到文件尾()) {" << std::endl;
    文件 << "                if (字节() == '*' && 位置 + 1 < 源代码.size() && 字节(1) == '/') {" << std::endl;
    文件 << "                    前进字节(2);" << std::endl;
    文件 << "                    break;" << std::endl;
    文件 << "                }" << std::endl;
    文件 << "                前进字节(1);" << std::endl;
    文件 << "            }" << std::endl;
    文件 << "        } else {" << std::endl;
    文件 << "            break;" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "    }" << std::endl;
    文件 << "}" << std::endl;
    文件 << std::endl;

    // 下一个标记
    文件 << "标记 词法分析器::下一个标记() {" << std::endl;
    文件 << "    跳过空白和注释();" << std::endl;
    文件 << "    if (已到文件尾()) return 标记(标记类型::文件结束, \"\", 当前行);" << std::endl;
    文件 << std::endl;
    文件 << "    uint8_t 当前字节 = static_cast<uint8_t>(字节());" << std::endl;
    文件 << std::endl;

    // 整数
    文件 << "    // 识别整数" << std::endl;
    文件 << "    if (isdigit(static_cast<unsigned char>(当前字节))) {" << std::endl;
    文件 << "        std::string 数字串;" << std::endl;
    文件 << "        while (isdigit(static_cast<unsigned char>(字节()))) {" << std::endl;
    文件 << "            数字串 += 字节();" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        return 标记(标记类型::整数, 数字串, 当前行);" << std::endl;
    文件 << "    }" << std::endl;
    文件 << std::endl;

    // 字符串
    文件 << "    // 识别字符串" << std::endl;
    文件 << "    if (当前字节 == '\"') {" << std::endl;
    文件 << "        前进字节(1);" << std::endl;
    文件 << "        std::string 内容;" << std::endl;
    文件 << "        while (!已到文件尾() && static_cast<uint8_t>(字节()) != '\"') {" << std::endl;
    文件 << "            if (字节() == '\\\\') {" << std::endl;
    文件 << "                前进字节(1);" << std::endl;
    文件 << "                if (已到文件尾()) break;" << std::endl;
    文件 << "                char 转义 = 字节();" << std::endl;
    文件 << "                switch (转义) {" << std::endl;
    文件 << "                    case 'n': 内容 += '\\n'; break;" << std::endl;
    文件 << "                    case 't': 内容 += '\\t'; break;" << std::endl;
    文件 << "                    case '\\\\': 内容 += '\\\\'; break;" << std::endl;
    文件 << "                    case '\"': 内容 += '\"'; break;" << std::endl;
    文件 << "                    default: 内容 += '\\\\'; 内容 += 转义; break;" << std::endl;
    文件 << "                }" << std::endl;
    文件 << "                前进字节(1);" << std::endl;
    文件 << "            } else {" << std::endl;
    文件 << "                内容 += 读取多字节字符();" << std::endl;
    文件 << "            }" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        if (已到文件尾()) throw std::runtime_error(\"未闭合的字符串（行 \" + std::to_string(当前行) + \"）\");" << std::endl;
    文件 << "        前进字节(1);" << std::endl;
    文件 << "        return 标记(标记类型::字符串, 内容, 当前行);" << std::endl;
    文件 << "    }" << std::endl;
    文件 << std::endl;

    // 标识符和关键字
    文件 << "    // 识别标识符和关键字" << std::endl;
    文件 << "    bool 是首字符 = (当前字节 & 0x80) ? (多字节长度(当前字节) >= 2) :" << std::endl;
    文件 << "                    (isalpha(当前字节) || 当前字节 == '_');" << std::endl;
    文件 << "    if (是首字符) {" << std::endl;
    文件 << "        std::string 标识符;" << std::endl;
    文件 << "        标识符 += 读取多字节字符();" << std::endl;
    文件 << "        while (!已到文件尾()) {" << std::endl;
    文件 << "            uint8_t 后续字节 = static_cast<uint8_t>(字节());" << std::endl;
    文件 << "            bool 是后续字符 = (后续字节 & 0x80) ? (多字节长度(后续字节) >= 2) :" << std::endl;
    文件 << "                              (isalnum(后续字节) || 后续字节 == '_');" << std::endl;
    文件 << "            if (!是后续字符) break;" << std::endl;
    文件 << "            标识符 += 读取多字节字符();" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        if (关键字.count(标识符)) return 标记(关键字[标识符], 标识符, 当前行);" << std::endl;
    文件 << "        return 标记(标记类型::标识符, 标识符, 当前行);" << std::endl;
    文件 << "    }" << std::endl;
    文件 << std::endl;

    // 运算符和分隔符
    文件 << "    // 识别运算符和分隔符" << std::endl;
    文件 << "    switch (当前字节) {" << std::endl;
    文件 << "        case '(': 前进字节(1); return 标记(标记类型::左括号, \"(\", 当前行);" << std::endl;
    文件 << "        case ')': 前进字节(1); return 标记(标记类型::右括号, \")\", 当前行);" << std::endl;
    文件 << "        case '{': 前进字节(1); return 标记(标记类型::左花括号, \"{\", 当前行);" << std::endl;
    文件 << "        case '}': 前进字节(1); return 标记(标记类型::右花括号, \"}\", 当前行);" << std::endl;
    文件 << "        case '[': 前进字节(1); return 标记(标记类型::左方括号, \"[\", 当前行);" << std::endl;
    文件 << "        case ']': 前进字节(1); return 标记(标记类型::右方括号, \"]\", 当前行);" << std::endl;
    文件 << "        case ';': 前进字节(1); return 标记(标记类型::分号, \";\", 当前行);" << std::endl;
    文件 << "        case ':': 前进字节(1); return 标记(标记类型::冒号, \":\", 当前行);" << std::endl;
    文件 << "        case ',': 前进字节(1); return 标记(标记类型::逗号, \",\", 当前行);" << std::endl;
    文件 << "        case '%': 前进字节(1); return 标记(标记类型::百分号, \"%\", 当前行);" << std::endl;
    文件 << "        case '+': {" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "            if (字节() == '=') { 前进字节(1); return 标记(标记类型::加等于, \"+=\", 当前行); }" << std::endl;
    文件 << "            return 标记(标记类型::加号, \"+\", 当前行);" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        case '-': {" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "            if (字节() == '=') { 前进字节(1); return 标记(标记类型::减等于, \"-=\", 当前行); }" << std::endl;
    文件 << "            return 标记(标记类型::减号, \"-\", 当前行);" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        case '*': {" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "            if (字节() == '=') { 前进字节(1); return 标记(标记类型::乘等于, \"*=\", 当前行); }" << std::endl;
    文件 << "            return 标记(标记类型::乘号, \"*\", 当前行);" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        case '/': {" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "            if (字节() == '=') { 前进字节(1); return 标记(标记类型::除等于, \"/=\", 当前行); }" << std::endl;
    文件 << "            return 标记(标记类型::除号, \"/\", 当前行);" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        case '!': {" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "            if (字节() == '=') { 前进字节(1); return 标记(标记类型::感叹号等于, \"!=\", 当前行); }" << std::endl;
    文件 << "            return 标记(标记类型::感叹号, \"!\", 当前行);" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        case '<': {" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "            if (字节() == '=') { 前进字节(1); return 标记(标记类型::小于等于, \"<=\", 当前行); }" << std::endl;
    文件 << "            return 标记(标记类型::小于, \"<\", 当前行);" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        case '>': {" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "            if (字节() == '=') { 前进字节(1); return 标记(标记类型::大于等于, \">=\", 当前行); }" << std::endl;
    文件 << "            return 标记(标记类型::大于, \">\", 当前行);" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        case '&': {" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "            if (字节() == '&') { 前进字节(1); return 标记(标记类型::与与, \"&&\", 当前行); }" << std::endl;
    文件 << "            return 标记(标记类型::未知, \"&\", 当前行);" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        case '|': {" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "            if (字节() == '|') { 前进字节(1); return 标记(标记类型::或或, \"||\", 当前行); }" << std::endl;
    文件 << "            return 标记(标记类型::未知, \"|\", 当前行);" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        case '?': {" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "            if (字节() == '=') { 前进字节(1); return 标记(标记类型::等于等于, \"?=\", 当前行); }" << std::endl;
    文件 << "            return 标记(标记类型::未知, \"?\", 当前行);" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "        case '=': 前进字节(1); return 标记(标记类型::等于, \"=\", 当前行);" << std::endl;
    文件 << "        default: {" << std::endl;
    文件 << "            std::string 未知符号(1, static_cast<char>(当前字节));" << std::endl;
    文件 << "            前进字节(1);" << std::endl;
    文件 << "            return 标记(标记类型::未知, 未知符号, 当前行);" << std::endl;
    文件 << "        }" << std::endl;
    文件 << "    }" << std::endl;
    文件 << "}" << std::endl;
}

void 解析器生成器::生成语法分析器头文件(const std::string& 输出目录) {
    std::string 文件路径 = 输出目录 + "/语法分析器.h";
    auto 文件 = 打开文件(文件路径);
    if (!文件.is_open()) {
        std::cerr << "无法创建文件: " << 文件路径 << std::endl;
        return;
    }

    文件 << "#pragma once" << std::endl;
    文件 << "#include \"词法分析器.h\"" << std::endl;
    文件 << "#include \"抽象语法树.h\"" << std::endl;
    文件 << "#include <memory>" << std::endl;
    文件 << std::endl;
    文件 << "class 语法分析器 {" << std::endl;
    文件 << "private:" << std::endl;
    文件 << "    词法分析器 _词法分析器;" << std::endl;
    文件 << "    标记 _当前标记;" << std::endl;
    文件 << std::endl;
    文件 << "    void 前进();" << std::endl;
    文件 << "    void 期望类型(标记类型 类型, const std::string& 信息);" << std::endl;
    文件 << "    void 期望(标记类型 类型, const std::string& 信息);" << std::endl;
    文件 << std::endl;

    // 为每个语法规则生成解析函数声明
    for (const auto& 规则 : 规则列表) {
        文件 << "    std::unique_ptr<" << 规则.名称 << "> 解析" << 规则.名称 << "();" << std::endl;
    }

    文件 << std::endl;
    文件 << "public:" << std::endl;
    文件 << "    语法分析器(const std::string& 文件名) : _词法分析器(文件名), _当前标记(标记类型::未知, \"\", 0) {" << std::endl;
    文件 << "        前进();" << std::endl;
    文件 << "    }" << std::endl;
    文件 << std::endl;
    文件 << "    std::unique_ptr<程序> 解析程序();" << std::endl;
    文件 << "    const 标记& 当前标记() const { return _当前标记; }" << std::endl;
    文件 << "};" << std::endl;
}

void 解析器生成器::生成语法分析器源文件(const std::string& 输出目录) {
    std::string 文件路径 = 输出目录 + "/语法分析器.cpp";
    auto 文件 = 打开文件(文件路径);
    if (!文件.is_open()) {
        std::cerr << "无法创建文件: " << 文件路径 << std::endl;
        return;
    }

    文件 << "#include \"语法分析器.h\"" << std::endl;
    文件 << "#include <stdexcept>" << std::endl;
    文件 << std::endl;

    // 基本函数
    文件 << "void 语法分析器::前进() {" << std::endl;
    文件 << "    _当前标记 = _词法分析器.下一个标记();" << std::endl;
    文件 << "}" << std::endl;
    文件 << std::endl;

    文件 << "void 语法分析器::期望类型(标记类型 类型, const std::string& 信息) {" << std::endl;
    文件 << "    if (_当前标记.类型 != 类型) {" << std::endl;
    文件 << "        throw std::runtime_error(信息 + \"（行 \" + std::to_string(_当前标记.行号) + \"，得到 '\" + _当前标记.值 + \"'）\");" << std::endl;
    文件 << "    }" << std::endl;
    文件 << "}" << std::endl;
    文件 << std::endl;

    文件 << "void 语法分析器::期望(标记类型 类型, const std::string& 信息) {" << std::endl;
    文件 << "    期望类型(类型, 信息);" << std::endl;
    文件 << "    前进();" << std::endl;
    文件 << "}" << std::endl;
    文件 << std::endl;

    // 为每个规则生成解析函数
    for (const auto& 规则 : 规则列表) {
        文件 << "std::unique_ptr<" << 规则.名称 << "> 语法分析器::解析" << 规则.名称 << "() {" << std::endl;
        文件 << "    // TODO: 实现 " << 规则.名称 << " 的解析逻辑" << std::endl;
        文件 << "    return nullptr;" << std::endl;
        文件 << "}" << std::endl;
        文件 << std::endl;
    }
}

void 解析器生成器::打印信息() {
    std::cout << "=== 词法标记 ===" << std::endl;
    for (const auto& 标记 : 标记列表) {
        std::cout << "  " << 标记.名称 << ": " << 标记.模式;
        if (标记.跳过) std::cout << " (跳过)";
        if (标记.是关键字) std::cout << " (关键字)";
        std::cout << std::endl;
    }

    std::cout << std::endl << "=== 语法规则 ===" << std::endl;
    for (const auto& 规则 : 规则列表) {
        std::cout << "  " << 规则.名称 << ":" << std::endl;
        for (const auto& 备选 : 规则.备选) {
            std::cout << "    |";
            for (const auto& 元素 : 备选.元素) {
                std::cout << " " << 元素;
            }
            std::cout << std::endl;
        }
    }
}

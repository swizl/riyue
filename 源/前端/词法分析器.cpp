#include "词法分析器.h"
#include "公共.h"
#include <fstream>
#include <cctype>
#include <iomanip>
#include <vector>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

// 由 工具/gen_hash.py 自动生成的完美hash查找函数
#include "关键字哈希.h"

std::string 标记类型转字符串(标记类型 类型) {
    switch (类型) {
        case 标记类型::文件结束: return "文件结束";
        case 标记类型::整数: return "整数";
        case 标记类型::浮点: return "浮点";
        case 标记类型::字符串: return "字符串";
        case 标记类型::字符串插值: return "字符串插值";
        case 标记类型::标识符: return "标识符";
        case 标记类型::加号: return "加号";
        case 标记类型::减号: return "减号";
        case 标记类型::乘号: return "乘号";
        case 标记类型::除号: return "除号";
        case 标记类型::百分号: return "百分号";
        case 标记类型::加等于: return "加等于";
        case 标记类型::减等于: return "减等于";
        case 标记类型::乘等于: return "乘等于";
        case 标记类型::除等于: return "除等于";
        case 标记类型::等于: return "等于";
        case 标记类型::等于等于: return "等于等于";
        case 标记类型::感叹号等于: return "感叹号等于";
        case 标记类型::小于: return "小于";
        case 标记类型::大于: return "大于";
        case 标记类型::小于等于: return "小于等于";
        case 标记类型::大于等于: return "大于等于";
        case 标记类型::与与: return "与与";
        case 标记类型::或或: return "或或";
        case 标记类型::感叹号: return "感叹号";
        case 标记类型::左括号: return "左括号";
        case 标记类型::右括号: return "右括号";
        case 标记类型::左花括号: return "左花括号";
        case 标记类型::右花括号: return "右花括号";
        case 标记类型::左方括号: return "左方括号";
        case 标记类型::右方括号: return "右方括号";
        case 标记类型::分号: return "分号";
        case 标记类型::逗号: return "逗号";
        case 标记类型::冒号: return "冒号";
        case 标记类型::箭头: return "箭头";
        case 标记类型::函数: return "函数";
        case 标记类型::空: return "空";
        case 标记类型::如果: return "如果";
        case 标记类型::否则: return "否则";
        case 标记类型::否则如果: return "否则如果";
        case 标记类型::循环: return "循环";
        case 标记类型::当: return "当";
        case 标记类型::中断: return "中断";
        case 标记类型::继续: return "继续";
        case 标记类型::变量: return "变量";
        case 标记类型::常量: return "常量";
        case 标记类型::打印: return "打印";
        case 标记类型::返回: return "返回";
        case 标记类型::匹配: return "匹配";
        case 标记类型::遍历: return "遍历";
        case 标记类型::导入: return "导入";
        case 标记类型::整数类型: return "整数类型";
        case 标记类型::浮点类型: return "浮点类型";
        case 标记类型::布尔类型: return "布尔类型";
        case 标记类型::字符串类型: return "字符串类型";
        case 标记类型::结构体: return "结构体";
        case 标记类型::枚举: return "枚举";
        case 标记类型::真: return "真";
        case 标记类型::假: return "假";
        case 标记类型::问号: return "问号";
        case 标记类型::问问: return "问问";
        case 标记类型::刀乐: return "刀乐";
        case 标记类型::方法: return "方法";
        case 标记类型::类型: return "类型";
        case 标记类型::入: return "入";
        case 标记类型::尝试: return "尝试";
        case 标记类型::捕获: return "捕获";
        case 标记类型::抛出: return "抛出";
        case 标记类型::最终: return "最终";
        case 标记类型::协程: return "协程";
        case 标记类型::让出: return "让出";
        default: return "未知";
    }
}

词法分析器::词法分析器(const std::string& 文件名) : 位置(0), 当前行(1), 当前列(1) {
#ifdef _WIN32
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 文件名.c_str(), (int)文件名.size(), nullptr, 0);
    std::vector<wchar_t> 宽文件名(宽长度 + 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, 文件名.c_str(), (int)文件名.size(), 宽文件名.data(), 宽长度);
    std::ifstream 文件(宽文件名.data(), std::ios::binary);
#else
    std::ifstream 文件(文件名, std::ios::binary);
#endif
    if (!文件.is_open()) {
        std::cerr << "无法打开文件: " << 文件名 << std::endl;
        exit(1);
    }
    源代码.assign((std::istreambuf_iterator<char>(文件)), 
                   std::istreambuf_iterator<char>());
    调试打印("[词法分析器][初始化] 读取文件完成，总字节数: " << 源代码.size());
}

char 词法分析器::字节(size_t 偏移) const {
    return (位置 + 偏移 < 源代码.size()) ? 源代码[位置 + 偏移] : '\0';
}

void 词法分析器::前进字节(size_t 步长) {
    for (size_t i = 0; i < 步长; ++i) {
        if (已到文件尾()) break;
        if (字节() == '\n') {
            当前行++;
            当前列 = 1;
        } else {
            当前列++;
        }
        位置++;
    }
    调试打印("[词法分析器][位置] 前进后: " << 位置 << "（行: " << 当前行 << "）");
}

bool 词法分析器::是空白字节(uint8_t 字节) const {
    return isspace(static_cast<unsigned char>(字节)) != 0;
}

bool 词法分析器::是全角空格() const {
    if (位置 + 2 >= 源代码.size()) return false;
    uint8_t b1 = static_cast<uint8_t>(字节(0));
    uint8_t b2 = static_cast<uint8_t>(字节(1));
    uint8_t b3 = static_cast<uint8_t>(字节(2));
    return (b1 == 0xE3 && b2 == 0x80 && b3 == 0x80);
}

void 词法分析器::跳过空白和注释() {
    while (!已到文件尾()) {
        uint8_t 当前字节 = static_cast<uint8_t>(字节());
        if (是空白字节(当前字节)) {
            前进字节(1);
        } else if (是全角空格()) {
            前进字节(3);
        } else if (当前字节 == '/' && static_cast<uint8_t>(字节(1)) == '/') {
            while (!已到文件尾() && 字节() != '\n') 前进字节(1);
        } else if (当前字节 == '/' && static_cast<uint8_t>(字节(1)) == '*') {
            前进字节(2);
            while (!已到文件尾()) {
                if (字节() == '*' && 字节(1) == '/') {
                    前进字节(2);
                    break;
                }
                前进字节(1);
            }
        } else {
            break;
        }
    }
}

int 词法分析器::多字节长度(uint8_t 首字节) const {
    if ((首字节 & 0x80) == 0x00) return 1;
    if ((首字节 & 0xE0) == 0xC0) return 2;
    if ((首字节 & 0xF0) == 0xE0) return 3;  // 中文属于此类（3字节）
    if ((首字节 & 0xF8) == 0xF0) return 4;
    return 1;
}

bool 词法分析器::验证后续字节(size_t 总长度) const {
    for (size_t i = 1; i < 总长度; ++i) {
        uint8_t 后续字节 = static_cast<uint8_t>(字节(i));
        if ((后续字节 & 0xC0) != 0x80) return false;
    }
    return true;
}

std::string 词法分析器::读取多字节字符() {
    if (已到文件尾()) return "";

    uint8_t 首字节 = static_cast<uint8_t>(字节());
    int 预期长度 = 多字节长度(首字节);

    // 强制处理中文常见范围（U+4E00~U+9FFF）的3字节编码
    if (首字节 >= 0xE4 && 首字节 <= 0xE9) {
        预期长度 = 3;
        调试打印("[词法分析器][UTF-8] 检测到中文范围首字节，强制3字节解析");
    }

    int 实际长度 = 预期长度;
    if (位置 + 预期长度 > 源代码.size()) {
        实际长度 = 源代码.size() - 位置;
        调试打印("[词法分析器][UTF-8] 不完整字符，预期: " << 预期长度 << "，实际: " << 实际长度);
    }

    if (实际长度 == 预期长度 && !验证后续字节(预期长度)) {
        实际长度 = 1;
        调试打印("[词法分析器][UTF-8] 无效序列，仅取首字节");
    }

    std::string 字符(源代码.substr(位置, 实际长度));
    调试打印("[词法分析器][UTF-8] 读取字符: '" << 字符 << "'（长度: " << 实际长度 << "）");
    前进字节(实际长度);
    return 字符;
}

std::string 词法分析器::解析字符串内容(bool 是插值模式) {
    std::string 内容;
    while (!已到文件尾() && static_cast<uint8_t>(字节()) != '"') {
        if (字节() == '\\') {
            前进字节(1);
            if (已到文件尾()) break;
            char 转义 = 字节();
            switch (转义) {
                case 'n': 内容 += '\n'; break;
                case 't': 内容 += '\t'; break;
                case '\\': 内容 += '\\'; break;
                case '"': 内容 += '"'; break;
                case '{': if (是插值模式) { 内容 += '\x01'; break; } // f-string转义{
                case '0': if (!是插值模式) { 内容 += '\0'; break; } // 普通字符串\0
                default: 内容 += '\\'; 内容 += 转义; break;
            }
            前进字节(1);
        } else {
            内容 += 读取多字节字符();
        }
    }
    return 内容;
}

标记 词法分析器::下一个标记() {
    跳过空白和注释();
    if (已到文件尾()) {
        return 标记(标记类型::文件结束, "", 当前行, 当前列);
    }

    int 标记起始列 = 当前列;
    uint8_t 当前字节 = static_cast<uint8_t>(字节());

    // 识别字符串字面量（支持 f"..." 字符串插值）
    if (当前字节 == 'f' && 位置 + 1 < 源代码.size() && static_cast<uint8_t>(字节(1)) == '"') {
        前进字节(1); // 跳过 'f'
        前进字节(1); // 跳过 '"'
        std::string 内容 = 解析字符串内容(true);
        if (已到文件尾()) {
            throw std::runtime_error("未闭合的字符串插值（行 " + std::to_string(当前行) + "）");
        }
        前进字节(1); // 跳过闭合 '"'
        调试打印("[词法分析器][标记] 字符串插值: '" << 内容 << "'");
        return 标记(标记类型::字符串插值, 内容, 当前行, 标记起始列);
    }

    // 识别字符字面量
    if (当前字节 == '\'') {
        前进字节(1);
        if (已到文件尾()) throw std::runtime_error("未闭合的字符字面量（行 " + std::to_string(当前行) + "）");
        int 字符值 = 0;
        if (字节() == '\\') {
            前进字节(1);
            if (已到文件尾()) throw std::runtime_error("未闭合的转义字符（行 " + std::to_string(当前行) + "）");
            char 转义 = 字节();
            switch (转义) {
                case 'n': 字符值 = '\n'; break;
                case 't': 字符值 = '\t'; break;
                case '\\': 字符值 = '\\'; break;
                case '\'': 字符值 = '\''; break;
                case '0': 字符值 = '\0'; break;
                default: 字符值 = static_cast<int>(转义); break;
            }
            前进字节(1);
        } else {
            // 读取 UTF-8 字符，计算 Unicode 码点
            std::string 字符 = 读取多字节字符();
            if (字符.size() == 1) {
                字符值 = static_cast<unsigned char>(字符[0]);
            } else if (字符.size() == 2) {
                字符值 = ((字符[0] & 0x1F) << 6) | (字符[1] & 0x3F);
            } else if (字符.size() == 3) {
                字符值 = ((字符[0] & 0x0F) << 12) | ((字符[1] & 0x3F) << 6) | (字符[2] & 0x3F);
            } else if (字符.size() == 4) {
                字符值 = ((字符[0] & 0x07) << 18) | ((字符[1] & 0x3F) << 12) | ((字符[2] & 0x3F) << 6) | (字符[3] & 0x3F);
            }
        }
        if (已到文件尾() || 字节() != '\'') {
            throw std::runtime_error("未闭合的字符字面量（行 " + std::to_string(当前行) + "）");
        }
        前进字节(1); // 跳过闭合 '
        调试打印("[词法分析器][标记] 字符: " << 字符值);
        return 标记(标记类型::字符, std::to_string(字符值), 当前行, 标记起始列);
    }

    // 识别字符串字面量
    if (当前字节 == '"') {
        前进字节(1);
        std::string 内容 = 解析字符串内容(false);
        if (已到文件尾()) {
            throw std::runtime_error("未闭合的字符串字面量（行 " + std::to_string(当前行) + "）");
        }
        前进字节(1);
        调试打印("[词法分析器][标记] 字符串: '" << 内容 << "'");
        return 标记(标记类型::字符串, 内容, 当前行, 标记起始列);
    }

    // 识别整数和浮点数
    if (isdigit(static_cast<unsigned char>(当前字节))) {
        std::string 数字串;
        bool 是浮点 = false;
        while (isdigit(static_cast<unsigned char>(字节()))) {
            数字串 += 字节();
            前进字节(1);
        }
        if (字节() == '.' && isdigit(static_cast<unsigned char>(字节(1)))) {
            是浮点 = true;
            数字串 += 字节();
            前进字节(1);
            while (isdigit(static_cast<unsigned char>(字节()))) {
                数字串 += 字节();
                前进字节(1);
            }
        }
        if (是浮点) {
            调试打印("[词法分析器][标记] 浮点: '" << 数字串 << "'");
            return 标记(标记类型::浮点, 数字串, 当前行, 标记起始列);
        }
        调试打印("[词法分析器][标记] 整数: '" << 数字串 << "'");
        return 标记(标记类型::整数, 数字串, 当前行, 标记起始列);
    }

    // 识别标识符（重点处理“计数”）
    bool 是首字符 = false;
    if ((当前字节 & 0x80) == 0) {
        是首字符 = isalpha(static_cast<unsigned char>(当前字节)) || (当前字节 == '_');
    } else {
        是首字符 = (多字节长度(当前字节) >= 2);  // 多字节字符作为首字符
    }

    if (是首字符) {
        std::string 标识符;
        标识符 += 读取多字节字符();
        调试打印("[词法分析器][标识符] 首片段: '" << 标识符 << "'");

        while (!已到文件尾()) {
            uint8_t 后续字节 = static_cast<uint8_t>(字节());
            bool 是后续字符 = false;

            if ((后续字节 & 0x80) == 0) {
                是后续字符 = isalnum(static_cast<unsigned char>(后续字节)) || (后续字节 == '_');
            } else {
                是后续字符 = (多字节长度(后续字节) >= 2);  // 允许多字节后续字符
            }

            if (!是后续字符) break;

            std::string 后续片段 = 读取多字节字符();
            标识符 += 后续片段;
            调试打印("[词法分析器][标识符] 拼接后: '" << 标识符 << "'");
        }

        // 重点追踪“计数”标识符
        if (标识符 == "计数") {
            调试打印("[词法分析器][标记] 检测到目标标识符: '计数'");
        }

        标记类型 关键字类型 = 完美hash查找关键字(标识符);
        调试打印("[词法分析器][标记] 标识符: '" << 标识符 << "'（是否关键字: " << (关键字类型 != 标记类型::未知 ? "是" : "否") << "）");
        if (关键字类型 != 标记类型::未知) {
            return 标记(关键字类型, 标识符, 当前行, 标记起始列);
        } else {
            return 标记(标记类型::标识符, 标识符, 当前行, 标记起始列);
        }
    }

    // 识别符号
    switch (当前字节) {
        case '(': 前进字节(1); return 标记(标记类型::左括号, "(", 当前行, 标记起始列);
        case ')': 前进字节(1); return 标记(标记类型::右括号, ")", 当前行, 标记起始列);
        case '{': 前进字节(1); return 标记(标记类型::左花括号, "{", 当前行, 标记起始列);
        case '}': 前进字节(1); return 标记(标记类型::右花括号, "}", 当前行, 标记起始列);
        case '[': 前进字节(1); return 标记(标记类型::左方括号, "[", 当前行, 标记起始列);
        case ']': 前进字节(1); return 标记(标记类型::右方括号, "]", 当前行, 标记起始列);
        case ';': 前进字节(1); return 标记(标记类型::分号, ";", 当前行, 标记起始列);
        case ':': 前进字节(1); return 标记(标记类型::冒号, ":", 当前行, 标记起始列);
        case '.': {
            前进字节(1);
            if (字节() == '.' && 位置 + 1 < 源代码.size() && static_cast<uint8_t>(源代码[位置 + 1]) == '.') {
                前进字节(1);
                前进字节(1);
                return 标记(标记类型::省略号, "...", 当前行, 标记起始列);
            }
            return 标记(标记类型::句号, ".", 当前行, 标记起始列);
        }
        case ',': 前进字节(1); return 标记(标记类型::逗号, ",", 当前行, 标记起始列);
        case '$': 前进字节(1); return 标记(标记类型::刀乐, "$", 当前行, 标记起始列);
        case '+': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::加等于, "+=", 当前行, 标记起始列); }
            if (字节() == '|') { 前进字节(1); return 标记(标记类型::自增, "+|", 当前行, 标记起始列); }
            return 标记(标记类型::加号, "+", 当前行, 标记起始列);
        }
        case '-': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::减等于, "-=", 当前行, 标记起始列); }
            if (字节() == '|') { 前进字节(1); return 标记(标记类型::自减, "-|", 当前行, 标记起始列); }
            return 标记(标记类型::减号, "-", 当前行, 标记起始列);
        }
        case '*': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::乘等于, "*=", 当前行, 标记起始列); }
            return 标记(标记类型::乘号, "*", 当前行, 标记起始列);
        }
        case '/': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::除等于, "/=", 当前行, 标记起始列); }
            return 标记(标记类型::除号, "/", 当前行, 标记起始列);
        }
        case '%': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::模等于, "%=", 当前行, 标记起始列); }
            return 标记(标记类型::百分号, "%", 当前行, 标记起始列);
        }
        case '!': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::感叹号等于, "!=", 当前行, 标记起始列); }
            return 标记(标记类型::感叹号, "!", 当前行, 标记起始列);
        }
        case '<': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::小于等于, "<=", 当前行, 标记起始列); }
            if (字节() == '|') {
                前进字节(1);
                if (字节() == '=') { 前进字节(1); return 标记(标记类型::左移等于, "<|=", 当前行, 标记起始列); }
                return 标记(标记类型::左移, "<|", 当前行, 标记起始列);
            }
            return 标记(标记类型::小于, "<", 当前行, 标记起始列);
        }
        case '>': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::大于等于, ">=", 当前行, 标记起始列); }
            if (字节() == '|') {
                前进字节(1);
                if (字节() == '=') { 前进字节(1); return 标记(标记类型::右移等于, ">|=", 当前行, 标记起始列); }
                return 标记(标记类型::右移, ">|", 当前行, 标记起始列);
            }
            return 标记(标记类型::大于, ">", 当前行, 标记起始列);
        }
        case '&': {
            前进字节(1);
            if (字节() == '&') { 前进字节(1); return 标记(标记类型::与与, "&&", 当前行, 标记起始列); }
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::位与等于, "&=", 当前行, 标记起始列); }
            return 标记(标记类型::位与, "&", 当前行, 标记起始列);
        }
        case '|': {
            前进字节(1);
            if (字节() == '|') { 前进字节(1); return 标记(标记类型::或或, "||", 当前行, 标记起始列); }
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::位或等于, "|=", 当前行, 标记起始列); }
            return 标记(标记类型::位或, "|", 当前行, 标记起始列);
        }
        case '^': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::位异或等于, "^=", 当前行, 标记起始列); }
            return 标记(标记类型::位异或, "^", 当前行, 标记起始列);
        }
        case '?': {
            前进字节(1);
            if (字节() == '?') {
                前进字节(1);
                return 标记(标记类型::问问, "??", 当前行, 标记起始列);
            }
            if (字节() == '=') {
                前进字节(1);
                return 标记(标记类型::等于等于, "?=", 当前行, 标记起始列);
            }
            return 标记(标记类型::问号, "?", 当前行, 标记起始列);
        }
        case '=': {
            前进字节(1);
            if (字节() == '>') { 前进字节(1); return 标记(标记类型::箭头, "=>", 当前行, 标记起始列); }
            return 标记(标记类型::等于, "=", 当前行, 标记起始列);
        }
        default: {
            std::string 未知符号(1, static_cast<char>(当前字节));
            前进字节(1);
            return 标记(标记类型::未知, 未知符号, 当前行, 标记起始列);
        }
    }
}
#include "词法分析器.h"
#include "公共.h"
#include <fstream>
#include <cctype>
#include <unordered_map>
#include <iomanip>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

static std::unordered_map<std::string, 标记类型> 关键字 = {
    {"函数", 标记类型::函数}, {"空", 标记类型::空}, 
    {"如果", 标记类型::如果}, {"否则", 标记类型::否则},
    {"否则如果", 标记类型::否则如果},
    {"循环", 标记类型::循环}, {"当", 标记类型::当},
    {"中断", 标记类型::中断}, {"继续", 标记类型::继续},
    {"变量", 标记类型::变量}, {"常量", 标记类型::常量}, {"打印", 标记类型::打印},
    {"返回", 标记类型::返回},
    {"匹配", 标记类型::匹配},
    {"遍历", 标记类型::遍历},
    {"导入", 标记类型::导入},
    {"整数", 标记类型::整数类型}, {"浮点", 标记类型::浮点类型}, {"布尔", 标记类型::布尔类型},
    {"字符串", 标记类型::字符串类型}, {"结构体", 标记类型::结构体},
    {"枚举", 标记类型::枚举},
    {"真", 标记类型::真}, {"假", 标记类型::假}
};

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
        default: return "未知";
    }
}

词法分析器::词法分析器(const std::string& 文件名) : 位置(0), 当前行(1) {
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
        if (字节() == '\n') 当前行++;
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

标记 词法分析器::下一个标记() {
    跳过空白和注释();
    if (已到文件尾()) {
        return 标记(标记类型::文件结束, "", 当前行);
    }

    uint8_t 当前字节 = static_cast<uint8_t>(字节());

    // 识别字符串字面量（支持 f"..." 字符串插值）
    if (当前字节 == 'f' && 位置 + 1 < 源代码.size() && static_cast<uint8_t>(字节(1)) == '"') {
        前进字节(1); // 跳过 'f'
        前进字节(1); // 跳过 '"'
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
                    case '{': 内容 += '\x01'; break;  // 转义的 { 用 \x01 标记
                    default: 内容 += '\\'; 内容 += 转义; break;
                }
                前进字节(1);
            } else {
                内容 += 读取多字节字符();
            }
        }
        if (已到文件尾()) {
            throw std::runtime_error("未闭合的字符串插值（行 " + std::to_string(当前行) + "）");
        }
        前进字节(1); // 跳过闭合 '"'
        调试打印("[词法分析器][标记] 字符串插值: '" << 内容 << "'");
        return 标记(标记类型::字符串插值, 内容, 当前行);
    }

    // 识别字符串字面量
    if (当前字节 == '"') {
        前进字节(1);
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
                    case '0': 内容 += '\0'; break;
                    default: 内容 += '\\'; 内容 += 转义; break;
                }
                前进字节(1);
            } else {
                内容 += 读取多字节字符();
            }
        }
        if (已到文件尾()) {
            throw std::runtime_error("未闭合的字符串字面量（行 " + std::to_string(当前行) + "）");
        }
        前进字节(1);
        调试打印("[词法分析器][标记] 字符串: '" << 内容 << "'");
        return 标记(标记类型::字符串, 内容, 当前行);
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
            return 标记(标记类型::浮点, 数字串, 当前行);
        }
        调试打印("[词法分析器][标记] 整数: '" << 数字串 << "'");
        return 标记(标记类型::整数, 数字串, 当前行);
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

        调试打印("[词法分析器][标记] 标识符: '" << 标识符 << "'（是否关键字: " << (关键字.count(标识符) ? "是" : "否") << "）");
        if (关键字.count(标识符)) {
            return 标记(关键字[标识符], 标识符, 当前行);
        } else {
            return 标记(标记类型::标识符, 标识符, 当前行);
        }
    }

    // 识别符号
    switch (当前字节) {
        case '(': 前进字节(1); return 标记(标记类型::左括号, "(", 当前行);
        case ')': 前进字节(1); return 标记(标记类型::右括号, ")", 当前行);
        case '{': 前进字节(1); return 标记(标记类型::左花括号, "{", 当前行);
        case '}': 前进字节(1); return 标记(标记类型::右花括号, "}", 当前行);
        case '[': 前进字节(1); return 标记(标记类型::左方括号, "[", 当前行);
        case ']': 前进字节(1); return 标记(标记类型::右方括号, "]", 当前行);
        case ';': 前进字节(1); return 标记(标记类型::分号, ";", 当前行);
        case ':': 前进字节(1); return 标记(标记类型::冒号, ":", 当前行);
        case '.': 前进字节(1); return 标记(标记类型::句号, ".", 当前行);
        case ',': 前进字节(1); return 标记(标记类型::逗号, ",", 当前行);
        case '+': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::加等于, "+=", 当前行); }
            return 标记(标记类型::加号, "+", 当前行);
        }
        case '-': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::减等于, "-=", 当前行); }
            return 标记(标记类型::减号, "-", 当前行);
        }
        case '*': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::乘等于, "*=", 当前行); }
            return 标记(标记类型::乘号, "*", 当前行);
        }
        case '/': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::除等于, "/=", 当前行); }
            return 标记(标记类型::除号, "/", 当前行);
        }
        case '%': 前进字节(1); return 标记(标记类型::百分号, "%", 当前行);
        case '!': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::感叹号等于, "!=", 当前行); }
            return 标记(标记类型::感叹号, "!", 当前行);
        }
        case '<': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::小于等于, "<=", 当前行); }
            return 标记(标记类型::小于, "<", 当前行);
        }
        case '>': {
            前进字节(1);
            if (字节() == '=') { 前进字节(1); return 标记(标记类型::大于等于, ">=", 当前行); }
            return 标记(标记类型::大于, ">", 当前行);
        }
        case '&': {
            前进字节(1);
            if (字节() == '&') { 前进字节(1); return 标记(标记类型::与与, "&&", 当前行); }
            return 标记(标记类型::未知, "&", 当前行);
        }
        case '|': {
            前进字节(1);
            if (字节() == '|') { 前进字节(1); return 标记(标记类型::或或, "||", 当前行); }
            return 标记(标记类型::未知, "|", 当前行);
        }
        case '?': {
            前进字节(1);
            if (字节() == '=') {
                前进字节(1);
                return 标记(标记类型::等于等于, "?=", 当前行);
            }
            return 标记(标记类型::未知, "?", 当前行);
        }
        case '=': {
            前进字节(1);
            if (字节() == '>') { 前进字节(1); return 标记(标记类型::箭头, "=>", 当前行); }
            return 标记(标记类型::等于, "=", 当前行);
        }
        default: {
            std::string 未知符号(1, static_cast<char>(当前字节));
            前进字节(1);
            return 标记(标记类型::未知, 未知符号, 当前行);
        }
    }
}
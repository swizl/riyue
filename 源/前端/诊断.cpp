#include "诊断.h"
#include <sstream>

void 诊断引擎::设置源码(const std::string& 源码, const std::string& 文件) {
    源代码 = 源码;
    文件名 = 文件;
    诊断列表.clear();
}

std::string 诊断引擎::提取行(int 行号) const {
    std::istringstream 流(源代码);
    std::string 当前行;
    for (int i = 1; i <= 行号; i++) {
        if (!std::getline(流, 当前行)) return "";
    }
    return 当前行;
}

std::string 诊断引擎::生成指示符(int 列号, int 长度) const {
    std::string 结果(列号 - 1, ' ');
    结果 += '^';
    if (长度 > 1) {
        for (int i = 1; i < 长度 && i < 20; i++) 结果 += '~';
    }
    return 结果;
}

std::string 诊断引擎::级别字符串(诊断信息::级别 等级) const {
    switch (等级) {
        case 诊断信息::错误: return "错误";
        case 诊断信息::警告: return "警告";
        case 诊断信息::注意: return "注意";
    }
    return "未知";
}

void 诊断引擎::报告错误(int 行号, int 列号, const std::string& 消息) {
    诊断信息 信息;
    信息.等级 = 诊断信息::错误;
    信息.位置 = {行号, 列号, 文件名};
    信息.消息 = 消息;
    信息.源码行 = 提取行(行号);
    信息.指示符 = 列号 > 0 ? 生成指示符(列号, 1) : "";
    诊断列表.push_back(信息);
}

void 诊断引擎::报告警告(int 行号, int 列号, const std::string& 消息) {
    诊断信息 信息;
    信息.等级 = 诊断信息::警告;
    信息.位置 = {行号, 列号, 文件名};
    信息.消息 = 消息;
    信息.源码行 = 提取行(行号);
    信息.指示符 = 列号 > 0 ? 生成指示符(列号, 1) : "";
    诊断列表.push_back(信息);
}

void 诊断引擎::报告错误带上下文(int 行号, int 列号, int 长度, const std::string& 消息) {
    诊断信息 信息;
    信息.等级 = 诊断信息::错误;
    信息.位置 = {行号, 列号, 文件名};
    信息.消息 = 消息;
    信息.源码行 = 提取行(行号);
    信息.指示符 = 列号 > 0 ? 生成指示符(列号, 长度) : "";
    诊断列表.push_back(信息);
}

void 诊断引擎::输出所有(std::ostream& 输出) const {
    for (const auto& 诊断 : 诊断列表) {
        输出 << 级别字符串(诊断.等级);
        if (!诊断.位置.文件名.empty()) {
            输出 << "[" << 诊断.位置.文件名 << "]";
        }
        输出 << ": " << 诊断.消息;
        输出 << "（行 " << 诊断.位置.行号;
        if (诊断.位置.列号 > 0) 输出 << ":" << 诊断.位置.列号;
        输出 << "）" << std::endl;

        if (!诊断.源码行.empty()) {
            输出 << "  " << 诊断.位置.行号 << " | " << 诊断.源码行 << std::endl;
            if (!诊断.指示符.empty()) {
                输出 << "  " << std::string(std::to_string(诊断.位置.行号).size(), ' ')
                     << " | " << 诊断.指示符 << std::endl;
            }
        }
    }
}

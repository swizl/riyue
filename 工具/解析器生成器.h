#ifndef 解析器生成器_H
#define 解析器生成器_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <regex>

struct 标记定义 {
    std::string 名称;
    std::string 模式;
    bool 是关键字 = false;
    bool 跳过 = false;
};

struct 产生式 {
    std::vector<std::string> 元素;
};

struct 规则定义 {
    std::string 名称;
    std::vector<产生式> 备选;
};

class 解析器生成器 {
private:
    std::vector<标记定义> 标记列表;
    std::vector<规则定义> 规则列表;
    std::set<std::string> 关键字集合;
    std::map<std::string, std::string> 字面量映射;  // 字面量 -> 标记名

    void 解析词法定义(const std::string& 内容);
    void 解析语法定义(const std::string& 内容);
    std::string 提取块(const std::string& 内容, size_t& 位置);

    void 生成词法分析器头文件(const std::string& 输出目录);
    void 生成词法分析器源文件(const std::string& 输出目录);
    void 生成语法分析器头文件(const std::string& 输出目录);
    void 生成语法分析器源文件(const std::string& 输出目录);

public:
    bool 解析(const std::string& 语法文件);
    void 生成(const std::string& 输出目录);
    void 打印信息();
};

#endif

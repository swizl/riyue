#ifndef 诊断_H
#define 诊断_H

#include <string>
#include <vector>
#include <iostream>

struct 源位置 {
    int 行号 = 0;
    int 列号 = 0;
    std::string 文件名;
};

struct 诊断信息 {
    enum 级别 { 错误, 警告, 注意 };
    级别 等级;
    源位置 位置;
    std::string 消息;
    std::string 源码行;     // 出错的源码行内容
    std::string 指示符;     // "^~~~~" 下划线
};

class 诊断引擎 {
    std::string 源代码;
    std::string 文件名;
    std::vector<诊断信息> 诊断列表;

public:
    void 设置源码(const std::string& 源码, const std::string& 文件);
    void 报告错误(int 行号, int 列号, const std::string& 消息);
    void 报告警告(int 行号, int 列号, const std::string& 消息);
    void 报告错误带上下文(int 行号, int 列号, int 长度, const std::string& 消息);
    void 输出所有(std::ostream& 输出 = std::cerr) const;
    bool 有错误() const { return !诊断列表.empty(); }
    int 错误数量() const { return (int)诊断列表.size(); }
    const std::vector<诊断信息>& 获取诊断() const { return 诊断列表; }

private:
    std::string 提取行(int 行号) const;
    std::string 生成指示符(int 列号, int 长度) const;
    std::string 级别字符串(诊断信息::级别 等级) const;
};

#endif // 诊断_H

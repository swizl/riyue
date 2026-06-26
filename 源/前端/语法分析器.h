#pragma once
#include "词法分析器.h"
#include "抽象语法树.h"
#include <memory>

class 语法分析器 {
private:
    词法分析器 _词法分析器;
    标记 _当前标记;
    std::vector<std::pair<std::string, std::vector<结构体成员>>> 结构体定义缓存;
    std::vector<std::string> 导入列表;
    std::unordered_map<std::string, std::string> 导入别名映射;
    std::unordered_map<std::string, std::vector<std::string>> 选择导入映射;

    void 前进();
    void 期望类型(标记类型 类型, const std::string& 信息);
    void 期望(标记类型 类型, const std::string& 信息);

    std::unique_ptr<表达式> 解析基本表达式();
    std::unique_ptr<表达式> 解析一元表达式();
    std::unique_ptr<表达式> 解析乘除表达式();
    std::unique_ptr<表达式> 解析加减表达式();
    std::unique_ptr<表达式> 解析移位表达式();
    std::unique_ptr<表达式> 解析比较表达式();
    std::unique_ptr<表达式> 解析位与表达式();
    std::unique_ptr<表达式> 解析位异或表达式();
    std::unique_ptr<表达式> 解析位或表达式();
    std::unique_ptr<表达式> 解析逻辑与表达式();
    std::unique_ptr<表达式> 解析逻辑或表达式();
    std::unique_ptr<表达式> 解析管道表达式();
    std::unique_ptr<表达式> 解析空值合并表达式();
    std::unique_ptr<表达式> 解析条件表达式();

    std::vector<函数参数> 解析参数列表(const std::string& 默认类型 = "", bool 支持默认值 = false, bool 支持变长 = false);
    std::vector<返回值描述> 解析返回值列表();

public:
    std::unique_ptr<表达式> 解析表达式();
    std::unique_ptr<语句> 解析语句();
    std::unique_ptr<函数> 解析函数();
    std::vector<std::unique_ptr<语句>> 解析代码块();

public:
    语法分析器(const std::string& 文件名) : _词法分析器(文件名), _当前标记(标记类型::未知, "", 0) {
        前进();
    }
    语法分析器(const char* 源码, size_t 长度) : _词法分析器(源码, 长度), _当前标记(标记类型::未知, "", 0) {
        前进();
    }

    std::unique_ptr<程序> 解析程序();
    const 标记& 当前标记() const { return _当前标记; }
    const std::vector<std::pair<std::string, std::vector<结构体成员>>>& 获取结构体定义() const { return 结构体定义缓存; }
    const std::vector<std::string>& 获取导入列表() const { return 导入列表; }
    const std::unordered_map<std::string, std::string>& 获取导入别名映射() const { return 导入别名映射; }
    const std::unordered_map<std::string, std::vector<std::string>>& 获取选择导入映射() const { return 选择导入映射; }
};

#ifndef 语法树_H
#define 语法树_H

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

enum class 表达式类型 { 整数, 浮点数, 布尔值, 字符, 字符串, 变量, 二元运算, 一元运算, 函数调用, 数组字面量, 下标访问, 匿名函数, 结构体实例, 成员访问, 映射字面量, 枚举成员, 元组字面量, 元组访问, 分片访问, 自增自减, 管道调用 };
enum class 二元操作符 { 加法, 减法, 乘法, 除法, 取模, 等于, 不等于, 小于, 大于, 小于等于, 大于等于, 逻辑与, 逻辑或, 左移, 右移, 位与, 位或, 位异或 };
enum class 一元操作符 { 逻辑非, 取负 };

class 表达式 {
public:
    表达式类型 类型;
    virtual ~表达式() = default;
protected:
    表达式(表达式类型 类型) : 类型(类型) {}
};

class 整数表达式 : public 表达式 {
public:
    int 值;
    整数表达式(int 值) : 表达式(表达式类型::整数), 值(值) {}
};

class 浮点表达式 : public 表达式 {
public:
    double 值;
    浮点表达式(double 值) : 表达式(表达式类型::浮点数), 值(值) {}
};

class 布尔表达式 : public 表达式 {
public:
    bool 值;
    布尔表达式(bool 值) : 表达式(表达式类型::布尔值), 值(值) {}
};

class 字符串表达式 : public 表达式 {
public:
    std::string 值;
    字符串表达式(std::string 值) : 表达式(表达式类型::字符串), 值(std::move(值)) {}
};

class 字符表达式 : public 表达式 {
public:
    int 值;
    字符表达式(int 值) : 表达式(表达式类型::字符), 值(值) {}
};

class 变量表达式 : public 表达式 {
public:
    std::string 名称;
    变量表达式(std::string 名称) : 表达式(表达式类型::变量), 名称(std::move(名称)) {}
};

class 二元运算表达式 : public 表达式 {
public:
    二元操作符 操作符;
    std::unique_ptr<表达式> 左操作数;
    std::unique_ptr<表达式> 右操作数;
    二元运算表达式(二元操作符 op, std::unique_ptr<表达式> 左, std::unique_ptr<表达式> 右)
        : 表达式(表达式类型::二元运算), 操作符(op), 左操作数(std::move(左)), 右操作数(std::move(右)) {}
};

class 一元运算表达式 : public 表达式 {
public:
    一元操作符 操作符;
    std::unique_ptr<表达式> 操作数;
    一元运算表达式(一元操作符 op, std::unique_ptr<表达式> 操作数_)
        : 表达式(表达式类型::一元运算), 操作符(op), 操作数(std::move(操作数_)) {}
};

class 函数调用表达式 : public 表达式 {
public:
    std::string 函数名;
    std::vector<std::unique_ptr<表达式>> 参数列表;
    函数调用表达式(std::string 名, std::vector<std::unique_ptr<表达式>> 参数)
        : 表达式(表达式类型::函数调用), 函数名(std::move(名)), 参数列表(std::move(参数)) {}
};

class 数组字面量表达式 : public 表达式 {
public:
    std::vector<std::unique_ptr<表达式>> 元素列表;
    数组字面量表达式(std::vector<std::unique_ptr<表达式>> 元素)
        : 表达式(表达式类型::数组字面量), 元素列表(std::move(元素)) {}
};

class 下标访问表达式 : public 表达式 {
public:
    std::string 数组名;
    std::unique_ptr<表达式> 索引;
    下标访问表达式(std::string 名, std::unique_ptr<表达式> idx)
        : 表达式(表达式类型::下标访问), 数组名(std::move(名)), 索引(std::move(idx)) {}
};

class 分片访问表达式 : public 表达式 {
public:
    std::string 数组名;
    std::unique_ptr<表达式> 开始索引;
    std::unique_ptr<表达式> 结束索引;
    分片访问表达式(std::string 名, std::unique_ptr<表达式> 开始, std::unique_ptr<表达式> 结束)
        : 表达式(表达式类型::分片访问), 数组名(std::move(名)), 开始索引(std::move(开始)), 结束索引(std::move(结束)) {}
};

struct 函数参数;
struct 返回值描述;
class 语句;

class 匿名函数表达式 : public 表达式 {
public:
    std::vector<函数参数> 参数列表;
    std::vector<返回值描述> 返回值列表;
    std::vector<std::unique_ptr<语句>> 主体;
    匿名函数表达式(std::vector<函数参数> 参数, std::vector<返回值描述> 返回值, std::vector<std::unique_ptr<语句>> body)
        : 表达式(表达式类型::匿名函数), 参数列表(std::move(参数)), 返回值列表(std::move(返回值)), 主体(std::move(body)) {}
};

class 结构体实例表达式 : public 表达式 {
public:
    std::string 结构体名;
    std::vector<std::pair<std::string, std::unique_ptr<表达式>>> 成员列表;
    结构体实例表达式(std::string 名, std::vector<std::pair<std::string, std::unique_ptr<表达式>>> 成员)
        : 表达式(表达式类型::结构体实例), 结构体名(std::move(名)), 成员列表(std::move(成员)) {}
};

class 成员访问表达式 : public 表达式 {
public:
    std::string 对象名;
    std::string 成员名;
    成员访问表达式(std::string 对象, std::string 成员)
        : 表达式(表达式类型::成员访问), 对象名(std::move(对象)), 成员名(std::move(成员)) {}
};

class 映射字面量表达式 : public 表达式 {
public:
    std::vector<std::pair<std::unique_ptr<表达式>, std::unique_ptr<表达式>>> 键值对列表;
    映射字面量表达式(std::vector<std::pair<std::unique_ptr<表达式>, std::unique_ptr<表达式>>> 键值对)
        : 表达式(表达式类型::映射字面量), 键值对列表(std::move(键值对)) {}
};

class 枚举成员表达式 : public 表达式 {
public:
    std::string 枚举名;
    std::string 成员名;
    枚举成员表达式(std::string 枚举, std::string 成员)
        : 表达式(表达式类型::枚举成员), 枚举名(std::move(枚举)), 成员名(std::move(成员)) {}
};

class 元组字面量表达式 : public 表达式 {
public:
    std::vector<std::unique_ptr<表达式>> 元素列表;
    元组字面量表达式(std::vector<std::unique_ptr<表达式>> 元素)
        : 表达式(表达式类型::元组字面量), 元素列表(std::move(元素)) {}
};

class 元组访问表达式 : public 表达式 {
public:
    std::string 元组名;
    int 索引;
    元组访问表达式(std::string 名, int idx)
        : 表达式(表达式类型::元组访问), 元组名(std::move(名)), 索引(idx) {}
};

class 自增自减表达式 : public 表达式 {
public:
    std::string 变量名;
    bool 是自增;
    自增自减表达式(std::string 名, bool 增)
        : 表达式(表达式类型::自增自减), 变量名(std::move(名)), 是自增(增) {}
};

class 管道调用表达式 : public 表达式 {
public:
    std::unique_ptr<表达式> 左表达式;  // 管道左边的表达式（通常是函数调用）
    std::string 右函数名;               // 管道右边的函数名
    std::vector<std::unique_ptr<表达式>> 右参数列表;  // 右边函数的额外参数
    std::unordered_map<std::string, std::string> 参数映射;  // 左返回值名 -> 右参数名映射

    管道调用表达式(std::unique_ptr<表达式> 左, std::string 右名,
                   std::vector<std::unique_ptr<表达式>> 右参数,
                   std::unordered_map<std::string, std::string> 映射)
        : 表达式(表达式类型::管道调用), 左表达式(std::move(左)), 右函数名(std::move(右名)),
          右参数列表(std::move(右参数)), 参数映射(std::move(映射)) {}
};

enum class 语句类型 { 变量声明, 常量声明, 赋值语句, 下标赋值语句, 解构赋值, 如果语句, 循环语句, 当循环语句, 做循环语句, 中断语句, 继续语句, 打印语句, 返回语句, 表达式语句, 代码块, 匹配语句, 遍历语句, 结构体定义, 枚举定义, 导入语句 };

class 语句 {
public:
    语句类型 类型;
    virtual ~语句() = default;
protected:
    语句(语句类型 类型) : 类型(类型) {}
};

class 变量声明 : public 语句 {
public:
    std::string 变量名;
    std::unique_ptr<表达式> 初始值;
    变量声明(std::string 名, std::unique_ptr<表达式> 值)
        : 语句(语句类型::变量声明), 变量名(std::move(名)), 初始值(std::move(值)) {}
};

class 常量声明 : public 语句 {
public:
    std::string 常量名;
    std::unique_ptr<表达式> 初始值;
    常量声明(std::string 名, std::unique_ptr<表达式> 值)
        : 语句(语句类型::常量声明), 常量名(std::move(名)), 初始值(std::move(值)) {}
};

class 赋值语句 : public 语句 {
public:
    std::string 变量名;
    std::unique_ptr<表达式> 值表达式;
    赋值语句(std::string 名, std::unique_ptr<表达式> expr)
        : 语句(语句类型::赋值语句), 变量名(std::move(名)), 值表达式(std::move(expr)) {}
};

class 下标赋值语句 : public 语句 {
public:
    std::string 数组名;
    std::unique_ptr<表达式> 索引;
    std::unique_ptr<表达式> 值表达式;
    下标赋值语句(std::string 名, std::unique_ptr<表达式> idx, std::unique_ptr<表达式> val)
        : 语句(语句类型::下标赋值语句), 数组名(std::move(名)), 索引(std::move(idx)), 值表达式(std::move(val)) {}
};

struct 否则如果分支 {
    std::unique_ptr<表达式> 条件;
    std::vector<std::unique_ptr<语句>> 主体;
};

class 如果语句 : public 语句 {
public:
    std::unique_ptr<表达式> 条件;
    std::vector<std::unique_ptr<语句>> then块;
    std::vector<否则如果分支> 否则如果列表;
    std::vector<std::unique_ptr<语句>> else块;
    如果语句(std::unique_ptr<表达式> cond, std::vector<std::unique_ptr<语句>> then,
             std::vector<否则如果分支> elifs, std::vector<std::unique_ptr<语句>> else_)
        : 语句(语句类型::如果语句), 条件(std::move(cond)), then块(std::move(then)),
          否则如果列表(std::move(elifs)), else块(std::move(else_)) {}
};

class 循环语句 : public 语句 {
public:
    std::unique_ptr<语句> 初始化;  // 可选
    std::unique_ptr<表达式> 条件;    // 可选
    std::unique_ptr<语句> 步进;      // 可选
    std::vector<std::unique_ptr<语句>> 主体;
    循环语句(std::vector<std::unique_ptr<语句>> body)
        : 语句(语句类型::循环语句), 主体(std::move(body)) {}
    循环语句(std::unique_ptr<语句> init, std::unique_ptr<表达式> cond,
             std::unique_ptr<语句> step, std::vector<std::unique_ptr<语句>> body)
        : 语句(语句类型::循环语句), 初始化(std::move(init)), 条件(std::move(cond)),
          步进(std::move(step)), 主体(std::move(body)) {}
};

class 当循环语句 : public 语句 {
public:
    std::unique_ptr<表达式> 条件;
    std::vector<std::unique_ptr<语句>> 主体;
    当循环语句(std::unique_ptr<表达式> cond, std::vector<std::unique_ptr<语句>> body)
        : 语句(语句类型::当循环语句), 条件(std::move(cond)), 主体(std::move(body)) {}
};

class 做循环语句 : public 语句 {
public:
    std::unique_ptr<表达式> 条件;
    std::vector<std::unique_ptr<语句>> 主体;
    bool 是当循环; // true = do-while, false = do-until
    做循环语句(std::unique_ptr<表达式> cond, std::vector<std::unique_ptr<语句>> body, bool 是当)
        : 语句(语句类型::做循环语句), 条件(std::move(cond)), 主体(std::move(body)), 是当循环(是当) {}
};

class 中断语句 : public 语句 {
public:
    中断语句() : 语句(语句类型::中断语句) {}
};

class 继续语句 : public 语句 {
public:
    继续语句() : 语句(语句类型::继续语句) {}
};

class 打印语句 : public 语句 {
public:
    std::unique_ptr<表达式> 值表达式;
    打印语句(std::unique_ptr<表达式> expr)
        : 语句(语句类型::打印语句), 值表达式(std::move(expr)) {}
};

class 返回语句 : public 语句 {
public:
    std::unique_ptr<表达式> 返回值;
    返回语句(std::unique_ptr<表达式> 值)
        : 语句(语句类型::返回语句), 返回值(std::move(值)) {}
};

class 表达式语句 : public 语句 {
public:
    std::unique_ptr<表达式> 值表达式;
    表达式语句(std::unique_ptr<表达式> expr)
        : 语句(语句类型::表达式语句), 值表达式(std::move(expr)) {}
};

class 解构赋值语句 : public 语句 {
public:
    std::vector<std::string> 变量名列表;
    std::unique_ptr<表达式> 值表达式;
    解构赋值语句(std::vector<std::string> 变量名, std::unique_ptr<表达式> 值)
        : 语句(语句类型::解构赋值), 变量名列表(std::move(变量名)), 值表达式(std::move(值)) {}
};

class 代码块语句 : public 语句 {
public:
    std::vector<std::unique_ptr<语句>> 语句列表;
    代码块语句(std::vector<std::unique_ptr<语句>> stmts)
        : 语句(语句类型::代码块), 语句列表(std::move(stmts)) {}
};

struct 匹配分支 {
    std::unique_ptr<表达式> 条件;  // nullptr 表示默认分支
    std::vector<std::unique_ptr<语句>> 主体;
};

class 匹配语句 : public 语句 {
public:
    std::unique_ptr<表达式> 匹配值;
    std::vector<匹配分支> 分支列表;
    匹配语句(std::unique_ptr<表达式> val, std::vector<匹配分支> branches)
        : 语句(语句类型::匹配语句), 匹配值(std::move(val)), 分支列表(std::move(branches)) {}
};

class 遍历语句 : public 语句 {
public:
    std::string 变量名;
    std::unique_ptr<表达式> 数组表达式;
    std::vector<std::unique_ptr<语句>> 主体;
    遍历语句(std::string 名, std::unique_ptr<表达式> arr, std::vector<std::unique_ptr<语句>> body)
        : 语句(语句类型::遍历语句), 变量名(std::move(名)), 数组表达式(std::move(arr)), 主体(std::move(body)) {}
};

struct 结构体成员 {
    std::string 名称;
    std::string 类型;
};

class 结构体定义语句 : public 语句 {
public:
    std::string 名称;
    std::vector<结构体成员> 成员列表;
    结构体定义语句(std::string 名, std::vector<结构体成员> 成员)
        : 语句(语句类型::结构体定义), 名称(std::move(名)), 成员列表(std::move(成员)) {}
};

class 导入语句 : public 语句 {
public:
    std::string 模块名;
    导入语句(std::string 名)
        : 语句(语句类型::导入语句), 模块名(std::move(名)) {}
};

class 枚举定义语句 : public 语句 {
public:
    std::string 名称;
    std::vector<std::string> 成员列表;
    枚举定义语句(std::string 名, std::vector<std::string> 成员)
        : 语句(语句类型::枚举定义), 名称(std::move(名)), 成员列表(std::move(成员)) {}
};

struct 全局变量声明 {
    std::string 变量名;
    std::unique_ptr<表达式> 初始值;
};

struct 函数参数 {
    std::string 名称;
    std::string 类型;
    std::unique_ptr<表达式> 默认值;
    bool 是否变长 = false;
    bool 是否数组 = false;
};

struct 返回值描述 {
    std::string 名称;
    std::string 类型;
    std::unique_ptr<表达式> 默认值;
};

class 函数 {
public:
    std::string 名称;
    std::vector<std::string> 类型参数列表;
    std::vector<函数参数> 参数列表;
    std::vector<返回值描述> 返回值列表;  // 支持多返回值
    std::vector<std::unique_ptr<语句>> 主体;
    函数(std::string 名, std::vector<std::string> 类型参数, std::vector<函数参数> 参数, std::vector<返回值描述> 返回值,
         std::vector<std::unique_ptr<语句>> body)
        : 名称(std::move(名)), 类型参数列表(std::move(类型参数)), 参数列表(std::move(参数)), 返回值列表(std::move(返回值)),
          主体(std::move(body)) {}
};

struct 程序 {
    std::vector<std::unique_ptr<全局变量声明>> 全局变量;
    std::vector<std::unique_ptr<函数>> 函数列表;
    std::vector<std::pair<std::string, std::vector<结构体成员>>> 结构体定义列表;
    std::vector<std::pair<std::string, std::vector<std::string>>> 枚举定义列表;
};

#endif

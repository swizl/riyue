#ifndef 字节码_H
#define 字节码_H

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

enum class 操作码 : uint8_t {
    停止 = 0,
    加载常量,    // A=寄存器 B=常量索引
    加载变量,    // A=寄存器 B=变量索引
    存储变量,    // A=变量索引 B=寄存器
    复制寄存器,  // A=目标 B=源

    加法,        // A=目标 B=左 C=右
    减法,
    乘法,
    除法,
    取模,
    等于,
    不等于,
    小于,
    大于,
    小于等于,
    大于等于,
    左移,        // A=目标 B=左 C=右
    右移,        // A=目标 B=左 C=右
    位与,        // A=目标 B=左 C=右
    位或,        // A=目标 B=左 C=右
    位异或,      // A=目标 B=左 C=右
    逻辑与,
    逻辑或,
    逻辑非,     // A=目标 B=源
    取负,        // A=目标 B=源

    // 浮点运算
    浮点加法,    // A=目标 B=左 C=右
    浮点减法,
    浮点乘法,
    浮点除法,
    浮点等于,
    浮点不等于,
    浮点小于,
    浮点大于,
    浮点小于等于,
    浮点大于等于,
    浮点取负,    // A=目标 B=源
    整数转浮点,  // A=目标 B=源
    浮点转整数,  // A=目标 B=源

    跳转,        // A=偏移（有符号16位）
    假跳转,      // A=寄存器 B=偏移
    调用,        // A=函数索引 B=参数数量 C=返回值寄存器
    返回值,      // A=寄存器
    返回空,
    打印,        // A=寄存器
    打印浮点,    // A=寄存器
    打印字符串,  // A=寄存器

    // 字符串操作
    加载字符串,  // A=寄存器 B=字符串表索引
    字符串拼接,  // A=目标 B=左 C=右
    字符串长度,  // A=目标 B=源
    字符串查找,  // A=目标 B=源 C=查找串
    整数转字符串, // A=目标 B=源
    浮点转字符串, // A=目标 B=源

    // 数组操作
    创建数组,    // A=寄存器 B=大小
    加载数组元素, // A=目标 B=数组寄存器 C=索引寄存器
    存储数组元素, // A=数组寄存器 B=索引寄存器 C=值寄存器

    // 数学函数
    绝对值运算,   // A=目标 B=源
    最小值运算,   // A=目标 B=左 C=右
    最大值运算,   // A=目标 B=左 C=右

    // 系统函数
    获取参数数量运算, // A=目标
    获取参数运算,     // A=目标 B=索引
    输出错误运算,     // A=寄存器
    创建目录运算,     // A=目标 B=路径
    目录存在运算,     // A=目标 B=路径
    删除文件运算,     // A=目标 B=路径
    分割行数运算,     // A=目标 B=文本
    获取行运算,       // A=目标 B=文本 C=行号
    去除空白运算,     // A=目标 B=文本
    转小写运算,       // A=目标 B=文本
    转大写运算,       // A=目标 B=文本
    字符串开头运算,   // A=目标 B=文本 C=前缀
    字符串结尾运算,   // A=目标 B=文本 C=后缀

    // 字符操作
    字符码运算,       // A=目标 B=文本 C=位置
    字符运算,         // A=目标 B=码点

    // 动态数组操作
    创建动态数组运算, // A=目标
    动态数组添加运算, // A=数组 B=值
    动态数组获取运算, // A=目标 B=数组 C=索引
    动态数组设置运算, // A=数组 B=索引 C=值
    动态数组大小运算, // A=目标 B=数组
    动态数组删除运算, // A=数组 B=索引
    动态数组插入运算, // A=数组 B=索引 C=值

    // 闭包操作
    创建闭包,    // A=目标 B=函数索引 C=捕获数量 (后续指令列出捕获变量寄存器)
    调用闭包,    // A=闭包寄存器 B=参数数量 C=返回值寄存器

    // 结构体操作
    创建结构体,  // A=目标 B=成员数量
    存储成员,    // A=结构体寄存器 B=成员名常量索引 C=值寄存器
    加载成员,    // A=目标 B=结构体寄存器 C=成员名常量索引

    // 目录操作
    列出目录运算,         // A=目标 B=路径
    获取目录项名称运算,   // A=目标 B=索引
    获取目录项是否目录运算, // A=目标 B=索引
    获取目录项大小运算,   // A=目标 B=索引
    递归遍历目录运算,     // A=目标 B=路径 C=深度

    // 异常处理
    设置异常处理, // A=偏移到catch块
    清除异常处理,
    抛出异常,     // A=异常值寄存器

    // 协程操作
    创建协程,     // A=目标寄存器 B=函数索引
    让出协程,     // A=返回值寄存器
    恢复协程,     // A=目标寄存器 B=协程寄存器

    // 通道操作
    创建通道运算,   // A=目标寄存器 B=容量
    发送通道运算,   // A=通道寄存器 B=值寄存器
    接收通道运算,   // A=目标寄存器 B=通道寄存器
    关闭通道运算,   // A=通道寄存器

    // 调试器
    断点运算,       // 无操作数
};

std::string 操作码名称(操作码 码);

struct 协程状态;
struct 通道结构;

// 值类型：支持整数、浮点数、字符串、数组、闭包、结构体、动态数组、协程、通道
struct 值 {
    enum 类型 { 整数, 浮点数, 字符串, 数组, 闭包, 结构体, 动态数组, 协程, 通道 } 类型;
    union {
        int32_t 整数值;
        double 浮点值;
    };
    std::string 字符串值;
    std::vector<int32_t> 数组值;
    // 闭包数据
    int 闭包函数索引 = -1;
    std::vector<值> 捕获变量;
    // 结构体数据
    std::unordered_map<std::string, 值> 成员映射;
    // 动态数组数据
    void* 动态数组指针 = nullptr;
    // 协程数据
    协程状态* 协程指针 = nullptr;
    // 通道数据
    通道结构* 通道指针 = nullptr;
    // 引用计数
    int* 引用计数 = nullptr;

    值() : 类型(整数), 整数值(0) {}
    值(int32_t v) : 类型(整数), 整数值(v) {}
    值(double v) : 类型(浮点数), 浮点值(v) {}
    值(const std::string& s) : 类型(字符串), 字符串值(s) {}

    // 拷贝构造：增加引用计数
    值(const 值& other) : 类型(other.类型), 整数值(other.整数值), 字符串值(other.字符串值),
        数组值(other.数组值), 闭包函数索引(other.闭包函数索引), 捕获变量(other.捕获变量),
        成员映射(other.成员映射), 动态数组指针(other.动态数组指针),
        协程指针(other.协程指针), 通道指针(other.通道指针), 引用计数(other.引用计数) {
        if (引用计数) (*引用计数)++;
    }

    // 移动构造：转移所有权
    值(值&& other) noexcept : 类型(other.类型), 整数值(other.整数值), 字符串值(std::move(other.字符串值)),
        数组值(std::move(other.数组值)), 闭包函数索引(other.闭包函数索引), 捕获变量(std::move(other.捕获变量)),
        成员映射(std::move(other.成员映射)), 动态数组指针(other.动态数组指针),
        协程指针(other.协程指针), 通道指针(other.通道指针), 引用计数(other.引用计数) {
        other.动态数组指针 = nullptr;
        other.协程指针 = nullptr;
        other.通道指针 = nullptr;
        other.引用计数 = nullptr;
    }

    // 拷贝赋值
    值& operator=(const 值& other) {
        if (this != &other) {
            释放资源();
            类型 = other.类型;
            整数值 = other.整数值;
            字符串值 = other.字符串值;
            数组值 = other.数组值;
            闭包函数索引 = other.闭包函数索引;
            捕获变量 = other.捕获变量;
            成员映射 = other.成员映射;
            动态数组指针 = other.动态数组指针;
            协程指针 = other.协程指针;
            通道指针 = other.通道指针;
            引用计数 = other.引用计数;
            if (引用计数) (*引用计数)++;
        }
        return *this;
    }

    // 移动赋值
    值& operator=(值&& other) noexcept {
        if (this != &other) {
            释放资源();
            类型 = other.类型;
            整数值 = other.整数值;
            字符串值 = std::move(other.字符串值);
            数组值 = std::move(other.数组值);
            闭包函数索引 = other.闭包函数索引;
            捕获变量 = std::move(other.捕获变量);
            成员映射 = std::move(other.成员映射);
            动态数组指针 = other.动态数组指针;
            协程指针 = other.协程指针;
            通道指针 = other.通道指针;
            引用计数 = other.引用计数;
            other.动态数组指针 = nullptr;
            other.协程指针 = nullptr;
            other.通道指针 = nullptr;
            other.引用计数 = nullptr;
        }
        return *this;
    }

    // 析构：减少引用计数，释放资源
    ~值() { 释放资源(); }

private:
    void 释放资源() {
        if (引用计数 && --(*引用计数) == 0) {
            delete 引用计数;
            // 动态数组由 C 运行时管理
            // 协程和通道由 VM 管理
        }
        引用计数 = nullptr;
        动态数组指针 = nullptr;
        协程指针 = nullptr;
        通道指针 = nullptr;
    }
};

struct 指令 {
    操作码 码;
    uint8_t A = 0;
    uint8_t B = 0;
    uint8_t C = 0;
    int16_t 偏移 = 0;

    static 指令 三操作数(操作码 码, uint8_t a, uint8_t b, uint8_t c) {
        指令 i; i.码 = 码; i.A = a; i.B = b; i.C = c; return i;
    }
    static 指令 双操作数(操作码 码, uint8_t a, uint8_t b) {
        指令 i; i.码 = 码; i.A = a; i.B = b; return i;
    }
    static 指令 单操作数(操作码 码, uint8_t a) {
        指令 i; i.码 = 码; i.A = a; return i;
    }
    static 指令 零操作数(操作码 码) {
        指令 i; i.码 = 码; return i;
    }
    static 指令 跳转指令(操作码 码, int16_t 偏移) {
        指令 i; i.码 = 码; i.偏移 = 偏移; return i;
    }
    static 指令 条件跳转(操作码 码, uint8_t a, int16_t 偏移) {
        指令 i; i.码 = 码; i.A = a; i.偏移 = 偏移; return i;
    }
    static 指令 调用指令(uint8_t 函数索引, uint8_t 参数数量, uint8_t 返回寄存器) {
        指令 i; i.码 = 操作码::调用; i.A = 函数索引; i.B = 参数数量; i.C = 返回寄存器; return i;
    }
};

struct 函数信息 {
    std::string 名称;
    uint8_t 参数数量 = 0;
    uint8_t 返回值数量 = 0;
    uint8_t 最大寄存器 = 0;
    std::vector<指令> 指令列表;
};

struct 字节码 {
    std::vector<值> 常量池;
    std::vector<函数信息> 函数表;
    int 入口函数索引 = -1;

    int 添加常量(int32_t v) {
        for (size_t i = 0; i < 常量池.size(); i++) {
            if (常量池[i].类型 == 值::整数 && 常量池[i].整数值 == v) return static_cast<int>(i);
        }
        常量池.push_back(值(v));
        return static_cast<int>(常量池.size() - 1);
    }

    int 添加浮点常量(double v) {
        for (size_t i = 0; i < 常量池.size(); i++) {
            if (常量池[i].类型 == 值::浮点数 && 常量池[i].浮点值 == v) return static_cast<int>(i);
        }
        常量池.push_back(值(v));
        return static_cast<int>(常量池.size() - 1);
    }

    int 添加字符串常量(const std::string& v) {
        for (size_t i = 0; i < 常量池.size(); i++) {
            if (常量池[i].类型 == 值::字符串 && 常量池[i].字符串值 == v) return static_cast<int>(i);
        }
        常量池.push_back(值(v));
        return static_cast<int>(常量池.size() - 1);
    }

    void 输出(const std::string& 文件名) const;
    static 字节码 读取(const std::string& 文件名);
};

#endif

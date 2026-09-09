#ifndef 符号表_H
#define 符号表_H

#include <unordered_map>
#include <vector>
#include <string>
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "../共享/LLVM中文.h"
#include "../前端/公共.h"
#include "../前端/抽象语法树.h"

using namespace llvm中文;

struct 闭包信息 {
    LLVM函数* 函数;
    std::vector<std::string> 捕获变量名;
    std::vector<LLVM值*> 捕获变量地址;
};

enum class 变量类型种类 {
    类型_整数,
    类型_浮点,
    字符串,
    类型_布尔,
    数组,
    映射,
    类型_结构体,
    函数,
    类型_空
};

struct 变量类型信息 {
    变量类型种类 种类 = 变量类型种类::类型_整数;
    std::string 结构体名;   // 仅结构体使用
    int 数组大小 = 0;       // 仅数组使用
    bool 是常量 = false;
};

class 符号表 {
private:
    std::vector<std::unordered_map<std::string, LLVM值*>> 作用域栈;
    std::unordered_map<std::string, 变量类型信息> 类型映射;
    std::unordered_map<std::string, 闭包信息> 闭包映射;
    std::unordered_map<std::string, std::vector<std::string>> 枚举定义映射;
    std::unordered_map<std::string, std::pair<std::string, int>> 枚举成员映射;
    int 匿名函数计数 = 0;

    // 每个作用域中新增的类型标记变量名
    std::vector<std::vector<std::string>> 类型标记作用域栈;

public:
    符号表() { 作用域栈.push_back({}); 类型标记作用域栈.push_back({}); }

    void 进入作用域() {
        作用域栈.push_back({});
        类型标记作用域栈.push_back({});
        调试打印("[符号表] 进入新作用域，当前深度: " << 作用域栈.size());
    }

    void 退出作用域() {
        if (作用域栈.size() > 1) {
            调试打印("[符号表] 退出作用域，移除 " << 作用域栈.back().size() << " 个变量");
            // 清理该作用域中新增的类型标记
            for (const auto& 名称 : 类型标记作用域栈.back()) {
                类型映射.erase(名称);
            }
            作用域栈.pop_back();
            类型标记作用域栈.pop_back();
        }
    }

    void 重置为全局作用域() {
        while (作用域栈.size() > 1) {
            for (const auto& 名称 : 类型标记作用域栈.back()) {
                类型映射.erase(名称);
            }
            作用域栈.pop_back();
            类型标记作用域栈.pop_back();
        }
    }

    void 声明变量(const std::string& 名称, LLVM值* 值) {
        作用域栈.back()[名称] = 值;
        调试打印("[符号表] 声明变量: '" << 名称 << "'（作用域深度 " << 作用域栈.size() << "）");
    }

    // 统一类型设置接口
    void 设置变量类型(const std::string& 名称, 变量类型信息 信息) {
        if (!类型映射.count(名称)) 类型标记作用域栈.back().push_back(名称);
        类型映射[名称] = 信息;
    }

    变量类型信息 获取变量类型(const std::string& 名称) const {
        auto it = 类型映射.find(名称);
        if (it != 类型映射.end()) return it->second;
        return {变量类型种类::类型_整数, "", 0, false};
    }

    // 便捷接口（保持向后兼容）
    void 设置浮点变量(const std::string& 名称) {
        设置变量类型(名称, {变量类型种类::类型_浮点, "", 0, false});
    }

    bool 是浮点变量(const std::string& 名称) const {
        auto it = 类型映射.find(名称);
        return it != 类型映射.end() && it->second.种类 == 变量类型种类::类型_浮点;
    }

    void 设置指针变量(const std::string& 名称) {
        设置变量类型(名称, {变量类型种类::字符串, "", 0, false});
    }

    bool 是指针变量(const std::string& 名称) const {
        auto it = 类型映射.find(名称);
        return it != 类型映射.end() && (it->second.种类 == 变量类型种类::字符串 || it->second.种类 == 变量类型种类::函数);
    }

    void 设置结构体变量(const std::string& 名称, const std::string& 类型名称) {
        设置变量类型(名称, {变量类型种类::类型_结构体, 类型名称, 0, false});
    }

    const std::string& 获取结构体类型(const std::string& 名称) const {
        static std::string 空字符串;
        auto it = 类型映射.find(名称);
        if (it != 类型映射.end() && it->second.种类 == 变量类型种类::类型_结构体) return it->second.结构体名;
        return 空字符串;
    }

    void 设置映射变量(const std::string& 名称) {
        设置变量类型(名称, {变量类型种类::映射, "", 0, false});
    }

    bool 是映射变量(const std::string& 名称) const {
        auto it = 类型映射.find(名称);
        return it != 类型映射.end() && it->second.种类 == 变量类型种类::映射;
    }

    void 设置字符串数组(const std::string& 名称) {
        设置变量类型(名称, {变量类型种类::数组, "", 0, false});
    }

    bool 是字符串数组(const std::string& 名称) const {
        auto it = 类型映射.find(名称);
        return it != 类型映射.end() && it->second.种类 == 变量类型种类::数组;
    }

    void 声明常量(const std::string& 名称) {
        auto 信息 = 获取变量类型(名称);
        信息.是常量 = true;
        设置变量类型(名称, 信息);
    }

    bool 是常量(const std::string& 名称) const {
        auto it = 类型映射.find(名称);
        return it != 类型映射.end() && it->second.是常量;
    }

    void 声明枚举(const std::string& 名称, const std::vector<枚举成员>& 成员列表) {
        std::vector<std::string> 成员名列表;
        for (const auto& 成员 : 成员列表) {
            成员名列表.push_back(成员.名称);
            枚举成员映射[名称 + "::" + 成员.名称] = {名称, 成员.值};
        }
        枚举定义映射[名称] = 成员名列表;
        调试打印("[符号表] 声明枚举: '" << 名称 << "' 成员数: " << 成员列表.size());
    }

    bool 是枚举(const std::string& 名称) const {
        return 枚举定义映射.count(名称) > 0;
    }

    const std::vector<std::string>* 获取枚举成员(const std::string& 名称) const {
        auto it = 枚举定义映射.find(名称);
        return (it != 枚举定义映射.end()) ? &it->second : nullptr;
    }

    int 获取枚举成员值(const std::string& 枚举名, const std::string& 成员名) const {
        auto it = 枚举成员映射.find(枚举名 + "::" + 成员名);
        if (it != 枚举成员映射.end()) return it->second.second;
        return -1;
    }

    void 声明全局变量(const std::string& 名称, LLVM值* 值) {
        作用域栈.front()[名称] = 值;
        调试打印("[符号表] 声明全局变量: '" << 名称 << "'");
    }

    LLVM值* 获取变量值(const std::string& 名称) {
        for (auto it = 作用域栈.rbegin(); it != 作用域栈.rend(); ++it) {
            auto found = it->find(名称);
            if (found != it->end()) return found->second;
        }
        return nullptr;
    }

    bool 变量存在(const std::string& 名称) const {
        for (auto it = 作用域栈.rbegin(); it != 作用域栈.rend(); ++it) {
            if (it->count(名称) > 0) return true;
        }
        return false;
    }

    bool 当前作用域存在(const std::string& 名称) const {
        return 作用域栈.back().count(名称) > 0;
    }

    std::string 获取所有变量() const {
        std::string 结果;
        for (const auto& 作用域 : 作用域栈) {
            for (const auto& [名, 值] : 作用域) 结果 += "'" + 名 + "' ";
        }
        return 结果.empty() ? "空" : 结果;
    }

    void 声明数组(const std::string& 名称, LLVM值* 值, int 数组大小) {
        声明变量(名称, 值);
        设置变量类型(名称, {变量类型种类::数组, "", 数组大小, false});
    }

    int 获取数组大小(const std::string& 名称) const {
        auto it = 类型映射.find(名称);
        if (it != 类型映射.end() && it->second.种类 == 变量类型种类::数组) return it->second.数组大小;
        return 0;
    }

    bool 是数组(const std::string& 名称) const {
        auto it = 类型映射.find(名称);
        return it != 类型映射.end() && it->second.种类 == 变量类型种类::数组;
    }

    void 声明函数指针(const std::string& 名称, LLVM函数* 函数,
                    const std::vector<std::string>& 捕获名 = {},
                    const std::vector<LLVM值*>& 捕获地址 = {}) {
        声明变量(名称, 函数);
        设置变量类型(名称, {变量类型种类::函数, "", 0, false});
        闭包信息 信息;
        信息.函数 = 函数;
        信息.捕获变量名 = 捕获名;
        信息.捕获变量地址 = 捕获地址;
        闭包映射[名称] = 信息;
    }

    bool 是函数指针(const std::string& 名称) const {
        return 闭包映射.count(名称) > 0;
    }

    const 闭包信息* 获取闭包信息(const std::string& 名称) const {
        auto it = 闭包映射.find(名称);
        return (it != 闭包映射.end()) ? &it->second : nullptr;
    }

    LLVM函数* 获取函数指针(const std::string& 名称) const {
        auto it = 闭包映射.find(名称);
        return (it != 闭包映射.end()) ? it->second.函数 : nullptr;
    }

    std::string 生成匿名函数名() {
        return "lambda_" + std::to_string(匿名函数计数++);
    }
};

class 作用域守卫 {
private:
    符号表& 表;
public:
    作用域守卫(符号表& t) : 表(t) { 表.进入作用域(); }
    ~作用域守卫() { 表.退出作用域(); }
    作用域守卫(const 作用域守卫&) = delete;
    作用域守卫& operator=(const 作用域守卫&) = delete;
};

#endif // 符号表_H

#ifndef 符号表_H
#define 符号表_H

#include <unordered_map>
#include <vector>
#include <string>
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "公共.h"

struct 闭包信息 {
    llvm::Function* 函数;
    std::vector<std::string> 捕获变量名;
    std::vector<llvm::Value*> 捕获变量地址;
};

class 符号表 {
private:
    std::vector<std::unordered_map<std::string, llvm::Value*>> 作用域栈;
    std::unordered_map<std::string, int> 数组大小映射;
    std::unordered_map<std::string, 闭包信息> 闭包映射;
    std::unordered_map<std::string, bool> 浮点变量映射;  // true = 浮点
    std::unordered_map<std::string, bool> 指针变量映射;  // true = 指针
    std::unordered_map<std::string, std::string> 结构体变量映射;  // 结构体类型名
    std::unordered_map<std::string, bool> 映射变量映射;  // true = 映射
    std::unordered_map<std::string, bool> 字符串数组映射;  // true = 字符串数组
    std::unordered_map<std::string, bool> 常量集合;  // true = 常量（不可赋值）
    std::unordered_map<std::string, std::vector<std::string>> 枚举定义映射;  // 枚举名 -> 成员列表
    std::unordered_map<std::string, std::pair<std::string, int>> 枚举成员映射;  // 枚举名::成员名 -> (枚举名, 值)
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
                浮点变量映射.erase(名称);
                指针变量映射.erase(名称);
                结构体变量映射.erase(名称);
                映射变量映射.erase(名称);
                字符串数组映射.erase(名称);
                常量集合.erase(名称);
            }
            作用域栈.pop_back();
            类型标记作用域栈.pop_back();
        }
    }

    void 重置为全局作用域() {
        // 清理所有内层作用域的类型标记
        while (作用域栈.size() > 1) {
            for (const auto& 名称 : 类型标记作用域栈.back()) {
                浮点变量映射.erase(名称);
                指针变量映射.erase(名称);
                结构体变量映射.erase(名称);
                映射变量映射.erase(名称);
                字符串数组映射.erase(名称);
                常量集合.erase(名称);
            }
            作用域栈.pop_back();
            类型标记作用域栈.pop_back();
        }
    }

    void 声明变量(const std::string& 名称, llvm::Value* 值) {
        作用域栈.back()[名称] = 值;
        调试打印("[符号表] 声明变量: '" << 名称 << "'（作用域深度 " << 作用域栈.size() << "）");
    }

    void 设置浮点变量(const std::string& 名称) {
        if (!浮点变量映射.count(名称)) 类型标记作用域栈.back().push_back(名称);
        浮点变量映射[名称] = true;
    }

    bool 是浮点变量(const std::string& 名称) const {
        return 浮点变量映射.count(名称) > 0 && 浮点变量映射.at(名称);
    }

    void 设置指针变量(const std::string& 名称) {
        if (!指针变量映射.count(名称)) 类型标记作用域栈.back().push_back(名称);
        指针变量映射[名称] = true;
    }

    bool 是指针变量(const std::string& 名称) const {
        return 指针变量映射.count(名称) > 0 && 指针变量映射.at(名称);
    }

    void 设置结构体变量(const std::string& 名称, const std::string& 类型名) {
        if (!结构体变量映射.count(名称)) 类型标记作用域栈.back().push_back(名称);
        结构体变量映射[名称] = 类型名;
    }

    const std::string& 获取结构体类型(const std::string& 名称) const {
        static std::string 空;
        auto it = 结构体变量映射.find(名称);
        return (it != 结构体变量映射.end()) ? it->second : 空;
    }

    void 设置映射变量(const std::string& 名称) {
        if (!映射变量映射.count(名称)) 类型标记作用域栈.back().push_back(名称);
        映射变量映射[名称] = true;
    }

    bool 是映射变量(const std::string& 名称) const {
        return 映射变量映射.count(名称) > 0 && 映射变量映射.at(名称);
    }

    void 设置字符串数组(const std::string& 名称) {
        if (!字符串数组映射.count(名称)) 类型标记作用域栈.back().push_back(名称);
        字符串数组映射[名称] = true;
    }

    bool 是字符串数组(const std::string& 名称) const {
        return 字符串数组映射.count(名称) > 0 && 字符串数组映射.at(名称);
    }

    void 声明常量(const std::string& 名称) {
        if (!常量集合.count(名称)) 类型标记作用域栈.back().push_back(名称);
        常量集合[名称] = true;
    }

    bool 是常量(const std::string& 名称) const {
        return 常量集合.count(名称) > 0 && 常量集合.at(名称);
    }

    void 声明枚举(const std::string& 名称, const std::vector<std::string>& 成员列表) {
        枚举定义映射[名称] = 成员列表;
        for (int i = 0; i < static_cast<int>(成员列表.size()); i++) {
            枚举成员映射[名称 + "::" + 成员列表[i]] = {名称, i};
        }
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

    void 声明全局变量(const std::string& 名称, llvm::Value* 值) {
        作用域栈.front()[名称] = 值;
        调试打印("[符号表] 声明全局变量: '" << 名称 << "'");
    }

    llvm::Value* 获取变量值(const std::string& 名称) {
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

    void 声明数组(const std::string& 名称, llvm::Value* 值, int 大小) {
        声明变量(名称, 值);
        数组大小映射[名称] = 大小;
    }

    int 获取数组大小(const std::string& 名称) const {
        auto it = 数组大小映射.find(名称);
        return (it != 数组大小映射.end()) ? it->second : 0;
    }

    bool 是数组(const std::string& 名称) const {
        return 数组大小映射.count(名称) > 0;
    }

    void 声明函数指针(const std::string& 名称, llvm::Function* 函数,
                    const std::vector<std::string>& 捕获名 = {},
                    const std::vector<llvm::Value*>& 捕获地址 = {}) {
        声明变量(名称, 函数);
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

    llvm::Function* 获取函数指针(const std::string& 名称) const {
        auto it = 闭包映射.find(名称);
        return (it != 闭包映射.end()) ? it->second.函数 : nullptr;
    }

    std::string 生成匿名函数名() {
        return "lambda_" + std::to_string(匿名函数计数++);
    }
};

#endif // 符号表_H

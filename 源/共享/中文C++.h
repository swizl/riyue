#如果未定义 中文CPP_H
#定义 中文CPP_H

// 标准库头文件
#包含 <string>
#包含 <vector>
#包含 <map>
#包含 <memory>
#包含 <iostream>
#包含 <fstream>
#包含 <sstream>
#包含 <functional>
#包含 <stdexcept>
#包含 <unordered_map>
#包含 <unordered_set>
#包含 <set>
#包含 <stack>
#包含 <algorithm>
#包含 <cassert>
#包含 <cstdio>
#包含 <cstdlib>
#包含 <cstring>
#包含 <cctype>
#包含 <cstdint>
#包含 <cmath>
#包含 <system_error>

// ── std 类型别名 ──
取用 文本 = std::string;
取用 宽文本 = std::wstring;
取用 输出流 = std::ostream;
取用 输入流 = std::istream;
取用 输入输出流 = std::iostream;
取用 文件输入流 = std::ifstream;
取用 文件输出流 = std::ofstream;
取用 文本流 = std::stringstream;
取用 输入文本流 = std::istringstream;
取用 输出文本流 = std::ostringstream;

范型<类型名_ T>
取用 数组向量 = std::vector<T>;

范型<类型名_ K, 类型名_ V>
取用 哈希映射 = std::unordered_map<K, V>;

范型<类型名_ K, 类型名_ V>
取用 有序映射 = std::map<K, V>;

范型<类型名_ T>
取用 哈希集合 = std::unordered_set<T>;

范型<类型名_ T>
取用 有序集合 = std::set<T>;

范型<类型名_ T>
取用 独占指针 = std::unique_ptr<T>;

范型<类型名_ T>
取用 共享指针 = std::shared_ptr<T>;

范型<类型名_ T>
取用 函数对象 = std::function<T>;

范型<类型名_ T>
取用 栈 = std::stack<T>;

范型<类型名_ T>
取用 初始化列表 = std::initializer_list<T>;

取用 运行时异常 = std::runtime_error;
取用 逻辑异常 = std::logic_error;
取用 标准异常 = std::exception;
取用 系统错误 = std::system_error;
取用 错误码 = std::error_code;

// ── std 函数 ──
内联 文本 转为文本(整数型 值) { 归返 std::to_string(值); }
内联 文本 转为文本(双精度型 值) { 归返 std::to_string(值); }
内联 文本 转为文本(长整型 值) { 归返 std::to_string(值); }
内联 文本 转为文本(无符号 长整型 值) { 归返 std::to_string(值); }
内联 文本 转为文本(大小类型 值) { 归返 std::to_string(值); }

范型<类型名_ T>
内联 独占指针<T> 创建独占(T* 指针) { 归返 std::unique_ptr<T>(指针); }

范型<类型名_ T, 类型名_... 参数类型>
内联 独占指针<T> 创建独占(参数类型&&... 参数) { 归返 std::make_unique<T>(std::forward<参数类型>(参数)...); }

// ── std 移动语义 ──
#define 移动 std::move
#define 转发 std::forward

// ── 全局流对象 ──
内联 输出流& 标准输出流 = std::cout;
内联 输出流& 标准错误流 = std::cerr;
内联 输入流& 标准输入流 = std::cin;

// ── 流操纵符 ──
内联 输出流& 换行(输出流& 流) { 归返 std::endl(流); }

#结束
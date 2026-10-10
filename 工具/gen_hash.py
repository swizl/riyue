#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成 T5 权威 token id 表与「真完美哈希」查找实现。

单一来源：本文件中的 关键字表 / 文本标志表。
id 分配（一经确定不可再变，列为规范）：
    关键字   1 – 99
    文本标志 101 – 199
    字面量类别 201+（见 源/前端/词法分析器.h，由本表之外的固定编号）
    未知     0

产出：
    源/共享/标记标识表.h   完美哈希桶表（仅由 源/运行时/运行时辅助.c 包含）
    源/共享/标记标识.h     查找函数声明（extern "C"，宿主与自举共用）
"""

import sys
import os

# —— 权威表：顺序即 id（关键字 id = 下标 + 1；文本标志 id = 下标 + 101）——
关键字表 = [
    "函数", "空", "如果", "否则", "否则如果", "循环", "当", "做", "到",
    "中断", "继续", "变量", "常量", "返回", "匹配", "遍历", "导入",
    "整数", "浮点", "布尔", "字符串", "结构体", "枚举", "真", "假",
    "方法", "尝试", "捕获", "抛出", "最终", "协程", "让出", "入",
]

文本标志表 = [
    "(", ")", "{", "}", "[", "]", ";", ":", ".", ",", "$",
    "+", "-", "*", "/", "%", "!", "!=", "<", "<=", ">", ">=",
    "&", "|", "^", "?", "=",
    "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
    "<|", ">|", "<|=", ">|=", "&&", "||", "??", "?=", "+|", "-|", "=>", "...", "::",
]

关键字id起 = 1
文本标志id起 = 101
FNV偏移 = 2166136261
FNV质数 = 16777619


def fnv1a(s):
    h = FNV偏移
    for b in s.encode('utf-8'):
        h ^= b
        h = (h * FNV质数) & 0xFFFFFFFF
    return h


def 找完美桶(表):
    """搜索无冲突模数与一个可用的（偏移, 质数）组合，构造完美哈希桶。"""
    质数候选 = [16777619, 16777639, 16777643, 16777657, 16777667, 16777669]
    偏移候选 = [2166136261, 2166136301, 2166136311, 2166136331]
    起 = len(表)
    for 质数 in 质数候选:
        for 偏移 in 偏移候选:
            def h(s):
                x = 偏移
                for b in s.encode('utf-8'):
                    x ^= b
                    x = (x * 质数) & 0xFFFFFFFF
                return x
            for 模 in range(起, 起 + 4096):
                桶 = [-1] * 模
                冲突 = False
                for i, s in enumerate(表):
                    p = h(s) % 模
                    if 桶[p] != -1:
                        冲突 = True
                        break
                    桶[p] = i
                if not 冲突:
                    return 模, 桶, 偏移, 质数
    raise SystemExit("找不到完美哈希参数")


def C字符串(s):
    out = ['"']
    for ch in s:
        if ch == '"':
            out.append('\\"')
        elif ch == '\\':
            out.append('\\\\')
        else:
            out.append(ch)
    out.append('"')
    return ''.join(out)


def 生成表头():
    L = []
    L.append("// 由 工具/gen_hash.py 自动生成，勿手动编辑")
    L.append("// 权威 token id 表 + 完美哈希桶（FNV-1a mod 模数，桶内单次比较）")
    L.append("#ifndef 标记标识表_H")
    L.append("#define 标记标识表_H")
    L.append("")

    for 名, 表, id起 in (("关键字", 关键字表, 关键字id起),
                        ("文本标志", 文本标志表, 文本标志id起)):
        模, 桶, 偏移, 质数 = 找完美桶(表)
        L.append("// %s：id = 下标 + %d" % (名, id起))
        L.append("#define %s表长 %d" % (名, len(表)))
        L.append("static const char* const %s表[%s表长] = {" % (名, 名))
        L.append("    " + ", ".join(C字符串(s) for s in 表) + ",")
        L.append("};")
        L.append("#define %s桶偏移 %du" % (名, 偏移))
        L.append("#define %s桶质数 %du" % (名, 质数))
        L.append("#define %s桶长 %d" % (名, 模))
        L.append("static const signed char %s桶[%s桶长] = {" % (名, 名))
        行 = []
        for i in range(0, 模, 24):
            行.append("    " + ", ".join(str(x) for x in 桶[i:i + 24]) + ",")
        L.extend(行)
        L.append("};")
        L.append("")

    L.append("#endif // 标记标识表_H")
    return '\n'.join(L)


def 生成声明头():
    return '\n'.join([
        "// 由 工具/gen_hash.py 自动生成，勿手动编辑",
        "#ifndef 标记标识_H",
        "#define 标记标识_H",
        "#ifdef __cplusplus",
        'extern "C" {',
        "#endif",
        "// 完美哈希查找：命中返回 token id（关键字 1-99 / 文本标志 101-199），否则返回 0",
        "int 关键字标识(const char* 标识符);",
        "int 文本标志标识(const char* 文本);",
        "#ifdef __cplusplus",
        "}",
        "#endif",
        "#endif // 标记标识_H",
    ])


def main():
    根 = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "..")
    共享 = os.path.join(根, "源", "共享")
    os.makedirs(共享, exist_ok=True)
    with open(os.path.join(共享, "标记标识表.h"), "w", encoding="utf-8") as f:
        f.write(生成表头())
    with open(os.path.join(共享, "标记标识.h"), "w", encoding="utf-8") as f:
        f.write(生成声明头())
    print("已生成: 源/共享/标记标识表.h, 源/共享/标记标识.h")


if __name__ == "__main__":
    main()
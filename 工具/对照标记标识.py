#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""T5 对照脚本：宿主权威表 ↔ 自举运行时 的 token id 序列一致性校验。

权威来源是 工具/gen_hash.py 中的 关键字表 / 文本标志表（= 由它生成的
源/共享/标记标识表.h，宿主与自举共用）。本脚本由权威表生成一份 日月 探针，
用自举编译器 日月.exe 编译运行，读取自举侧 关键字编号 / 文本标志编号
（二者在运行时委托给同一 C 实现）返回的 id，逐一比对。

用法：python 工具/对照标记标识.py
"""

import importlib.util
import os
import subprocess
import sys

sys.dont_write_bytecode = True

根 = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
阶段 = os.path.join(根, "阶段")


def 载入生成器():
    spec = importlib.util.spec_from_file_location(
        "gen_hash", os.path.join(根, "工具", "gen_hash.py"))
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


def 生成探针(样例):
    L = ["// 由 工具/对照标记标识.py 生成，勿手动编辑", "函数 主程序 {"]
    for i, s in enumerate(样例):
        L.append('    ("%s")关键字编号 = v;' % s)
        L.append('    如果 (v ?= 0) { ("%s")文本标志编号 = v; }' % s)
        L.append("    (%d)打印;" % i)
        L.append("    (v)打印;")
    L.append("}")
    return "\n".join(L) + "\n"


def 主要():
    g = 载入生成器()
    样例 = []
    期望 = []
    for i, s in enumerate(g.关键字表):
        样例.append(s)
        期望.append(i + 1)
    for i, s in enumerate(g.文本标志表):
        样例.append(s)
        期望.append(101 + i)
    for s in ["绝对不是关键字", "abc", "函数式", "（", "&&&"]:
        样例.append(s)
        期望.append(0)

    os.makedirs(阶段, exist_ok=True)
    探针源 = os.path.join(阶段, "_对照探针.心")
    相对源 = os.path.join("阶段", "_对照探针.心")
    相对exe = os.path.join("阶段", "_对照探针.exe")
    if os.path.exists(os.path.join(根, 相对exe)):
        os.remove(os.path.join(根, 相对exe))
    with open(探针源, "w", encoding="utf-8") as f:
        f.write(生成探针(样例))

    r = subprocess.run([os.path.join(根, "日月.exe"), 相对源, 相对exe],
                       cwd=根, capture_output=True)
    if r.returncode != 0 or not os.path.exists(os.path.join(根, 相对exe)):
        print("探针编译失败:", r.stdout.decode("utf-8", "replace"),
              r.stderr.decode("utf-8", "replace"))
        return 1

    out = subprocess.run([相对exe], cwd=根, capture_output=True)
    行 = out.stdout.decode("utf-8", "replace").split()
    号 = [int(x) for x in 行]

    失败 = 0
    if len(号) != 2 * len(样例):
        print("输出条数不符: 得到", len(号), "期望", 2 * len(样例))
        return 1
    for k in range(len(样例)):
        i, v = 号[2 * k], 号[2 * k + 1]
        if i != k:
            print("序号错位:", i, k)
            失败 += 1
        elif v != 期望[k]:
            print("id 不符: 样例=%r 自举=%d 权威=%d" % (样例[k], v, 期望[k]))
            失败 += 1
    print("对照完成：样例 %d 个，失败项 = %d" % (len(样例), 失败))
    return 1 if 失败 else 0


if __name__ == "__main__":
    sys.exit(主要())
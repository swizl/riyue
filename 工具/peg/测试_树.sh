#!/bin/bash
# S4.2c 表达式链语义动作回归：生成树模式解析示例，校验 AST 形状
cd /d/src/riyue/riyue || exit 1
export MSYS2_ARG_CONV_EXCL='*'
export PATH=/usr/bin:/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:$PATH

./日月.exe 工具/peg/主程序.心 阶段/peg_cli.exe >/dev/null 2>&1 || { echo 构建peg_cli失败; exit 1; }
./阶段/peg_cli.exe 生成树 工具/日月/日月.文法 工具/peg/gen_tree.心 >/dev/null 2>&1 || { echo 生成树失败; exit 1; }
./日月.exe 工具/peg/gen_tree.心 阶段/日月树.exe >/dev/null 2>&1 || { echo 编译树解析器失败; exit 1; }

out=$(./阶段/日月树.exe 工具/日月/日月.文法 工具/peg/示例/ast_表达式.心)
want='(二元运算 +(整数 1)(二元运算 *(整数 2)(整数 3)))'
rm -f 工具/peg/gen_tree.心
if echo "$out" | grep -qF "$want"; then
    echo 树模式测试通过
    exit 0
else
    echo 树模式测试失败
    echo "$out"
    exit 1
fi

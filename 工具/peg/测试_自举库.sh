#!/usr/bin/env bash
set -e
cd /d/src/riyue/riyue
export MSYS2_ARG_CONV_EXCL='*'
export PATH=/usr/bin:/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:$PATH

./日月.exe 工具/peg/主程序.心 阶段/peg_cli.exe >/dev/null
阶段/peg_cli.exe 生成自举库 工具/日月/日月.文法 自举/解析器.心
./日月.exe 工具/peg/测试_自举库.心 阶段/自举库测试.exe >/dev/null
阶段/自举库测试.exe 工具/peg/示例/ast_表达式.心 > 阶段/自举库_out.txt
EXPECT='(程序(函数(标识符 主程序)(代码块(如果(如果头(二元运算 +(整数 1)(二元运算 *(整数 2)(整数 3))))(代码块(返回(整数 0))))(表达式语句(二元运算 +(整数 1)(二元运算 *(整数 2)(整数 3)))))))'
GOT=$(cat 阶段/自举库_out.txt)
if [ "$GOT" = "$EXPECT" ]; then
  echo 自举库测试通过
else
  echo 自举库测试失败
  echo "got: $GOT"
  exit 1
fi
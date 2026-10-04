#!/bin/bash
# S4.1 库模式回归：生成解析库（库初始化(文法文本) + 解析(源)→CST），
#   与解释器（peg_cli 识别）逐例对比全部 示例/*.心；
#   以及 host/boot 生成源码逐字节一致性。
# 与 测试.sh（玩具文法）、测试_日月.sh（CLI 生成模式）互补。
cd /d/src/riyue/riyue || exit 1
export MSYS2_ARG_CONV_EXCL='*'
export PATH=/usr/bin:/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:$PATH

WENFA=工具/日月/日月.文法
GEN=工具/peg/生成_日月_库.心
BOOTGEN=工具/peg/生成_日月_库_boot.心
fail=0

echo "== host 链 =="
./日月.exe 工具/peg/主程序.心 阶段/peg_cli.exe || { echo "peg_cli 编译失败"; exit 1; }
./阶段/peg_cli.exe 生成库 "$WENFA" "$GEN" || { echo "生成库失败"; exit 1; }
./日月.exe 工具/peg/测试_库.心 阶段/库测试.exe || { echo "库测试编译失败"; exit 1; }

echo "== 解释器 vs 库（遍历 示例/*.心）=="
ok=0; bad=0
for f in 示例/*.心; do
  a=$(./阶段/peg_cli.exe 识别 "$WENFA" "$f" | tail -n 1)
  b=$(./阶段/库测试.exe "$f")
  if [ "$a" = "$b" ]; then
    ok=$((ok + 1))
  else
    bad=$((bad + 1)); fail=1; echo "不一致: $f"
  fi
done
echo "一致: $ok  不一致: $bad"

if [ -x 阶段/一级.exe ]; then
  echo "== 自举链：host/boot 生成一致性 =="
  [ -f 阶段/运行时.o ] || clang -O0 -c 源/运行时/运行时辅助.c -o 阶段/运行时.o
  ./阶段/一级.exe 工具/peg/主程序.心 阶段/cli_boot.ll && \
  llvm-as 阶段/cli_boot.ll -o 阶段/cli_boot.bc && \
  llc 阶段/cli_boot.bc -mtriple=x86_64-w64-windows-gnu -filetype=obj -o 阶段/cli_boot.o && \
  clang 阶段/cli_boot.o 阶段/运行时.o -o 阶段/cli_boot.exe -lws2_32 -lm -lstdc++ || { echo "boot 链接失败"; fail=1; }
  ./阶段/cli_boot.exe 生成库 "$WENFA" "$BOOTGEN" || { echo "boot 生成失败"; fail=1; }
  diff "$GEN" "$BOOTGEN" || { echo "host/boot 生成不一致"; fail=1; }
else
  echo "（跳过自举链：阶段/一级.exe 不存在）"
fi

rm -f "$GEN" "$BOOTGEN"
[ "$fail" = 0 ] && echo "库模式测试通过" || echo "库模式测试有失败"
exit $fail

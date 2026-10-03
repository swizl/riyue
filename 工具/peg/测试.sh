#!/bin/bash
# PEG 内核 host/boot 一致性与端到端测试
cd /d/src/riyue/riyue || exit 1
export MSYS2_ARG_CONV_EXCL='*'
export PATH=/usr/bin:/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:$PATH

WENFA=工具/peg/示例/算术.文法
SHURU=工具/peg/示例/算术.输入
QIWANG='(程序 (加减 (乘除 (原子 1)) + (乘除 (原子 2) * (原子 3))))'
fail=0

echo "== host 链 =="
./日月.exe 工具/peg/主程序.心 阶段/peg_cli.exe || { echo "host CLI 编译失败"; exit 1; }
out=$(./阶段/peg_cli.exe 识别 "$WENFA" "$SHURU" | tail -n 1)
echo "识别: $out"
[ "$out" = "$QIWANG" ] || { echo "识别 CST 不符"; fail=1; }
./阶段/peg_cli.exe 生成 "$WENFA" 工具/peg/生成_测试.心 || { echo "生成失败"; fail=1; }
./日月.exe 工具/peg/生成_测试.心 阶段/生成_测试.exe || { echo "生成解析器编译失败"; fail=1; }
out2=$(./阶段/生成_测试.exe "$WENFA" "$SHURU")
echo "生成解析器: $out2"
[ "$out2" = "$QIWANG" ] || { echo "生成解析器 CST 不符"; fail=1; }

if [ -x 阶段/一级.exe ]; then
  echo "== 自举链 =="
  ./阶段/一级.exe 工具/peg/主程序.心 阶段/cli_boot.ll || { echo "boot CLI 生成 .ll 失败"; fail=1; }
  llvm-as 阶段/cli_boot.ll -o 阶段/cli_boot.bc && \
  llc 阶段/cli_boot.bc -mtriple=x86_64-w64-windows-gnu -filetype=obj -o 阶段/cli_boot.o && \
  clang 阶段/cli_boot.o 阶段/运行时.o -o 阶段/cli_boot.exe -lws2_32 -lm -lstdc++ || { echo "boot CLI 链接失败"; fail=1; }
  out3=$(./阶段/cli_boot.exe 识别 "$WENFA" "$SHURU" | tail -n 1)
  echo "boot 识别: $out3"
  [ "$out3" = "$QIWANG" ] || { echo "boot 识别 CST 不符"; fail=1; }
  ./阶段/cli_boot.exe 生成 "$WENFA" 工具/peg/生成_测试_boot.心 || { echo "boot 生成失败"; fail=1; }
  diff 工具/peg/生成_测试.心 工具/peg/生成_测试_boot.心 || { echo "host/boot 生成不一致"; fail=1; }
else
  echo "（跳过自举链：阶段/一级.exe 不存在）"
fi

rm -f 工具/peg/生成_测试.心 工具/peg/生成_测试_boot.心
[ "$fail" = 0 ] && echo "PEG 测试全部通过" || echo "PEG 测试有失败"
exit $fail

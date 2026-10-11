#!/bin/bash
# 日月自举不动点验证（护栏）。
# 链：种子 引导.exe 编译 自举/主入口.心 → 阶段/一级.exe；
#     一级 编译自身 → 阶段/中间码1.ll；经 llvm-as/llc + 运行时 用 ld.lld 链接 → 阶段/二级.exe；
#     二级 再编译自身 → 阶段/中间码2.ll。
# 判定：中间码1.ll 与 中间码2.ll 必须完全一致（哈希相同即自举闭环成立）。
# 证据（两份中间码的字节数与 SHA256，以及不一致时的 diff）固化到 阶段/不动点.log。
export MSYS2_ARG_CONV_EXCL='*'
export PATH=/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:/usr/bin:/bin:$PATH
cd /d/src/riyue/riyue || exit 9

LOG=阶段/不动点.log
mkdir -p 阶段
: > "$LOG"

记录() { echo "$@" | tee -a "$LOG"; }

rm -f 阶段/中间码1.ll 阶段/中间码2.ll 阶段/一级.exe 阶段/二级.exe

./引导.exe 自举/主入口.心 阶段/一级.exe      || { 记录 "失败: 引导.exe 编译自举源码"; exit 1; }
./阶段/一级.exe 自举/主入口.心 阶段/中间码1.ll || { 记录 "失败: 一级.exe 生成 中间码1.ll"; exit 2; }
llvm-as 阶段/中间码1.ll -o 阶段/中间码1.bc  || { 记录 "失败: llvm-as 汇编 中间码1.ll"; exit 3; }
llc -mtriple=x86_64-w64-windows-gnu -filetype=obj 阶段/中间码1.bc -o 阶段/中间码1.o || { 记录 "失败: llc 生成目标文件"; exit 4; }
clang -O0 -finput-charset=UTF-8 -fexec-charset=UTF-8 -c 源/运行时/运行时辅助.c -o 阶段/运行时.o || { 记录 "失败: 编译运行时"; exit 5; }
cp 'C:/tools/msys64/mingw64/lib/crt2.o' 阶段/启动.o || { 记录 "失败: 复制 crt2.o"; exit 6; }
ld.lld -m i386pep 阶段/启动.o 阶段/中间码1.o 阶段/运行时.o -o 阶段/二级.exe \
  -LC:/tools/msys64/mingw64/lib \
  -LC:/tools/msys64/mingw64/lib/gcc/x86_64-w64-mingw32/16.1.0 \
  -lmingw32 -lmingwex -lmsvcrt -lgcc -lmoldname -lws2_32 -lm -ladvapi32 -lshell32 -luser32 -lkernel32 \
  || { 记录 "失败: ld.lld 链接 二级.exe"; exit 7; }
./阶段/二级.exe 自举/主入口.心 阶段/中间码2.ll || { 记录 "失败: 二级.exe 生成 中间码2.ll"; exit 8; }

# 注意：bash 变量名只能是 ASCII，哈希/字节数用 ASCII 名；注释与输出仍为中文。
H1=$(sha256sum 阶段/中间码1.ll | cut -d' ' -f1)
H2=$(sha256sum 阶段/中间码2.ll | cut -d' ' -f1)
B1=$(stat -c %s 阶段/中间码1.ll)
B2=$(stat -c %s 阶段/中间码2.ll)
记录 "中间码1.ll  字节=$B1  SHA256=$H1"
记录 "中间码2.ll  字节=$B2  SHA256=$H2"

if [ "$H1" = "$H2" ]; then
    记录 "门禁: 通过（不动点成立，两份中间码完全一致）"
    exit 0
fi

记录 "门禁: 未通过（不动点被破坏，两份中间码不一致）"
记录 "--- diff 中间码1.ll 中间码2.ll ---"
diff 阶段/中间码1.ll 阶段/中间码2.ll >> "$LOG" 2>&1
exit 1
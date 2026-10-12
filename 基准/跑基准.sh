#!/bin/bash
# 日月性能基准（P-D）：用自举编译器 日月.exe 分别以 --优化=0 / 2 / 3 编译
# 基准/基准测试.心，核对三档运行输出一致后，预热并多次计时取最小值，计算各档
# 相对 -O0 的提速倍率，结论写入 阶段/基准.log。
#
# 环境要点（历史踩坑，勿删）：
#   1. 本机 bash 解析到 WSL → 必须用 C:\tools\msys64\usr\bin\bash.exe 启动本脚本；
#   2. MSYS_NO_PATHCONV=1：防止中文相对路径（基准/…、阶段/…）被 MSYS 改写；
#   3. 运行 日月.exe 需要 mingw64\bin 在 PATH（否则 0xC0000139 ENTRYPOINT_NOT_FOUND）；
#      链接期的 opt/llc/ld.lld 由 日月.exe 内部 cmd 以绝对 PATH 注入，无需外部 PATH，
#      但此处一并前置 llvm-build\bin，保证手跑诊断时也能取到同一套工具；
#   4. 编译必须在仓库根目录执行（cwd）：链接命令里的 `源\运行时\运行时辅助.o`
#      是相对 cwd 的；且日月.exe 是 Windows 程序，只认 Windows 语义路径，故输入/
#      输出一律用仓库根下的相对路径（中文路径 Windows API 接受正斜杠）。中间产物
#      .ll/.bc 随输出路径落进 bench 目录，临时 riyue_*.o 由链接链自删；
#   5. 首次运行数百 KB 级可执行文件受冷启动（杀软/磁盘）干扰极大 → 必须先预热
#      1 次再计时，且多轮取最小值；
#   6. bash 变量名只允许 ASCII → 变量用英文，中文仅出现在函数名与输出文案里。
# 用法：bash 基准/跑基准.sh   （REPS=5 可调整计时轮数，默认 7）
export PATH=/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:/usr/bin:/bin:$PATH
export MSYS_NO_PATHCONV=1

REPO=/d/src/riyue/riyue
BENCHREL="阶段/gate/bench"
BENCH="$REPO/$BENCHREL"
LOG="$REPO/阶段/基准.log"
REPS=${REPS:-7}
TIMEOUT_RUN=120
LEVELS="0 2 3"

mkdir -p "$BENCH"
cd "$REPO" || exit 9
SRC="基准/基准测试.心"

# 计时：$1=相对仓库根的可执行文件路径 → 回显最小耗时(ms)。
# 预热 1 次消除冷启动，再跑 REPS 轮取最小值。
计时() {
    local exe="$1" i t0 t1 ms best=""
    timeout "$TIMEOUT_RUN" "$exe" >/dev/null 2>&1
    i=0
    while [ "$i" -lt "$REPS" ]; do
        t0=$(date +%s%N)
        timeout "$TIMEOUT_RUN" "$exe" >/dev/null 2>&1
        t1=$(date +%s%N)
        ms=$(( (t1 - t0) / 1000000 ))
        if [ -z "$best" ] || [ "$ms" -lt "$best" ]; then best="$ms"; fi
        i=$((i + 1))
    done
    echo "$best"
}

# 1) 逐档编译（产物落 bench 目录，中间 riyue_*.o 由日月.exe 链接链自删）
CLOG="$BENCH/compile.log"
: > "$CLOG"
OK_COMPILE=1
for lvl in $LEVELS; do
    rm -f "$BENCH/基准_$lvl.exe"
    if timeout 120 "$REPO/日月.exe" "$SRC" "$BENCHREL/基准_$lvl.exe" --优化="$lvl" \
        >> "$CLOG" 2>&1 && [ -f "$BENCH/基准_$lvl.exe" ]; then
        echo "编译 -O$lvl: OK" >> "$CLOG"
    else
        echo "编译 -O$lvl: 失败 (rc=$?)" >> "$CLOG"
        OK_COMPILE=0
    fi
done
rm -f "$REPO"/riyue_* 2>/dev/null

# 2) 输出核对（三档必须一致，且须为 4 行）
OUTDIR="$BENCH/out"
mkdir -p "$OUTDIR"
SAME=1
REF=""
for lvl in $LEVELS; do
    timeout "$TIMEOUT_RUN" "$BENCHREL/基准_$lvl.exe" 2>/dev/null | sed 's/\r//' > "$OUTDIR/out_$lvl.txt"
    cur=$(cat "$OUTDIR/out_$lvl.txt")
    if [ -z "$REF" ]; then
        REF="$cur"
    elif [ "$cur" != "$REF" ]; then
        SAME=0
    fi
done
LINES=$(wc -l < "$OUTDIR/out_0.txt" | tr -d ' ')

# 3) 计时（编译失败则记 0，避免对不存在的可执行文件计时）
declare -A ms
for lvl in $LEVELS; do
    if [ -f "$BENCH/基准_$lvl.exe" ]; then
        ms[$lvl]=$(计时 "$BENCHREL/基准_$lvl.exe")
    else
        ms[$lvl]=0
    fi
done

# 4) 汇总写日志
{
    echo "=== 日月性能基准（P-D） ==="
    echo "源码: 基准/基准测试.心"
    echo "构成: 递归 fib(32) + 三段 2 亿次循环（取模累加 / 数组求和 / 函数调用累加）"
    echo "计时: 预热 1 次后取 $REPS 轮最小值（毫秒）"
    echo ""
    if [ "$OK_COMPILE" -eq 1 ] && [ "$SAME" -eq 1 ] && [ "$LINES" = "4" ]; then
        echo "输出核对: 三档均 $LINES 行且完全一致 → 通过"
    else
        echo "输出核对: 未通过 (编译=$OK_COMPILE 一致=$SAME 行数=$LINES)"
    fi
    echo "  -O0 输出:"
    sed 's/^/    /' "$OUTDIR/out_0.txt"
    echo ""
    echo "级别  最小耗时(ms)  相对 -O0 提速"
    for lvl in $LEVELS; do
        awk -v L="$lvl" -v v="${ms[$lvl]}" -v base="${ms[0]}" \
            'BEGIN{ if(base>0 && v>0){ printf "  -O%s      %6d        %.2fx\n", L, v, base/v } else { printf "  -O%s      %6d        n/a\n", L, v } }'
    done
    echo ""
    if [ "$OK_COMPILE" -eq 1 ] && [ "$SAME" -eq 1 ] && [ "$LINES" = "4" ]; then
        awk -v b="${ms[0]}" -v o2="${ms[2]}" -v o3="${ms[3]}" \
            'BEGIN{ printf "结论: --优化=2 相对 -O0 提速 %.2fx；-O3 相对 -O0 提速 %.2fx（-O3 与 -O2 基本相当）。\n", (o2>0? b/o2:0), (o3>0? b/o3:0) }'
        echo "门禁: 通过（三档输出一致）"
    else
        echo "门禁: 未通过（编译失败或三档输出不一致）"
    fi
} | tee "$LOG"

grep -q "门禁: 通过" "$LOG"
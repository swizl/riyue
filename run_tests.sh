#!/bin/bash
# 日月端到端回归：遍历 示例/*.心，用自举编译器 日月.exe 编译并运行，
# 与 示例/<名>.期望 比对（无 .期望 的用例只要编译成功且退出码为 0 即通过）。
# 结果同时打印到控制台并固化到 阶段/回归.log；末尾按门禁判定退出码。
# 支持 Windows(MSYS2/MinGW) 与 Linux 两个宿主：工具链一律取自 riyue 源码树
# 构建的本地化 LLVM（Windows → llvm-build，Linux → llvm-build-linux）。
cd "$(dirname "$0")" || exit 9

case "$(uname -s)" in
    Linux)
        export PATH=/usr/bin:/bin:$PATH
        LLVM_BIN=/mnt/d/src/riyue/llvm/llvm-build-linux/bin
        RT_LIBS="-lpthread -ldl -lm"
        OUT_EXE=./out_t
        SELFTEST_EXE=./_rt_selftest.exe
        C_SHARED_FLAGS="-fPIC"
        ;;
    *)
        export PATH=/usr/bin:/bin:/mingw64/bin:$PATH
        export MSYS_NO_PATHCONV=1
        LLVM_BIN=/d/src/riyue/llvm/llvm-build/bin
        RT_LIBS="-lws2_32 -lm"
        OUT_EXE=./out_t.exe
        SELFTEST_EXE=./_rt_selftest.exe
        C_SHARED_FLAGS=""
        ;;
esac

# 门禁：Pass 不得低于用例总数，Fail 必须为 0（当前 示例/*.心 共 218 个）。
MIN_PASS=218
LOG=阶段/回归.log

PASS=0
FAIL=0
FAIL_LIST=""
TIMEOUT_COMPILE=120
TIMEOUT_RUN=30
COMPILE_RETRY=2

# 清理编译残留的进程和文件，避免文件锁导致后续编译卡住（Windows 专有；
# Linux 无文件锁问题，只清产物即可）
清理残留() {
    if [ "$(uname -s)" != "Linux" ]; then
        /c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe -NoProfile -Command \
            "Get-Process -Name '日月','out_t','clang++','ld','llc' -ErrorAction SilentlyContinue | Stop-Process -Force" 2>/dev/null
    fi
    rm -f out_t out_t.exe out_t.ll out_t.s out_t.o
}

# K7 故障注入自测（C 级）：底层读错误必须返回 NULL，而不是半截数据。
# 计入门禁（失败即 Fail+1），但不计入 Pass（Pass 只统计 示例/*.心）。
运行时读取自测() {
    "$LLVM_BIN/clang" -fuse-ld=lld \
        -finput-charset=UTF-8 -fexec-charset=UTF-8 -O1 \
        测试/运行时读取自测.c -o _rt_selftest.exe $RT_LIBS 2> 阶段/.rt_selftest_cmp
    if [ $? -ne 0 ]; then
        echo "FAIL: 运行时读取自测（编译失败）"
        sed 's/\r//' 阶段/.rt_selftest_cmp
        rm -f _rt_selftest.exe
        return 1
    fi
    $SELFTEST_EXE > 阶段/.rt_selftest_out 2>&1
    rc=$?
    sed 's/\r//' 阶段/.rt_selftest_out
    rm -f _rt_selftest.exe
    [ $rc -eq 0 ] && return 0
    echo "FAIL: 运行时读取自测（退出码 $rc）"
    return 1
}

# C 互操作自测（阶段10 续43）：用本地化 clang 把 C 侧编成 `_cinterop.dll`（Linux 为同名
# ELF 共享库），供 示例/C互操作测试.心 以 `外部 函数` 调用，验证 clang + 日月联合编译路径。
# 计入门禁（失败即 Fail+1），但不计入 Pass（Pass 只统计 示例/*.心）。
构建C互操作库() {
    "$LLVM_BIN/clang" -fuse-ld=lld -finput-charset=UTF-8 -fexec-charset=UTF-8 -O2 $C_SHARED_FLAGS -shared \
        示例/C互操作/工具.c -o _cinterop.dll 2> 阶段/.cinterop_cmp
    if [ $? -ne 0 ]; then
        echo "FAIL: C互操作库（编译失败）"
        sed 's/\r//' 阶段/.cinterop_cmp
        return 1
    fi
    return 0
}

# 链接期附加输入自测（阶段10 续45）：clang 单独编出 .o、llvm-ar 打包成 .a，再用
# `--附加对象=` / `--附加库=` 把两者与 日月 产物一并交给链接器（`--链接库=` 拼 -l）。
# 示例/C互操作/链接用例.心 不定义这两个符号，靠链接期附加输入解析，验证「C → .o/.a → 日月」
# 的混合链接入口。计入门禁（失败即 Fail+1），但不计入 Pass（Pass 只统计 示例/*.心）。
链接互操作自测() {
    "$LLVM_BIN/clang" -finput-charset=UTF-8 -fexec-charset=UTF-8 -O2 -c \
        示例/C互操作/链接加法.c -o _clink_add.o 2> 阶段/.clink_cmp \
        && "$LLVM_BIN/clang" -finput-charset=UTF-8 -fexec-charset=UTF-8 -O2 -c \
            示例/C互操作/链接乘法.c -o _clink_mul.o 2>> 阶段/.clink_cmp \
        && "$LLVM_BIN/llvm-ar" rcs _clink.a _clink_mul.o 2>> 阶段/.clink_cmp \
        && "$LLVM_BIN/clang" -finput-charset=UTF-8 -fexec-charset=UTF-8 -O2 -c \
            示例/C互操作/链接减法.c -o _clink_sub.o 2>> 阶段/.clink_cmp \
        && mkdir -p _clink_lib \
        && "$LLVM_BIN/llvm-ar" rcs _clink_lib/libclink_sub.a _clink_sub.o 2>> 阶段/.clink_cmp
    if [ $? -ne 0 ]; then
        echo "FAIL: 链接互操作（附加对象/归档构建失败）"
        sed 's/\r//' 阶段/.clink_cmp
        rm -rf _clink_add.o _clink_mul.o _clink_sub.o _clink.a _clink_lib
        return 1
    fi
    清理残留
    timeout $TIMEOUT_COMPILE ./日月.exe 示例/C互操作/链接用例.心 out_t \
        --附加对象=./_clink_add.o --附加库=./_clink.a \
        --库路径=./_clink_lib --链接库=clink_sub --链接库=m > /dev/null 2> 阶段/.clink_cmp
    rc=$?
    if [ $rc -ne 0 ]; then
        echo "FAIL: 链接互操作（编译失败，退出码 $rc）"
        sed 's/\r//' 阶段/.clink_cmp
        rm -rf _clink_add.o _clink_mul.o _clink_sub.o _clink.a _clink_lib
        return 1
    fi
    actual=$(timeout $TIMEOUT_RUN $OUT_EXE 2>/dev/null | sed 's/\r//')
    expected=$(cat 示例/C互操作/链接用例.期望)
    rm -rf _clink_add.o _clink_mul.o _clink_sub.o _clink.a _clink_lib
    if [ "$actual" != "$expected" ]; then
        echo "FAIL: 链接互操作（输出不匹配）"
        return 1
    fi
    echo "OK: 链接互操作（--附加对象 / --附加库 / --链接库 / --库路径）"
    return 0
}

# C 头文件导入自测（阶段10 续50）：用 工具/头文件导入.心 从 示例/C互操作/演示库.h
# 生成 `.心` 声明（#define 常量 → 枚举、enum → 枚举、函数原型 → 外部 函数），再由
# 示例/C互操作/头文件用例.心 导入并运行，验证「C 头文件 → 心声明 → 运行期 FFI」链路。
# 计入门禁（失败即 Fail+1），但不计入 Pass（Pass 只统计 示例/*.心）。
头文件导入自测() {
    清理残留
    timeout $TIMEOUT_COMPILE ./日月.exe 工具/头文件导入.心 out_t > /dev/null 2> 阶段/.hdr_cmp
    if [ $? -ne 0 ]; then
        echo "FAIL: 头文件导入（工具编译失败）"
        sed 's/\r//' 阶段/.hdr_cmp
        return 1
    fi
    rm -f 阶段/.hdr导入.心
    $OUT_EXE 示例/C互操作/演示库.h 阶段/.hdr导入.心 > /dev/null 2>> 阶段/.hdr_cmp
    if [ $? -ne 0 ] || [ ! -f 阶段/.hdr导入.心 ]; then
        echo "FAIL: 头文件导入（声明生成失败）"
        sed 's/\r//' 阶段/.hdr_cmp
        return 1
    fi
    清理残留
    timeout $TIMEOUT_COMPILE ./日月.exe 示例/C互操作/头文件用例.心 out_t > /dev/null 2> 阶段/.hdr_cmp
    rc=$?
    if [ $rc -ne 0 ]; then
        echo "FAIL: 头文件导入（用例编译失败，退出码 $rc）"
        sed 's/\r//' 阶段/.hdr_cmp
        rm -f 阶段/.hdr导入.心
        return 1
    fi
    actual=$(timeout $TIMEOUT_RUN $OUT_EXE 2>/dev/null | sed 's/\r//')
    expected=$(cat 示例/C互操作/头文件用例.期望)
    rm -f 阶段/.hdr导入.心
    if [ "$actual" != "$expected" ]; then
        echo "FAIL: 头文件导入（输出不匹配）"
        return 1
    fi
    echo "OK: 头文件导入（#define 常量 / enum / 函数原型 → 外部 函数）"
    return 0
}

# 结构体按值自测（阶段10 续52）：C 聚合 ABI 走「链接期直调」——clang 把 工具.c 编成
# `_cstruct.o`，日月把结构体形参按 Win64/SysV 一致规则折算为同宽整数实参，直接调用
# C 符号（不再经运行期 `外部调用N` 的固定标量转发）。验证 8 字节结构体 {int,int}。
# 计入门禁（失败即 Fail+1），但不计入 Pass（Pass 只统计 示例/*.心）。
结构体按值自测() {
    "$LLVM_BIN/clang" -finput-charset=UTF-8 -fexec-charset=UTF-8 -O2 -c \
        示例/C互操作/工具.c -o _cstruct.o 2> 阶段/.cstruct_cmp
    if [ $? -ne 0 ]; then
        echo "FAIL: 结构体按值（C 侧编译失败）"
        sed 's/\r//' 阶段/.cstruct_cmp
        rm -f _cstruct.o
        return 1
    fi
    清理残留
    timeout $TIMEOUT_COMPILE ./日月.exe 示例/C互操作/结构体按值用例.心 out_t \
        --附加对象=./_cstruct.o > /dev/null 2> 阶段/.cstruct_cmp
    rc=$?
    if [ $rc -ne 0 ]; then
        echo "FAIL: 结构体按值（用例编译失败，退出码 $rc）"
        sed 's/\r//' 阶段/.cstruct_cmp
        rm -f _cstruct.o
        return 1
    fi
    actual=$(timeout $TIMEOUT_RUN $OUT_EXE 2>/dev/null | sed 's/\r//')
    expected=$(cat 示例/C互操作/结构体按值用例.期望)
    rm -f _cstruct.o
    if [ "$actual" != "$expected" ]; then
        echo "FAIL: 结构体按值（输出不匹配）"
        return 1
    fi
    echo "OK: 结构体按值（C 聚合 ABI 链接期直调）"
    return 0
}

# 档位（C++29 profiles 式，阶段10 续66/续67）自测：安全档的结构性禁用与 `免检` 豁免。
# 判据：安全档下违规用例（C013 命令执行 / C015 文件系统写 / C016 网络）须报对应错误码且不产生产物；
# 登记 免检 后须降级为审计行并正常产出；默认（标准）档对同一用例不设限；非法档位取值须报 C014。
档位自测() {
    rm -f 阶段/.档位_禁.ll 阶段/.档位_免.ll 阶段/.档位_常.ll 阶段/.档位_错.ll
    rm -f 阶段/.档位_写.ll 阶段/.档位_写免.ll 阶段/.档位_网.ll 阶段/.档位_网免.ll
    timeout $TIMEOUT_COMPILE ./日月.exe --档位=安全 示例/档位/安全档禁令.心 阶段/.档位_禁.ll \
        > /dev/null 2> 阶段/.档位_禁.err
    if [ -f 阶段/.档位_禁.ll ]; then
        echo "FAIL: 档位（安全档禁令仍产生产物）"
        return 1
    fi
    if ! grep -q "错误\[C013\]" 阶段/.档位_禁.err; then
        echo "FAIL: 档位（安全档禁令未报 C013）"
        sed 's/\r//' 阶段/.档位_禁.err
        return 1
    fi
    timeout $TIMEOUT_COMPILE ./日月.exe --档位=安全 示例/档位/安全档免检.心 阶段/.档位_免.ll \
        > /dev/null 2> 阶段/.档位_免.err
    if [ ! -f 阶段/.档位_免.ll ]; then
        echo "FAIL: 档位（免检后未产出）"
        sed 's/\r//' 阶段/.档位_免.err
        return 1
    fi
    if ! grep -q "审计\[C013\]" 阶段/.档位_免.err; then
        echo "FAIL: 档位（免检未输出审计行）"
        return 1
    fi
    # 续67：C015 文件系统写禁用 + 免检豁免
    timeout $TIMEOUT_COMPILE ./日月.exe --档位=安全 示例/档位/安全档文件写.心 阶段/.档位_写.ll \
        > /dev/null 2> 阶段/.档位_写.err
    if [ -f 阶段/.档位_写.ll ]; then
        echo "FAIL: 档位（安全档文件写仍产生产物）"
        return 1
    fi
    if ! grep -q "错误\[C015\]" 阶段/.档位_写.err; then
        echo "FAIL: 档位（安全档文件写未报 C015）"
        sed 's/\r//' 阶段/.档位_写.err
        return 1
    fi
    timeout $TIMEOUT_COMPILE ./日月.exe --档位=安全 示例/档位/安全档文件写免检.心 阶段/.档位_写免.ll \
        > /dev/null 2> 阶段/.档位_写免.err
    if [ ! -f 阶段/.档位_写免.ll ]; then
        echo "FAIL: 档位（C015 免检后未产出）"
        sed 's/\r//' 阶段/.档位_写免.err
        return 1
    fi
    if ! grep -q "审计\[C015\]" 阶段/.档位_写免.err; then
        echo "FAIL: 档位（C015 免检未输出审计行）"
        return 1
    fi
    # 续67：C016 网络禁用 + 免检豁免
    timeout $TIMEOUT_COMPILE ./日月.exe --档位=安全 示例/档位/安全档联网.心 阶段/.档位_网.ll \
        > /dev/null 2> 阶段/.档位_网.err
    if [ -f 阶段/.档位_网.ll ]; then
        echo "FAIL: 档位（安全档联网仍产生产物）"
        return 1
    fi
    if ! grep -q "错误\[C016\]" 阶段/.档位_网.err; then
        echo "FAIL: 档位（安全档联网未报 C016）"
        sed 's/\r//' 阶段/.档位_网.err
        return 1
    fi
    timeout $TIMEOUT_COMPILE ./日月.exe --档位=安全 示例/档位/安全档联网免检.心 阶段/.档位_网免.ll \
        > /dev/null 2> 阶段/.档位_网免.err
    if [ ! -f 阶段/.档位_网免.ll ]; then
        echo "FAIL: 档位（C016 免检后未产出）"
        sed 's/\r//' 阶段/.档位_网免.err
        return 1
    fi
    if ! grep -q "审计\[C016\]" 阶段/.档位_网免.err; then
        echo "FAIL: 档位（C016 免检未输出审计行）"
        return 1
    fi
    timeout $TIMEOUT_COMPILE ./日月.exe 示例/档位/安全档禁令.心 阶段/.档位_常.ll \
        > /dev/null 2> 阶段/.档位_常.err
    if [ ! -f 阶段/.档位_常.ll ]; then
        echo "FAIL: 档位（标准档误禁）"
        sed 's/\r//' 阶段/.档位_常.err
        return 1
    fi
    timeout $TIMEOUT_COMPILE ./日月.exe --档位=错 示例/档位/安全档禁令.心 阶段/.档位_错.ll \
        > /dev/null 2> 阶段/.档位_错.err
    if ! grep -q "错误\[C014\]" 阶段/.档位_错.err; then
        echo "FAIL: 档位（非法取值未报 C014）"
        return 1
    fi
    rm -f 阶段/.档位_禁.ll 阶段/.档位_免.ll 阶段/.档位_常.ll 阶段/.档位_错.ll
    rm -f 阶段/.档位_写.ll 阶段/.档位_写免.ll 阶段/.档位_网.ll 阶段/.档位_网免.ll
    echo "OK: 档位（安全档 C012/C013/C015/C016 禁用 + 函数内免检豁免）"
    return 0
}

运行回归() {
    if ! 运行时读取自测; then
        FAIL=$((FAIL + 1))
        FAIL_LIST="$FAIL_LIST 运行时读取自测"
    fi
    if ! 构建C互操作库; then
        FAIL=$((FAIL + 1))
        FAIL_LIST="$FAIL_LIST C互操作库(cmp)"
    fi
    if ! 链接互操作自测; then
        FAIL=$((FAIL + 1))
        FAIL_LIST="$FAIL_LIST 链接互操作"
    fi
    if ! 头文件导入自测; then
        FAIL=$((FAIL + 1))
        FAIL_LIST="$FAIL_LIST 头文件导入"
    fi
    if ! 结构体按值自测; then
        FAIL=$((FAIL + 1))
        FAIL_LIST="$FAIL_LIST 结构体按值"
    fi
    if ! 档位自测; then
        FAIL=$((FAIL + 1))
        FAIL_LIST="$FAIL_LIST 档位"
    fi
    for f in 示例/*.心; do
        name=$(basename "$f" .心)
        清理残留
        timeout $TIMEOUT_COMPILE ./日月.exe "$f" out_t 2>/dev/null 1>/dev/null
        rc=$?
        retry=0
        while [ $rc -eq 124 ] && [ $retry -lt $COMPILE_RETRY ]; do
            清理残留
            retry=$((retry + 1))
            echo "RETRY: $name (compile, $retry/$COMPILE_RETRY)"
            timeout $TIMEOUT_COMPILE ./日月.exe "$f" out_t 2>/dev/null 1>/dev/null
            rc=$?
        done
        if [ $rc -eq 124 ]; then
            echo "TIMEOUT: $name (compile)"
            FAIL=$((FAIL + 1))
            FAIL_LIST="$FAIL_LIST $name(cmp_timeout)"
            清理残留
            continue
        fi
        if [ $rc -ne 0 ]; then
            echo "FAIL: $name (compile)"
            FAIL=$((FAIL + 1))
            FAIL_LIST="$FAIL_LIST $name(cmp)"
            清理残留
            continue
        fi
        if [ -f "示例/${name}.期望错" ]; then
            # 期望运行期「被拦截并中止」（如越界）：编译须成功、退出码须非零、
            # 且 stderr（去 \r / 去空行）须等于 .期望错 内容。
            timeout $TIMEOUT_RUN $OUT_EXE > 阶段/.rt_out 2> 阶段/.rt_err
            rc_run=$?
            actual_err=$(sed 's/\r//' 阶段/.rt_err | sed '/^[[:space:]]*$/d')
            expected_err=$(cat "示例/${name}.期望错")
            if [ $rc_run -eq 124 ]; then
                echo "TIMEOUT: $name (run)"
                FAIL=$((FAIL + 1))
                FAIL_LIST="$FAIL_LIST $name(rt_timeout)"
            elif [ $rc_run -eq 0 ]; then
                echo "FAIL: $name (期望运行期中止，但正常退出)"
                FAIL=$((FAIL + 1))
                FAIL_LIST="$FAIL_LIST $name(rt_ok)"
            elif [ "$actual_err" != "$expected_err" ]; then
                echo "FAIL: $name (运行时诊断不匹配)"
                FAIL=$((FAIL + 1))
                FAIL_LIST="$FAIL_LIST $name(rt_err)"
            else
                echo "OK: $name (运行期越界被拦截)"
                PASS=$((PASS + 1))
            fi
        elif [ -f "示例/${name}.期望" ]; then
            actual=$(timeout $TIMEOUT_RUN $OUT_EXE 2>/dev/null | sed 's/\r//')
            rc_run=$?
            if [ $rc_run -eq 124 ]; then
                echo "TIMEOUT: $name (run)"
                FAIL=$((FAIL + 1))
                FAIL_LIST="$FAIL_LIST $name(rt_timeout)"
            else
                expected=$(cat "示例/${name}.期望")
                if [ "$actual" = "$expected" ]; then
                    echo "OK: $name (output match)"
                    PASS=$((PASS + 1))
                else
                    echo "FAIL: $name (output mismatch)"
                    FAIL=$((FAIL + 1))
                    FAIL_LIST="$FAIL_LIST $name(out)"
                fi
            fi
        else
            timeout $TIMEOUT_RUN $OUT_EXE 2>/dev/null 1>/dev/null
            rc2=$?
            if [ $rc2 -eq 124 ]; then
                echo "TIMEOUT: $name (run)"
                FAIL=$((FAIL + 1))
                FAIL_LIST="$FAIL_LIST $name(rt_timeout)"
            elif [ $rc2 -eq 0 ]; then
                echo "OK: $name"
                PASS=$((PASS + 1))
            else
                echo "FAIL: $name (runtime)"
                FAIL=$((FAIL + 1))
                FAIL_LIST="$FAIL_LIST $name(rt)"
            fi
        fi
        清理残留
    done

    rm -f out_t out_t.exe out_t.ll out_t.s out_t.o
    rm -f _cinterop.dll
    rm -f _clink_add.o _clink_mul.o _clink_sub.o _clink.a
    rm -rf _clink_lib

    echo ""
    echo "=== Result ==="
    echo "Pass: $PASS  Fail: $FAIL"
    if [ $FAIL -gt 0 ]; then
        echo "Failed:$FAIL_LIST"
    fi
    if [ "$FAIL" -eq 0 ] && [ "$PASS" -ge "$MIN_PASS" ]; then
        echo "门禁: 通过 (Pass=$PASS >= $MIN_PASS, Fail=0)"
        return 0
    fi
    echo "门禁: 未通过 (Pass=$PASS, Fail=$FAIL；要求 Pass>=$MIN_PASS 且 Fail=0)"
    return 1
}

mkdir -p 阶段
: > "$LOG"
运行回归 2>&1 | tee "$LOG"
exit "${PIPESTATUS[0]}"

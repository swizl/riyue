#!/bin/bash
# 日月端到端回归：遍历 示例/*.心，用自举编译器 日月.exe 编译并运行，
# 与 示例/<名>.期望 比对（无 .期望 的用例只要编译成功且退出码为 0 即通过）。
# 结果同时打印到控制台并固化到 阶段/回归.log；末尾按门禁判定退出码。
export PATH=/usr/bin:/bin:/mingw64/bin:$PATH
export MSYS_NO_PATHCONV=1
cd /d/src/riyue/riyue || exit 9

# 门禁：Pass 不得低于用例总数，Fail 必须为 0（当前 示例/*.心 共 109 个）。
MIN_PASS=109
LOG=阶段/回归.log

PASS=0
FAIL=0
FAIL_LIST=""
TIMEOUT_COMPILE=30
TIMEOUT_RUN=10
COMPILE_RETRY=2

# 清理编译残留的进程和文件，避免文件锁导致后续编译卡住
清理残留() {
    /c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe -NoProfile -Command \
        "Get-Process -Name '日月','out_t','clang++','ld','llc' -ErrorAction SilentlyContinue | Stop-Process -Force" 2>/dev/null
    rm -f out_t.exe out_t.ll out_t.s out_t.o
}

运行回归() {
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
            timeout $TIMEOUT_RUN ./out_t.exe > 阶段/.rt_out 2> 阶段/.rt_err
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
            actual=$(timeout $TIMEOUT_RUN ./out_t.exe 2>/dev/null | sed 's/\r//')
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
            timeout $TIMEOUT_RUN ./out_t.exe 2>/dev/null 1>/dev/null
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

    rm -f out_t.exe out_t.ll out_t.s out_t.o

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
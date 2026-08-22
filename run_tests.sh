#!/bin/bash
export PATH=/usr/bin:/bin:/mingw64/bin:$PATH
export MSYS_NO_PATHCONV=1
cd /d/src/lune2

PASS=0
FAIL=0
FAIL_LIST=""
TIMEOUT_COMPILE=10
TIMEOUT_RUN=5
COMPILE_RETRY=1

# 清理编译残留的进程和文件，避免文件锁导致后续编译卡住
清理残留() {
    /c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe -NoProfile -Command \
        "Get-Process -Name '日月','out_t','clang++','ld','llc' -ErrorAction SilentlyContinue | Stop-Process -Force" 2>/dev/null
    rm -f out_t.exe out_t.ll out_t.s
}

for f in 示例/*.心; do
    name=$(basename "$f" .心)
    timeout $TIMEOUT_COMPILE ./日月.exe "$f" out_t 2>/dev/null 1>/dev/null
    rc=$?
    if [ $rc -eq 124 ]; then
        # 首次超时可能因杀毒扫描/文件锁偶发阻塞，清理残留后重试
        已重试=0
        while [ $rc -eq 124 ] && [ $已重试 -lt $COMPILE_RETRY ]; do
            清理残留
            已重试=$((已重试 + 1))
            echo "RETRY: $name (compile, $已重试/$COMPILE_RETRY)"
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
    fi
    if [ $rc -ne 0 ]; then
        echo "FAIL: $name (compile)"
        FAIL=$((FAIL + 1))
        FAIL_LIST="$FAIL_LIST $name(cmp)"
        continue
    fi
    if [ -f "示例/${name}.期望" ]; then
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
done

rm -f out_t.exe out_t.ll out_t.s

echo ""
echo "=== Result ==="
echo "Pass: $PASS  Fail: $FAIL"
if [ $FAIL -gt 0 ]; then
    echo "Failed:$FAIL_LIST"
fi

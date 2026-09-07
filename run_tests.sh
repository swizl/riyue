#!/bin/bash
export PATH=/usr/bin:/bin:/mingw64/bin:$PATH
export MSYS_NO_PATHCONV=1
cd /d/src/riyue/riyue

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
    清理残留
done

rm -f out_t.exe out_t.ll out_t.s out_t.o

echo ""
echo "=== Result ==="
echo "Pass: $PASS  Fail: $FAIL"
if [ $FAIL -gt 0 ]; then
    echo "Failed:$FAIL_LIST"
fi
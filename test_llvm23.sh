#!/bin/bash
export PATH=/d/src/riyue/llvm/llvm-build/bin:/mingw64/bin:$PATH
export MSYS_NO_PATHCONV=1
cd /d/src/riyue/riyue
PASS=0; FAIL=0; FAILS=""
for f in "$@"; do
  [ -f "$f" ] || { echo "SKIP: $f (no file)"; continue; }
  name=$(basename "$f" .心)
  rm -f out_t.exe
  timeout 30 ./日月.exe "$f" out_t >/dev/null 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then echo "FAIL: $name (compile rc=$rc)"; FAIL=$((FAIL+1)); FAILS="$FAILS $name(cmp)"; continue; fi
  timeout 10 ./out_t.exe >/dev/null 2>&1
  rcr=$?
  if [ $rcr -eq 0 ]; then echo "OK:   $name"; PASS=$((PASS+1)); else echo "FAIL: $name (run rc=$rcr)"; FAIL=$((FAIL+1)); FAILS="$FAILS $name(rt)"; fi
done
rm -f out_t.exe
echo ""; echo "=== Pass: $PASS  Fail: $FAIL ==="
[ -n "$FAILS" ] && echo "Failed:$FAILS"

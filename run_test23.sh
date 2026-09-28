#!/bin/bash
export PATH=/d/src/riyue/llvm/llvm-build/bin:/mingw64/bin:$PATH
export MSYS_NO_PATHCONV=1
cd /d/src/riyue/riyue || exit 1
bash test_llvm23.sh 示例/*.心

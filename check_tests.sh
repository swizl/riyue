#!/bin/bash
export PATH=/mingw64/bin:$PATH
export MSYS_NO_PATHCONV=1
cd /d/src/lune2

tests=(
"示例/数组基础.心"
"示例/数组元素赋值测试.心"
"示例/标准库数组测试.心"
"示例/范围循环简单测试.心"
"示例/数组传参.心"
"示例/遍历测试.心"
"示例/闭包.心"
"示例/闭包简单.心"
"示例/包管理器.心"
"示例/标准库测试.心"
"示例/目录操作测试.心"
"示例/文件操作测试.心"
"示例/自举编译器.心"
"示例/综合功能测试.心"
)

for f in "${tests[@]}"; do
    name=$(basename "$f" .心)
    timeout 5 ./日月.exe "$f" out_t 2>/dev/null 1>/dev/null
    rc=$?
    if [ $rc -eq 124 ]; then
        echo "CMP_TIMEOUT: $name"
        continue
    fi
    if [ $rc -ne 0 ]; then
        echo "CMP_FAIL: $name"
        continue
    fi
    timeout 3 ./out_t.exe 2>/dev/null 1>/dev/null
    rc2=$?
    if [ $rc2 -eq 124 ]; then
        echo "RUN_TIMEOUT: $name"
    elif [ $rc2 -ne 0 ]; then
        echo "RUN_FAIL: $name"
    else
        echo "OK: $name"
    fi
done

rm -f out_t.exe out_t.ll out_t.s

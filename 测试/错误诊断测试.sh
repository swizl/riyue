#!/bin/bash
# 错误诊断测试（改进计划 Task 3.4）
#
# 断言：语义 / 语法 / 入口 / 导入 / 档位 五类错误都给出**带错误码**的诊断，
#       且编译失败时**不产出中间产物**（避免「报错却仍生成半成品」）。
# 覆盖错误码：C001 未声明标识符、C003 解析失败（含选择性导入名单非法）、
#             C004 无法读取输入文件、C005 找不到模块 / 循环导入、C014 未知档位。
# 依赖：仓库根的 日月.exe（`make 日月.exe`）。由 run_tests.sh 的「错误诊断自测」调用，
#       也可单独执行：`bash 测试/错误诊断测试.sh`（退出码非 0 即失败）。
cd "$(dirname "$0")/.." || exit 9

WORK=阶段/.错误诊断
rm -rf "$WORK"
mkdir -p "$WORK"

PASS=0
FAIL=0
TMO=120

# check_case NAME SRC EXPECT_CODE OUT_TAG [EXTRA CLI ARGS...]
check_case() {
    name="$1"
    src="$2"
    want="$3"
    out="$WORK/$4.ll"
    shift 4

    rm -f "$out"
    got=$(timeout $TMO ./日月.exe "$@" "$src" "$out" 2>&1)

    if [ -f "$out" ]; then
        echo "FAIL: $name（编译失败却仍产生产物）"
        FAIL=$((FAIL + 1))
        return
    fi
    if ! echo "$got" | grep -q "错误\[$want\]"; then
        echo "FAIL: $name（未报 $want）"
        echo "    实际输出: $(echo "$got" | sed 's/\r//' | head -3 | tr '\n' ' ')"
        FAIL=$((FAIL + 1))
        return
    fi
    echo "OK: $name（$want）"
    PASS=$((PASS + 1))
}

echo "=== 错误诊断测试 ==="

printf '%s' '函数 主程序 { (未定义变量)打印; }' > "$WORK/未声明.心"
check_case "未声明标识符" "$WORK/未声明.心" C001 未声明

printf '%s' '函数 主程序 {' > "$WORK/语法残缺.心"
check_case "解析失败" "$WORK/语法残缺.心" C003 语法残缺

check_case "输入文件不存在" "$WORK/不存在.心" C004 不存在

printf '%s' '导入 "阶段/.错误诊断/不存在模块.心"; 函数 主程序 { ("x")打印; }' > "$WORK/缺模块.心"
check_case "找不到模块" "$WORK/缺模块.心" C005 缺模块

printf '%s' '导入 "阶段/.错误诊断/乙.心"; 函数 主程序 { ("甲")打印; }' > "$WORK/甲.心"
printf '%s' '导入 "阶段/.错误诊断/甲.心"; 函数 辅助 { ("乙")打印; }' > "$WORK/乙.心"
check_case "循环导入" "$WORK/甲.心" C005 循环

printf '%s' '导入 "示例/选择性导入/工具模块.心" { 1 }; 函数 主程序 { ("x")打印; }' > "$WORK/导入名单.心"
check_case "选择性导入名单内应为标识符" "$WORK/导入名单.心" C003 导入名单

printf '%s' '函数 主程序 { ("档位")打印; }' > "$WORK/档位.心"
check_case "未知档位取值" "$WORK/档位.心" C014 档位 --档位=怪

echo ""
echo "错误诊断: 通过 $PASS  失败 $FAIL"
[ "$FAIL" -eq 0 ] || exit 1
echo "OK: 错误诊断（C001/C003/C004/C005/C014 均报码且不产生产物）"

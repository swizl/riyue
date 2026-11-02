#!/bin/bash
# 错误诊断测试（改进计划 Task 3.4）
#
# 断言：语义 / 语法 / 入口 / 导入 / 档位 五类错误都给出**带错误码**的诊断，
#       且编译失败时**不产出中间产物**（避免「报错却仍生成半成品」）。
# 覆盖错误码：C001 未声明标识符、C003 解析失败（含选择性导入名单非法）、
#             C004 无法读取输入文件、C005 找不到模块 / 循环导入、C014 未知档位、
#             C017 嵌套过深（以 300 层嵌套括号验证安全阀有界，非绑定单一码）。
# 依赖：仓库根的 日月.exe（`make 日月.exe`）。由 run_tests.sh 的「错误诊断自测」调用，
#       也可单独执行：`bash 测试/错误诊断测试.sh`（退出码非 0 即失败）。
cd "$(dirname "$0")/.." || exit 9
# 单独执行时也需要关闭 MSYS 的路径转换：日月.exe 是原生 Windows 程序，
# 否则「阶段/.错误诊断/xxx.心」会被改写成「阶段C:/tools/msys64/...」。
export MSYS_NO_PATHCONV=1

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

# 深嵌套安全阀（8JT）：300 层嵌套括号必须**不崩溃**——
# 要么正常产出产物（说明未触及 512 的解析深度上界），要么给出带码诊断
# （错误[C0xx]，C003 解析失败 / C017 嵌套过深）；不得以超时或信号退出（128+n）。
# 判据取「有界且可解释」，而非绑定某一个错误码，避免把上界调小后判例失真。
printf '%s' '函数 主程序 { ' > "$WORK/深嵌套.心"
i=0
while [ "$i" -lt 300 ]; do printf '(' >> "$WORK/深嵌套.心"; i=$((i + 1)); done
printf '%s' '1' >> "$WORK/深嵌套.心"
i=0
while [ "$i" -lt 300 ]; do printf ')' >> "$WORK/深嵌套.心"; i=$((i + 1)); done
printf '%s\n' ')打印; }' >> "$WORK/深嵌套.心"

rm -f "$WORK/深嵌套.ll"
got=$(timeout $TMO ./日月.exe "$WORK/深嵌套.心" "$WORK/深嵌套.ll" 2>&1)
rc=$?
if [ "$rc" -ge 124 ]; then
    echo "FAIL: 深嵌套 300 层（超时或崩溃退出，退出码 $rc）"
    FAIL=$((FAIL + 1))
elif [ "$rc" -eq 0 ] && [ -f "$WORK/深嵌套.ll" ]; then
    echo "OK: 深嵌套 300 层（正常产出，未触及解析深度上界）"
    PASS=$((PASS + 1))
elif echo "$got" | grep -q '错误\[C0'; then
    echo "OK: 深嵌套 300 层（安全阀拒绝并报码）"
    PASS=$((PASS + 1))
else
    echo "FAIL: 深嵌套 300 层（失败但无带码诊断，退出码 $rc）"
    echo "    实际输出: $(echo "$got" | sed 's/\r//' | head -3 | tr '\n' ' ')"
    FAIL=$((FAIL + 1))
fi

# 合法 300 层嵌套（正例）：上面的判例断言「有界」，这里断言「不误伤」——
# 300 层括号是合法代码，必须正常编译出 .ll（解析深度阀 8192 覆盖约 5100 的
# 实际穿透量），而不是靠报 C003 兜底。
printf '%s' '函数 主程序 { ' > "$WORK/合法深嵌套.心"
i=0
while [ "$i" -lt 300 ]; do printf '(' >> "$WORK/合法深嵌套.心"; i=$((i + 1)); done
printf '%s' '1' >> "$WORK/合法深嵌套.心"
i=0
while [ "$i" -lt 300 ]; do printf ')' >> "$WORK/合法深嵌套.心"; i=$((i + 1)); done
printf '%s\n' '打印; }' >> "$WORK/合法深嵌套.心"

rm -f "$WORK/合法深嵌套.ll"
got=$(timeout $TMO ./日月.exe "$WORK/合法深嵌套.心" "$WORK/合法深嵌套.ll" 2>&1)
rc=$?
if [ "$rc" -eq 0 ] && [ -s "$WORK/合法深嵌套.ll" ]; then
    echo "OK: 合法深嵌套 300 层（正常产出 .ll）"
    PASS=$((PASS + 1))
else
    echo "FAIL: 合法深嵌套 300 层（退出码 $rc，未产出 .ll）"
    echo "    实际输出: $(echo "$got" | sed 's/\r//' | head -3 | tr '\n' ' ')"
    FAIL=$((FAIL + 1))
fi

# 交叉目标闭环（阶段5G 续）：
#   ① aarch64 Linux 静态 ELF：链入交叉运行时后不得有未定义符号，可执行文件头为 AArch64；
#   ② wasm32-wasip1：产出 WASI 命令模块，_start 可被 Node WASI 实例化并打印「你好」。
# 本机无 qemu，aarch64 只做静态校验；wasm 由 Node 实际运行（无 node 时跳过该断言）。
case "$(uname -s)" in
    Linux) LLVM_BIN=/mnt/d/src/riyue/llvm/llvm-build-linux/bin ;;
    *)     LLVM_BIN=/d/src/riyue/llvm/llvm-build/bin
           # 本地化 LLVM 是 MinGW 构建，需 mingw64/bin 提供 libstdc++ 等运行库
           PATH="$LLVM_BIN:/mingw64/bin:$PATH"; export PATH ;;
esac

printf '%s\n' '函数 主程序 { ("你好")打印; }' > "$WORK/交叉宿主.心"

rm -f "$WORK/交叉aarch64"
got=$(timeout $TMO ./日月.exe --目标=aarch64-unknown-linux-gnu "$WORK/交叉宿主.心" "$WORK/交叉aarch64" 2>&1)
rc=$?
if [ "$rc" -eq 0 ] && [ -f "$WORK/交叉aarch64" ] \
   && "$LLVM_BIN/llvm-readelf" -h "$WORK/交叉aarch64" 2>/dev/null | grep -q 'Machine:.*AArch64' \
   && [ -z "$("$LLVM_BIN/llvm-nm" -u "$WORK/交叉aarch64" 2>/dev/null)" ]; then
    echo "OK: aarch64 交叉可执行（ELF64 AArch64，无未定义符号）"
    PASS=$((PASS + 1))
else
    echo "FAIL: aarch64 交叉可执行（退出码 $rc，未产出可链接产物）"
    echo "    实际输出: $(echo "$got" | sed 's/\r//' | head -3 | tr '\n' ' ')"
    FAIL=$((FAIL + 1))
fi

# Node 可能不在 MSYS 登录 shell 的 PATH 里：回退到本机常见安装位置
NODE=$(command -v node 2>/dev/null)
if [ -z "$NODE" ] && [ -x /d/ProgramData/nodejs/node.exe ]; then NODE=/d/ProgramData/nodejs/node.exe; fi
if [ -z "$NODE" ] && [ -x "/c/Program Files/nodejs/node.exe" ]; then NODE="/c/Program Files/nodejs/node.exe"; fi

rm -f "$WORK/交叉wasi.wasm"
got=$(timeout $TMO ./日月.exe --目标=wasm32-wasip1 "$WORK/交叉宿主.心" "$WORK/交叉wasi.wasm" 2>&1)
rc=$?
if [ "$rc" -ne 0 ] || [ ! -s "$WORK/交叉wasi.wasm" ]; then
    echo "FAIL: wasm32-wasip1 产物（退出码 $rc，未产出 .wasm）"
    echo "    实际输出: $(echo "$got" | sed 's/\r//' | head -3 | tr '\n' ' ')"
    FAIL=$((FAIL + 1))
elif [ -z "$NODE" ]; then
    echo "OK: wasm32-wasip1 产物（已产出；无 node，跳过运行验证）"
    PASS=$((PASS + 1))
else
    printf '%s\n' \
        'const fs=require("node:fs"); const {WASI}=require("node:wasi");' \
        'const w=new WASI({version:"preview1",args:[],env:{}});' \
        'const m=new WebAssembly.Module(fs.readFileSync(process.argv[2]));' \
        'w.start(new WebAssembly.Instance(m,w.getImportObject()));' \
        > "$WORK/跑wasi.js"
    # 只取 stdout：Node 会把 WASI 实验特性警告打到 stderr，混入会污染断言
    got=$(timeout $TMO "$NODE" "$WORK/跑wasi.js" "$WORK/交叉wasi.wasm" 2>/dev/null | tr -d '\r')
    if [ "$got" = "你好" ]; then
        echo "OK: wasm32-wasip1 可运行（Node WASI 输出「你好」）"
        PASS=$((PASS + 1))
    else
        echo "FAIL: wasm32-wasip1 运行输出不符（实际: $(echo "$got" | head -3 | tr '\n' ' ')）"
        FAIL=$((FAIL + 1))
    fi
fi

echo ""
echo "错误诊断: 通过 $PASS  失败 $FAIL"
[ "$FAIL" -eq 0 ] || exit 1
echo "OK: 错误诊断（C001/C003/C004/C005/C014 + 深嵌套安全阀 均报码/不产半成品）"

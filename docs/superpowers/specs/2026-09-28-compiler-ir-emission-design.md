# compiler.心 发射层改为中文 LLVM IR 设计文档

- 日期：2026-09-28
- 状态：已批准
- 背景：当前自举走「发射 C 路线」——日月源码 → compiler.心 → C 文本 → clang → 可执行。本设计把发射层从 C 文本改为**中文 LLVM IR 文本**，实现「摆脱 C」。

## 1. 总体架构

### 保持不变
- 词法分析 / 类型表（局部/全局/函数返回）/ 函数签名解析 / 语句语法解析 / 两遍扫描结构。
- 运行时辅助.o 作为预编译对象参与链接，不做日月化运行时。
- 日月.exe 仍作宿主编译 compiler.心 → stage1.exe（日月.exe 的 LLVM 后端独立于 compiler.心 的发射层，不受影响）。

### 变更
- 发射缓冲内容：C → 中文 LLVM IR。
- 表达式链：从「拼接 C 表达式字符串」改为「发射三地址码指令 + 返回寄存器名」。
- 新增：字符串表（字符串字面量集中编号发射）、标识符转义（非 ASCII → `\EHH` 字节转义）。

## 2. IR 前导（发射前导 重写）

```
; ModuleID = '程序'
source_filename = "\E7\A8\8B\E5\BA\8F"
```

### 声明
- libc：`声明 i32 @printf(ptr, ...)`、`声明 ptr @malloc(i64)`、`声明 ptr @memcpy(ptr, ptr, i64)`、`声明 i64 @strlen(ptr)`、`声明 i32 @strcmp(ptr, ptr)`、`声明 i32 @sprintf(ptr, ptr, ...)`、`声明 ptr @strdup(ptr)`。
- 运行时函数声明（与 C 前导清单一致）：设置参数 / 获取参数数量 / 获取参数 / 读取文件 / 写入文件 / 字符位置到字节位置 / 获取字符数 / 字符码 / 输出错误 / 分割行数 / 获取行 / 字符串开头 等。

### 三个 static helper 改为 IR 定义块
- 日月子串：字符位置到字节位置 + memcpy + malloc。
- 日月拼接：strlen + memcpy + malloc。
- 日月转字符串：sprintf + strdup。

## 3. 字符串表 + 全局变量

- compiler.心 新增 字符串表 数据结构，收集所有字符串字面量，统一发射：
  `@N = 私有 未命名地址 常量 [N x i8] c"...", align 1`
- 全局变量（数字/整数）：`@"\E…" = 内部 全局 i32 零初始化`
- 全局字符串：`@"\E…" = 内部 全局 ptr @N`（指针指向字符串常量）
- 本地寄存器命名策略：临时值用数字 `%N` / `%tmpN`；用户变量 alloca 用转义中文名 `%"\E…"`。

## 4. 函数定义（第二遍发射）

```
定义 内部 <返回类型> @<名>(<参数列表>) {
entry:
  %"…输出…" = 分配 <类型>, align 4        ; 有输出参数时
  ; 整数(int) 值参数 复制到 alloca（可被修改）
  ; 字符串(ptr) 参数直接使用
  ; 局部声明 → 全量 alloca
  ; 函数体 → IR 指令序列
  返回 <类型> %reg   /   返回 void
}
```

- main 入口：`定义 i32 @main(i32 %0, ptr %1)` → 存储到 alloca → `调用 void @设置参数(...)` → `调用 void @主程序()` → `返回 i32 0`。

## 5. 表达式与语句的三地址码降低

- 二元/一元运算各生成一条指令 + 一个新临时寄存器：`%t = 加 i32 %a, 1`。
- 比较得 i1：`整数比较 sgt i32 %a, %b`；赋值时 `零扩展 i1 to i32` 再存储，条件中直接分支。
- 字符串 `==`：`调用 i32 (ptr, ptr) @strcmp(...)` + `整数比较 eq`。
- 字符串 `+`：`调用 ptr (ptr, ptr) @日月拼接(...)`。
- 控制流：
  - 如果 / 当 / 循环 → `分支 i1 %cond, 标签 %L1, 标签 %L2` 标签块。
  - 中断 → 跳出口标签；继续 → 跳回条件标签。
  - 循环 for 映射现有 循环 变量（循环变量/循环开始C/循环结束C/循环步长C），顺带清理未使用变量 `循环步长是1`。
- 内置函数映射不变，仅发射形式改为 `调用 <类型> (<签名>) @名(...)`。

## 6. Makefile 自举目标改造

```
日月.exe 编译 compiler.心 → stage/stage1.exe
stage1.exe 编译 compiler.心 → stage/ir1.ll
llvm-as  ir1.ll → ir1.bc
llc -mtriple=x86_64-w64-windows-gnu ir1.bc → ir1.o
ld.lld crt2.o ir1.o 运行时辅助.o -o stage2.exe  (-L/c/tools/msys64/mingw64/lib -L…/gcc/x86_64-w64-mingw32/16.1.0 -lmingw32 -lmingwex -lmsvcrt -lgcc -lmoldname -lws2_32 -lm -ladvapi32 -lshell32 -luser32 -lkernel32)
stage2.exe 编译 compiler.心 → stage/ir2.ll
diff ir1.ll ir2.ll      ← 不动点验证
```

- llvm-as / llc / ld.lld 需在 PATH 含 `/c/tools/msys64/mingw64/bin` 的 MSYS2 bash 下运行。
- 输出路径用 ASCII（stage/），避免中文目录导致 MSYS2 ld 失败。

## 7. 验证与测试

1. 简单样例（闭包 / t_loop）→ 新 compiler → IR → exe → 运行比对输出。
2. `make 自举`：diff ir1.ll == ir2.ll。
3. 全量回归 test_all23.ps1（先确认走 日月.exe 后端还是走 C 发射；若后者同步改调用链）。
4. 更新 riyue/progress.md，提交。

## 8. 已知风险

- 非 ASCII 标识符转义是最大工作量：新增 `转义标识符`（字节 → `\EHH`，可打印 ASCII 且非特殊字符保留字面量），覆盖全局名 / 函数名 / alloca 名 / 标签名。
- 输出从 28894 字节 C 变成数万行 IR；llvm-as/llc/ld.lld 吞吐没问题，但 self-host 编译耗时可能超过 26s。
- 易错点：字符串全局初始化、i64 返回值（strlen）、ptr↔i64 转换（字符位置到字节位置）、所有用到 printf 的调用需带函数指针类型 `(ptr, ...)`。

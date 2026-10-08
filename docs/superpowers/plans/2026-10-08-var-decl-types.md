# 变量声明类型默认规则 + 定宽整数/浮点类型 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 实现定宽类型（整数8/16/32/64、浮点32/64）与变量声明的类型默认规则（值侧/名侧类型标注、默认类型、混合提升），宿主与自举编译器同步落地。

**Architecture:** 类型解析路径统一为「类型名 → 基础类型 → IR/局部码」。宿主（源/）与自举（自举/）各有一套等价实现；文法（工具/日月/日月.文法）为唯一语法源，改动后重新生成 PEG 解析器（自举/解析器.心）。声明发射按优先级规则选定最终类型并在需要时插入转换指令；算术按宽度提升。

**Tech Stack:** 日月自举编译器；C++ 宿主（clang/LLVM IR 中文方言）；工具/peg PEG 生成器；msys bash + PowerShell 测试链（test_all23.ps1）。

**Spec:** docs/superpowers/specs/2026-10-08-var-decl-types-design.md（已批准，commit 546b029）

## Global Constraints

- 日月 赋值语义：`A = B` 即 `B := A`（值在左，目标在右）。`?=` 是相等比较。
- 所有 自举/*.心 必须 UTF-8 无 BOM；编辑后检查 `git ls-files --eol <file>`，若 w/crlf 用 `sed -i "s/\r$//" <file>` 归一化（msys bash）。
- 构建命令（msys bash，必须带 PATH）：`& 'C:\tools\msys64\usr\bin\bash.exe' -lc 'cd /d/src/riyue/riyue && export MSYS2_ARG_CONV_EXCL="*" && export PATH=/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:/usr/bin:$PATH && make 2>&1 | tail -3'`。
- 全量测试：`powershell -NoProfile -ExecutionPolicy Bypass -File C:\Users\jalenli\AppData\Local\Temp\opencode\t.ps1` → res.txt（read 读取）。基准 68/16；允许 数学工具/数学模块 失败；其余须全绿。
- LSP/clangd 对 源/*.cpp|*.c|*.h 的报错是已知噪音，忽略。
- 现有类型名向后兼容：整数≡整数32、浮点≡浮点64，旧程序行为不变。

---

### Task 1: 文法新增定宽类型名与值侧类型标注（工具/peg 链）

**Files:**
- Modify: `工具/日月/日月.文法`（类型规则 + 声明规则）
- Regenerate: `自举/解析器.心`（经由 工具/peg 生成器）
- Test: `工具/peg/测试_日月.sh` 及 示例/新增回归样例

**Interfaces:**
- Produces: 类型名集合 {整数8,整数16,整数32,整数64,浮点32,浮点64,整数,浮点,字符串,布尔,函数,字符,数组,结构体,映射,元组,空}；声明语法 `值 [:类型] = [变量] 名 [:类型] ;` 的 AST 节点。

- [ ] **Step 1: 阅读当前文法类型与声明规则**

Run（读文件）：`read 工具/日月/日月.文法` 定位「类型」规则与「全局变量/局部变量声明」规则的实际写法；再 `read 工具/peg/主程序.心` 确认重新生成命令（生成树库/生成自举库 的确切参数形式）。

- [ ] **Step 2: 修改类型规则**

在 类型 规则中新增分支：整数8/整数16/整数32/整数64/浮点32/浮点64（与现有 整数/浮点/字符串… 并列，均产出 类型 叶节点，文本为类型名原样）。

- [ ] **Step 3: 修改声明规则支持值侧类型**

声明规则改为：`值 [: 类型] = [变量] 名 [: 类型] ;`。值侧类型标注为「值表达式 + `:` + 类型」；与名侧类型并列产出，AST 节点保留两侧类型文本（值侧类型节点名 `值类型`，名侧 `类型`）。

- [ ] **Step 4: 重新生成 自举/解析器.心**

用 Step 1 确认的命令从 日月.文法 重新生成 解析器.心（含动作模式 AST 语义动作）。构建并验证：`make`（宿主 引导.exe 编译 主入口.心 产出 日月.exe）。

- [ ] **Step 5: 语法回归验证**

Run: `tools/peg/测试_日月.sh`（或对应生成库/树测试脚本），预期全过；抽查生成 解析器.心 中 类型 与 声明 规则代码。

- [ ] **Step 6: Commit**

```bash
git add 工具/日月/日月.文法 自举/解析器.心
git commit -m "feat(grammar): 定宽整数/浮点类型名 + 值侧类型标注语法"
```

---

### Task 2: 宿主词法/语法分析支持新语法

**Files:**
- Modify: `源/前端/词法分析器.cpp`、`源/前端/语法分析器.cpp`、`源/前端/模块合并器.cpp`（若涉及类型收集）

**Interfaces:**
- Consumes: Task 1 的语法约定。
- Produces: 宿主侧能解析 `0:整数8 = 变量 甲:整数8;`、`0 = 变量 甲;` 等，AST 携带 值类型/名侧类型。

- [ ] **Step 1: 词法分析器新增类型关键字**

定位类型关键字表，加入 整数8/整数16/整数32/整数64/浮点32/浮点64（注意：整数 与 整数8 前缀重合，词法须最长匹配，避免 整数 吞掉 `整数8`）。

- [ ] **Step 2: 语法分析器支持值侧类型**

在声明解析处：值表达式后若跟 `:` 且后随类型关键字 → 读入为「值类型」并附到声明节点；名侧类型沿用现有逻辑；两侧类型文本都存入节点。

- [ ] **Step 3: 构建 + 冒烟**

构建宿主 引导.exe；用 `0:整数8 = 变量 甲:整数8;` 写最小样例编译，确认 AST 含两侧类型。

- [ ] **Step 4: Commit**

```bash
git add 源/前端/词法分析器.cpp 源/前端/语法分析器.cpp
git commit -m "feat(host-front): 定宽类型关键字 + 值侧类型标注解析"
```

---

### Task 3: 宿主类型表与转换发射

**Files:**
- Modify: `源/后端/代码生成器.cpp`、`源/后端/代码生成器_表达式.cpp`、`源/后端/代码生成器_语句.cpp`、`源/后端/LLVM辅助.cpp`

**Interfaces:**
- Consumes: Task 2 的 AST 类型文本。
- Produces: 类型名→LLVM 类型映射；`值类型`/`名类型` 判定与转换插入函数。

- [ ] **Step 1: 类型名→IR 映射**

新增映射：整数8→i8、整数16→i16、整数32→i32、整数64→i64、浮点32→float、浮点64→double、整数→i32、浮点→double。把现有 查类型 单字符码体系扩展为可表示 i8/i16/i64/float/double 的码（建议沿用单字符码：新增 `1`→i8? 冲突；改为 类型码 用 两位/复合 或直接存 IR 名，宿主侧可自由扩展枚举）。

- [ ] **Step 2: 声明类型判定**

实现规则优先级：(a) 名侧有类型 → 名侧类型，值转换；(b) 仅值侧有类型 → 值侧类型；(c) 两侧都有且不同 → 编译错误「类型不一致」；(d) 都无 → 按字面量默认（整数→整数32、浮点→浮点64、字符串→字符串、真/假→布尔）。

- [ ] **Step 3: 转换插入**

在声明发射处，若 值实际类型 ≠ 名侧类型，插入：整数族窄→宽 `符号扩展`（sext）、宽→窄 `截断`（trunc）、整数→浮点 `有符号转浮点`（sitofp）、浮点→整数 `浮点转有符号`（fptosi）。

- [ ] **Step 4: 算术提升**

二元运算按 宽度 提升：同族取宽者；整数+浮点 → 浮点64（整数先 sitofp 到 double）。一元保持类型。比较同规则。

- [ ] **Step 5: 打印格式**

打印 分支按 表达式类型 选格式：i8/i16 先 sext 到 i32 用 `%d`；i32 `%d`；i64 `%lld`；float `%f`；double `%g`。

- [ ] **Step 6: 构建 + 最小样例**

用 `示例/` 新增 定宽测试.心：`0:整数8 = 变量 甲:整数8;`、`0 = 变量 乙;`、`3.14:浮点32 = 变量 丙:浮点32;`、`1 = 变量 丁:整数64;`、`1:整数 = 变量 戊:浮点;`（应报编译错误）。编译+运行验证输出与错误。

- [ ] **Step 7: Commit**

```bash
git add 源/后端/*.cpp 示例/定宽测试.心
git commit -m "feat(host-back): 定宽类型声明/转换/提升/打印"
```

---

### Task 4: 自举编译器镜像（类型表 + 声明 + 提升 + 打印）

**Files:**
- Modify: `自举/发射.心`（查类型/查函数返回类型/类型名到IR 邻近）、`自举/语法_树.心`（类型名到局部码/类型名到IR/发射局部声明/发射调用/发射函数定义节点/收集函数原型节点）、`自举/编译器.心`（新增全局 类型别名表）

**Interfaces:**
- Consumes: Task 1 重新生成的 解析器.心（新类型节点）；Task 3 的语义规则。
- Produces: 自举编译器等价能力。

- [ ] **Step 1: 类型码扩展**

在 发射.心 查类型 与 语法_树.心 类型名到局部码/类型名到IR：新增固定宽度类型到 IR 的映射（i8/i16/i32/i64/float/double）。单字符码不足时，扩为 名侧类型直接存 IR 文本 的路径（新增 局部类型表 条目 `名:IR名`，并让 发射局部声明/类型名到IR 识别）。

- [ ] **Step 2: 声明发射判定 + 转换**

在 语法_树.心 的局部/全局声明发射处实现 优先级(a-d)：名侧类型、值侧类型、默认类型；插入 sext/trunc/sitofp/fptosi（日月 IR 中文指令 `符号扩展/截断/有符号转浮点/浮点转有符号`）；两侧类型不同 → `输出错误` 编译错误。

- [ ] **Step 3: 算术提升**

发射调用/二元运算节点：按 宽度 提升；整数+浮点 → 浮点64。对照宿主实现逐一镜像。

- [ ] **Step 4: 打印格式**

打印 分支按 表达式类型 选择格式串（i8/i16 扩 i32 `%d`；i64 `%lld`；float `%f`；double `%g`）。

- [ ] **Step 5: 类型别名机制合并**

复用已暂停的别名修复计划（见 spec §10）：编译器.心 加 `"" = 变量 类型别名表;`；语法_树.心 加 `函数 (节点:整数)收集类型别名节点`（遍历顶层「类型别名」节点：children=[基础类型,可选数组标记,"=",标识符(别名),":",类型,";"]，取首个 类型 子文本为基名、首个 标识符 子文本为别名名，`(类型别名表, 别名名, 基名)表添加`），在 发射程序 开头发射前调用；类型名到局部码/类型名到IR 的 else 分支查 类型别名表 递归解析（别名→基名→基础类型）。收集函数原型节点 改按 类型名到局部码 注册 S/I/F。

- [ ] **Step 6: 构建 + 回归**

`make`；`t.ps1`。预期修复：类型别名完整测试、复杂类型别名测试（别名解析）不再「加载 void」；新增 定宽测试.心 通过；无回归（保留 数学工具/数学模块 允许失败）。

- [ ] **Step 7: Commit**

```bash
git add 自举/发射.心 自举/语法_树.心 自举/编译器.心
git commit -m "feat(selfhost): 定宽类型/值侧类型默认规则/别名解析镜像"
```

---

### Task 5: 文档与全量回归收尾

**Files:**
- Modify: `docs/superpowers/plans/2026-10-03-riyue-grammar-spec.md`
- Test: 全量

**Interfaces:**
- Consumes: Task 3/4 的行为。

- [ ] **Step 1: 更新文法文档**

在 2026-10-03-riyue-grammar-spec.md 补充：定宽类型名表、声明语法 `值[:类型] = [变量] 名[:类型];`、默认类型表、类型一致性错误、算术提升规则、转换指令、打印格式。内容与 spec 一致。

- [ ] **Step 2: 全量回归**

`t.ps1` 全绿（除允许的 数学工具/数学模块）；`make 自举` 不动点如超时则记录已知遗留（自 commit 6873ef6 起破损，执行命令 声明已补）。

- [ ] **Step 3: EOL 归一化 + Commit**

```bash
# 对所有改动的 自举/*.心 执行 sed -i "s/\r$//"（msys bash），git ls-files --eol 确认 lf
git add -A
git commit -m "docs: 更新文法规格（定宽类型/声明类型规则）"
```

---

## Self-Review

**Spec coverage:**
- §2 类型词汇 → Task 1（文法）、Task 3 Step 1（宿主映射）、Task 4 Step 1（自举映射）。
- §3 声明语法 → Task 1 Step 3、Task 2、Task 4 Step 2。
- §4 类型规则优先级 → Task 3 Step 2、Task 4 Step 2。
- §5 默认类型表 → Task 3 Step 2、Task 4 Step 2。
- §6 算术提升 → Task 3 Step 4、Task 4 Step 3。
- §7 转换规则 → Task 3 Step 3、Task 4 Step 2。
- §8 打印格式 → Task 3 Step 5、Task 4 Step 4。
- §9 实现范围 → 覆盖。
- §10 别名衔接 → Task 4 Step 5。
- 文档 → Task 5。

**Placeholder scan:** 无 TBD/TODO；唯一待确认点是 Task 1 Step 1 的「重新生成命令」，已显式列为第一步阅读确认，属于执行期信息而非占位。

**Type consistency:** 转换指令名在宿主用 C++ API、自举用中文 IR 名，两处分别以各自代码库既有命名表述；打印格式串一致。

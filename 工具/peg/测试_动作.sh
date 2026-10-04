#!/usr/bin/env bash
# S4.2b 基础设施回归：CST 模式忽略 {…}（与解释器一致），AST 模式执行 {…} 建树
cd /d/src/riyue/riyue
export MSYS2_ARG_CONV_EXCL='*'
export PATH=/usr/bin:/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:$PATH
bad=0
./日月.exe 工具/peg/主程序.心 阶段/peg_cli.exe || bad=1
./阶段/peg_cli.exe 生成 工具/peg/示例/动作.文法 阶段/动作_cst.心 || bad=1
./阶段/peg_cli.exe 生成树 工具/peg/示例/动作.文法 阶段/动作_ast.心 || bad=1
./日月.exe 阶段/动作_cst.心 阶段/动作_cst.exe || bad=1
./日月.exe 阶段/动作_ast.心 阶段/动作_ast.exe || bad=1
interp=$(./阶段/peg_cli.exe 识别 工具/peg/示例/动作.文法 工具/peg/示例/动作.输入 | tail -n 1)
cst=$(./阶段/动作_cst.exe 工具/peg/示例/动作.文法 工具/peg/示例/动作.输入)
ast=$(./阶段/动作_ast.exe 工具/peg/示例/动作.文法 工具/peg/示例/动作.输入)
rm -f 阶段/动作_cst.心 阶段/动作_ast.心
if [ "$interp" != "$cst" ]; then echo "CST 模式与解释器不一致: [$interp] vs [$cst]"; bad=1; fi
if [ "$ast" != "(程序 (配对 1 , 2))" ]; then echo "AST 模式未执行动作: [$ast]"; bad=1; fi
if [ "$bad" != "0" ]; then echo "动作模式测试失败"; exit 1; fi
echo "动作模式测试通过"

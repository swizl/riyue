# 编译器和 LLVM 库均使用 D:\src\riyue\llvm\llvm-build 自定义构建的中文 LLVM 23。
# 可用 `make LLVM_CONFIG=/path/to/llvm-config CXX=...` 覆盖。
LLVM_CONFIG ?= /d/src/riyue/llvm/llvm-build/bin/llvm-config

CXX = /d/src/riyue/llvm/llvm-build/bin/clang++
CC = /d/src/riyue/llvm/llvm-build/bin/clang
LLVM_CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags | sed 's/-fno-exceptions//; s/-fno-rtti//')
LLVM_LDFLAGS  := $(shell $(LLVM_CONFIG) --ldflags)
LLVM_LIBS     := $(shell $(LLVM_CONFIG) --libs core orcjit native nativecodegen mcparser option target analysis aarch64 webassembly)
LLVM_SYSLIBS  := $(shell $(LLVM_CONFIG) --system-libs)

# 引导.exe 仅作自举种子，无需调试信息（-g 使单文件编译慢约 3.6x），故不加 -g。
# 如要调试编译器本身，另用带 -g 的调试构建。
CXXFLAGS = -std=c++17 -MMD -MP -finput-charset=UTF-8 -fexec-charset=UTF-8 \
           -D_GNU_SOURCE -fexceptions -frtti \
           $(LLVM_CXXFLAGS)
CFLAGS = -Wall -MMD -MP -finput-charset=UTF-8 -fexec-charset=UTF-8
LDFLAGS = $(LLVM_LDFLAGS) $(LLVM_LIBS) $(LLVM_SYSLIBS) -lshell32 -lws2_32 -lpthread

# 默认并行编译；用 make -jN 可覆盖。
MAKEFLAGS += -j$(shell nproc 2>/dev/null || echo 4)

# 用 lld 链接（比默认 GNU ld 快约 2.7x）。
LDLLD = -fuse-ld=lld

# 源文件
前端_SRC = 源/前端/词法分析器.cpp 源/前端/语法分析器.cpp 源/前端/模块合并器.cpp 源/前端/诊断.cpp
后端_SRC = 源/后端/代码生成器.cpp 源/后端/代码生成器_表达式.cpp 源/后端/代码生成器_语句.cpp 源/后端/LLVM辅助.cpp
虚拟机_SRC = 源/虚拟机/字节码.cpp 源/虚拟机/字节码编译器.cpp 源/虚拟机/虚拟机.cpp
运行时_SRC = 源/运行时/运行时辅助.c

ALL_CXX_SRC = $(前端_SRC) $(后端_SRC) $(虚拟机_SRC) 源/主程序.cpp
ALL_OBJ = $(ALL_CXX_SRC:.cpp=.o) $(运行时_SRC:.c=.o)

# S4.6 路线2 构建链重排：
#   引导.exe = 宿主 C++ 编译器（仅作引导种子）
#   日月.exe = 由 引导.exe 编译 自举/主入口.心 产出的自举编译器（第一代自举）
TARGET = 引导.exe
SELFHOST_SRC = $(wildcard 自举/*.心) $(wildcard 工具/peg/*.心) 工具/日月/日月.文法

.PHONY: all clean 测试 单元测试 运行 自举 引导 重生成解析器 标准库冒烟

# 默认产出：自举 日月.exe（引导.exe 仅作前置种子）
all: 日月.exe

引导: $(TARGET)

# 代码生成按 AST 递归（if/else 链、嵌套块）在 -O0 下栈帧较大，默认 1MB 栈不足以
# 编译完整自举源码（在 发射函数定义节点 处栈溢出），显式设为 8MB。
$(TARGET): $(ALL_OBJ)
	$(CXX) $(LDLLD) -Wl,--stack,8388608 -o $@ $^ $(LDFLAGS)

# 自举 日月.exe：种子编译器 引导.exe 编译自举编译器源码 → 原生可执行文件
日月.exe: $(TARGET) $(SELFHOST_SRC)
	export MSYS2_ARG_CONV_EXCL='*'; \
	PATH=/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:$$PATH; export PATH; \
	./$(TARGET) 自举/主入口.心 日月.exe

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 单元测试
测试_SRC = 测试/单元测试.cpp
测试_OBJ = $(测试_SRC:.cpp=.o) $(前端_SRC:.cpp=.o) $(后端_SRC:.cpp=.o)

测试/%.o: 测试/%.cpp
	$(CXX) $(CXXFLAGS) -I源 -c $< -o $@

单元测试.exe: $(测试_OBJ)
	$(CXX) $(CXXFLAGS) $(LDLLD) -I源 -o $@ $^ $(LDFLAGS)

单元测试: 单元测试.exe
	./单元测试.exe

# 端到端测试（默认用自举 日月.exe）
测试: 日月.exe
	bash run_tests.sh

# 标准库冒烟：示例/标准库聚合测试.心 经 所有.心 导入全部标准库模块，
# 编译后运行，验证聚合链路（导入解析 + 跨模块调用 + 内建发射）无错误。
标准库冒烟: 日月.exe
	export MSYS2_ARG_CONV_EXCL='*'; \
	PATH=/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:$$PATH; export PATH; \
	./日月.exe 示例/标准库聚合测试.心 阶段/标准库冒烟.exe && \
	./阶段/标准库冒烟.exe

运行: 日月.exe
	./日月.exe 示例/简单测试.心 构建/简单测试.exe
	./构建/简单测试.exe

# 自举 IR 不动点验证（种子 引导.exe）：链路与判定见 不动点验证.sh，证据留证到 阶段/不动点.log。
# 链：引导.exe → 一级.exe → 中间码1.ll →(llvm-as/llc)→ 中间码1.o →(ld.lld)→ 二级.exe → 中间码2.ll；
#     中间码1.ll 与 中间码2.ll 必须完全一致（哈希相同即不动点/自举闭环成立）。
自举: $(TARGET)
	bash 不动点验证.sh

clean:
	rm -f $(ALL_OBJ) $(测试_OBJ) $(ALL_OBJ:.o=.d) $(测试_OBJ:.o=.d) $(TARGET) 日月.exe 单元测试.exe out_t.* out2.*

# 从文法重新生成自举解析器（改动 工具/peg/*.心 或 工具/日月/日月.文法 后必须执行一次；
# 该再生成无自动依赖，故独立成目标以免遗漏）。
重生成解析器: 日月.exe
	export MSYS2_ARG_CONV_EXCL='*'; \
	PATH=/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:$$PATH; export PATH; \
	./日月.exe 工具/peg/主程序.心 阶段/peg_cli.exe && \
	阶段/peg_cli.exe 生成自举库 工具/日月/日月.文法 自举/解析器.心

# 头文件依赖（由 -MMD 生成）；改动 .h 后自动重编受影响的 TU。
-include $(ALL_OBJ:.o=.d) $(测试_OBJ:.o=.d)

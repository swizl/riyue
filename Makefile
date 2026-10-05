# 编译器和 LLVM 库均使用 D:\src\riyue\llvm\llvm-build 自定义构建的中文 LLVM 23。
# 可用 `make LLVM_CONFIG=/path/to/llvm-config CXX=...` 覆盖。
LLVM_CONFIG ?= /d/src/riyue/llvm/llvm-build/bin/llvm-config

CXX = /d/src/riyue/llvm/llvm-build/bin/clang++
CC = /d/src/riyue/llvm/llvm-build/bin/clang
LLVM_CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags | sed 's/-fno-exceptions//; s/-fno-rtti//')
LLVM_LDFLAGS  := $(shell $(LLVM_CONFIG) --ldflags)
LLVM_LIBS     := $(shell $(LLVM_CONFIG) --libs core orcjit native nativecodegen mcparser option target analysis)
LLVM_SYSLIBS  := $(shell $(LLVM_CONFIG) --system-libs)

CXXFLAGS = -std=c++17 -g -finput-charset=UTF-8 -fexec-charset=UTF-8 \
           -D_GNU_SOURCE -fexceptions -frtti \
           $(LLVM_CXXFLAGS)
CFLAGS = -Wall -g -finput-charset=UTF-8 -fexec-charset=UTF-8
LDFLAGS = $(LLVM_LDFLAGS) $(LLVM_LIBS) $(LLVM_SYSLIBS) -lshell32 -lws2_32 -lpthread

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

.PHONY: all clean 测试 单元测试 运行 自举 引导

# 默认产出：自举 日月.exe（引导.exe 仅作前置种子）
all: 日月.exe

引导: $(TARGET)

$(TARGET): $(ALL_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

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
	$(CXX) $(CXXFLAGS) -I源 -o $@ $^ $(LDFLAGS)

单元测试: 单元测试.exe
	./单元测试.exe

# 端到端测试（默认用自举 日月.exe）
测试: 日月.exe
	bash run_tests.sh

运行: 日月.exe
	./日月.exe 示例/简单测试.心 构建/简单测试.exe
	./构建/简单测试.exe

# 自举 IR 不动点验证（种子 引导.exe）：
# 引导.exe 编译 自举/主入口.心 → 一级.exe；一级 编译自身 → 中间码1.ll；
# llvm-as/llc 汇编为 中间码1.o；与 运行时.o 经 ld.lld 链接 → 二级.exe；
# 二级 编译自身 → 中间码2.ll；中间码1.ll 与 中间码2.ll 必须完全一致（不动点/自举闭环成立）。
# 注意：MSYS2_ARG_CONV_EXCL='*' 禁止参数路径转换（否则 自举/主入口.心 被改写导致打不开）；
#       代价是原生工具参数不再转换，故 启动.o 与 -L 用 Windows 形式；llc 需 -filetype=obj。
自举: $(TARGET)
	export MSYS2_ARG_CONV_EXCL='*'; \
	PATH=/d/src/riyue/llvm/llvm-build/bin:/c/tools/msys64/mingw64/bin:$$PATH; export PATH; \
	./$(TARGET) 自举/主入口.心 阶段/一级.exe && \
	./阶段/一级.exe 自举/主入口.心 阶段/中间码1.ll && \
	llvm-as 阶段/中间码1.ll -o 阶段/中间码1.bc && \
	llc -mtriple=x86_64-w64-windows-gnu -filetype=obj 阶段/中间码1.bc -o 阶段/中间码1.o && \
	$(CC) -O0 -finput-charset=UTF-8 -fexec-charset=UTF-8 -c 源/运行时/运行时辅助.c -o 阶段/运行时.o && \
	cp 'C:/tools/msys64/mingw64/lib/crt2.o' 阶段/启动.o && \
	ld.lld 阶段/启动.o 阶段/中间码1.o 阶段/运行时.o -o 阶段/二级.exe -LC:/tools/msys64/mingw64/lib -LC:/tools/msys64/mingw64/lib/gcc/x86_64-w64-mingw32/16.1.0 -lmingw32 -lmingwex -lmsvcrt -lgcc -lmoldname -lws2_32 -lm -ladvapi32 -lshell32 -luser32 -lkernel32 && \
	./阶段/二级.exe 自举/主入口.心 阶段/中间码2.ll && \
	diff 阶段/中间码1.ll 阶段/中间码2.ll

clean:
	rm -f $(ALL_OBJ) $(测试_OBJ) $(TARGET) 日月.exe 单元测试.exe out_t.* out2.*

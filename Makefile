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
前端_SRC = 源/前端/词法分析器.cpp 源/前端/语法分析器.cpp 源/前端/诊断.cpp
后端_SRC = 源/后端/代码生成器.cpp 源/后端/代码生成器_表达式.cpp 源/后端/代码生成器_语句.cpp 源/后端/LLVM辅助.cpp
虚拟机_SRC = 源/虚拟机/字节码.cpp 源/虚拟机/字节码编译器.cpp 源/虚拟机/虚拟机.cpp
运行时_SRC = 源/运行时/运行时辅助.c

ALL_CXX_SRC = $(前端_SRC) $(后端_SRC) $(虚拟机_SRC) 源/主程序.cpp
ALL_OBJ = $(ALL_CXX_SRC:.cpp=.o) $(运行时_SRC:.c=.o)

TARGET = 日月.exe

.PHONY: all clean 测试 单元测试 运行 自举

all: $(TARGET)

$(TARGET): $(ALL_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

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

# 端到端测试
测试: $(TARGET)
	bash run_tests.sh

运行: $(TARGET)
	./$(TARGET) --调试 示例/简单测试.心 构建/简单测试.exe
	./构建/简单测试.exe

# 自举 C 不动点验证：
# 日月.exe 编译 自举/compiler.心 → stage1.exe（Stage1 编译器）；
# stage1 编译 compiler.心 → c1.c；clang 编译 c1.c+运行时辅助.c → stage2.exe（Stage2 编译器）；
# stage2 编译 compiler.心 → c2.c；c1.c 与 c2.c 必须完全一致（不动点/自举闭环成立）。
# 注意：输出路径用 ASCII（stage/），中文目录会让 MSYS2 ld 失败；
#       clang 需 -Wno-implicit-function-declaration（中文标识符 typo-correction 误报）。
自举: $(TARGET)
	./$(TARGET) 自举/compiler.心 stage/stage1.exe
	cp 自举/compiler.心 stage/compiler_copy.txt
	./stage/stage1.exe stage/compiler_copy.txt stage/c1.c
	$(CC) -O0 -finput-charset=UTF-8 -fexec-charset=UTF-8 \
	      -Wno-implicit-function-declaration -Wno-parentheses-equality \
	      -c stage/c1.c -o stage/c1.o
	$(CC) -O0 -finput-charset=UTF-8 -fexec-charset=UTF-8 \
	      -c 源/运行时/运行时辅助.c -o stage/rt.o
	$(CC) stage/c1.o stage/rt.o -o stage/stage2.exe -lws2_32 -lm
	./stage/stage2.exe stage/compiler_copy.txt stage/c2.c
	diff stage/c1.c stage/c2.c

clean:
	rm -f $(ALL_OBJ) $(测试_OBJ) $(TARGET) 单元测试.exe out_t.* out2.*

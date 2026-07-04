CXX = clang++
CC = clang
CXXFLAGS = -std=c++17 -Wall -Wextra -g -finput-charset=UTF-8 -fexec-charset=UTF-8 \
           -I/mingw64/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS \
           -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
CFLAGS = -Wall -g -finput-charset=UTF-8 -fexec-charset=UTF-8
LDFLAGS = -L/mingw64/lib -lLLVM-22 -lshell32 -lws2_32

# 源文件
前端_SRC = 源/前端/词法分析器.cpp 源/前端/语法分析器.cpp 源/前端/诊断.cpp
后端_SRC = 源/后端/代码生成器.cpp 源/后端/代码生成器_表达式.cpp 源/后端/代码生成器_语句.cpp 源/后端/LLVM辅助.cpp
虚拟机_SRC = 源/虚拟机/字节码.cpp 源/虚拟机/字节码编译器.cpp 源/虚拟机/虚拟机.cpp
运行时_SRC = 源/运行时/运行时辅助.c

ALL_CXX_SRC = $(前端_SRC) $(后端_SRC) $(虚拟机_SRC) 源/主程序.cpp
ALL_OBJ = $(ALL_CXX_SRC:.cpp=.o) $(运行时_SRC:.c=.o)

TARGET = 日月.exe

.PHONY: all clean 测试 单元测试 运行

all: $(TARGET)

$(TARGET): $(ALL_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 单元测试
测试_SRC = 测试/单元测试.cpp
测试_OBJ = $(测试_SRC:.cpp=.o) $(前端_SRC:.cpp=.o)

测试/%.o: 测试/%.cpp
	$(CXX) $(CXXFLAGS) -I源 -c $< -o $@

单元测试.exe: $(测试_OBJ)
	$(CXX) $(CXXFLAGS) -I源 -o $@ $^

单元测试: 单元测试.exe
	./单元测试.exe

# 端到端测试
测试: $(TARGET)
	bash run_tests.sh

运行: $(TARGET)
	./$(TARGET) --调试 示例/简单测试.心 构建/简单测试.exe
	./构建/简单测试.exe

clean:
	rm -f $(ALL_OBJ) $(测试_OBJ) $(TARGET) 单元测试.exe out_t.* out2.*

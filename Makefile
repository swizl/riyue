CXX = clang++
LLVM_PREFIX = /mingw64
CXXFLAGS = -std=c++17 -Wall -Wextra -g \
           -finput-charset=UTF-8 -fexec-charset=UTF-8 \
           -I$(LLVM_PREFIX)/include \
           -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS \
           -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
LDFLAGS = -L$(LLVM_PREFIX)/lib -lLLVM-22 -lshell32
SRCS = 源/主程序.cpp 源/词法分析器.cpp 源/语法分析器.cpp 源/代码生成器.cpp 源/字节码.cpp 源/字节码编译器.cpp
OBJS = $(patsubst 源/%.cpp,构建/%.o,$(SRCS))
COMPILER = 日月.exe
EXAMPLE_SRC = 示例/控制流.心
TARGET = 构建/输出.exe

all: 构建 $(COMPILER)
	@echo "编译器已生成: $(COMPILER)"
	@echo "用法:"
	@echo "  ./$(COMPILER) 输入.心 输出.exe    (编译为原生可执行文件)"
	@echo "  ./$(COMPILER) --运行 输入.心       (虚拟机直接运行)"
	@echo "  ./$(COMPILER) --字节码 输入.心 输出.riue (编译为字节码)"
	@echo "  ./$(COMPILER) --执行 输出.riue    (执行字节码)"

构建:
	@mkdir -p 构建

$(COMPILER): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(COMPILER) $(OBJS) $(LDFLAGS)

运行: $(COMPILER)
	MSYS_NO_PATHCONV=1 ./$(COMPILER) $(EXAMPLE_SRC) $(TARGET) && ./$(TARGET)

虚拟机: $(COMPILER)
	MSYS_NO_PATHCONV=1 ./$(COMPILER) --运行 $(EXAMPLE_SRC)

调试: $(COMPILER)
	MSYS_NO_PATHCONV=1 ./$(COMPILER) --调试 $(EXAMPLE_SRC) $(TARGET) && ./$(TARGET)

清理:
	rm -rf 构建 $(COMPILER)

测试:
	$(CXX) $(CXXFLAGS) -I源 -o 构建/单元测试.exe 测试/单元测试.cpp 源/词法分析器.cpp 源/语法分析器.cpp && 构建/单元测试.exe

构建/%.o: 源/%.cpp | 构建
	MSYS_NO_PATHCONV=1 $(CXX) $(CXXFLAGS) -c $< -o $@

# 解析器生成器
生成器:
	cd 工具 && $(MAKE)

生成: 生成器
	工具/解析器生成器.exe 日月.语法 生成

# 自举测试
自举: $(COMPILER)
	@echo "=== Stage 0: 基础功能 ==="
	MSYS_NO_PATHCONV=1 ./$(COMPILER) 自举/stage0_基础.心 构建/stage0 && ./构建/stage0.exe
	@echo ""
	@echo "=== Stage 1: 表达式求值器 ==="
	MSYS_NO_PATHCONV=1 ./$(COMPILER) 自举/stage1_求值器.心 构建/stage1 && ./构建/stage1.exe
	@echo ""
	@echo "=== Stage 2: 词法分析器 ==="
	MSYS_NO_PATHCONV=1 ./$(COMPILER) 自举/stage2_词法器.心 构建/stage2 && ./构建/stage2.exe
	@echo ""
	@echo "=== Stage 3: 子集编译器 ==="
	MSYS_NO_PATHCONV=1 ./$(COMPILER) 自举/stage3_编译器.心 构建/stage3 && ./构建/stage3.exe
	@echo ""
	@echo "=== 自举测试全部通过 ==="

自举VM: $(COMPILER)
	@echo "=== Stage 0 (VM) ==="
	MSYS_NO_PATHCONV=1 ./$(COMPILER) --运行 自举/stage0_基础.心
	@echo ""
	@echo "=== Stage 1 (VM) ==="
	MSYS_NO_PATHCONV=1 ./$(COMPILER) --运行 自举/stage1_求值器.心
	@echo ""
	@echo "=== Stage 2 (VM) ==="
	MSYS_NO_PATHCONV=1 ./$(COMPILER) --运行 自举/stage2_词法器.心
	@echo ""
	@echo "=== Stage 3 (VM) ==="
	MSYS_NO_PATHCONV=1 ./$(COMPILER) --运行 自举/stage3_编译器.心
	@echo ""
	@echo "=== 自举测试全部通过 (VM) ==="

.PHONY: all 运行 虚拟机 调试 清理 测试 生成器 生成 自举 自举VM

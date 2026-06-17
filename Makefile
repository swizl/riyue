CXX = clang++
LLVM_PREFIX = /mingw64
CXXFLAGS = -std=c++17 -Wall -Wextra -g \
           -finput-charset=UTF-8 -fexec-charset=UTF-8 \
           -I$(LLVM_PREFIX)/include \
           -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS \
           -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
LDFLAGS = -L$(LLVM_PREFIX)/lib -lLLVM-22 -lshell32 -lws2_32
SRCS = 源/主程序.cpp \
       源/前端/词法分析器.cpp 源/前端/语法分析器.cpp \
       源/后端/代码生成器.cpp 源/后端/代码生成器_语句.cpp 源/后端/代码生成器_表达式.cpp \
       源/虚拟机/字节码.cpp 源/虚拟机/字节码编译器.cpp 源/虚拟机/虚拟机.cpp
OBJS = $(SRCS:.cpp=.o)
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
	rm -rf 构建 $(COMPILER) 源/**/*.o 源/*.o

测试:
	$(CXX) $(CXXFLAGS) -I源 -o 构建/单元测试.exe 测试/单元测试.cpp 源/前端/词法分析器.cpp 源/前端/语法分析器.cpp && 构建/单元测试.exe

测试语言: $(COMPILER)
	MSYS_NO_PATHCONV=1 ./$(COMPILER) 测试/语言特性测试.心 构建/语言测试 && ./构建/语言测试.exe

测试语言VM: $(COMPILER)
	MSYS_NO_PATHCONV=1 ./$(COMPILER) --运行 测试/语言特性测试.心

%.o: %.cpp
	MSYS_NO_PATHCONV=1 $(CXX) $(CXXFLAGS) -c $< -o $@

# 解析器生成器
生成器:
	cd 工具 && $(MAKE)

生成: 生成器
	工具/解析器生成器.exe 语法/日月.语法 生成

# 日月编译器验证
编译器: $(COMPILER)
	@echo "=== 日月编译器验证 ==="
	@echo "[种子] 种子验证..."
	MSYS_NO_PATHCONV=1 ./$(COMPILER) 编译器/种子验证.心 构建/种子验证 && ./构建/种子验证.exe
	@echo ""
	@echo "[词法] 词法器..."
	MSYS_NO_PATHCONV=1 ./$(COMPILER) 编译器/词法器.心 构建/词法器 && ./构建/词法器.exe
	@echo ""
	@echo "[语法] 语法器..."
	MSYS_NO_PATHCONV=1 ./$(COMPILER) 编译器/语法器.心 构建/语法器 && ./构建/语法器.exe
	@echo ""
	@echo "=== 编译器验证完成 ==="

编译器虚拟机: $(COMPILER)
	@echo "=== 日月编译器验证 (虚拟机) ==="
	@echo "[种子] 种子验证..."
	MSYS_NO_PATHCONV=1 ./$(COMPILER) --运行 编译器/种子验证.心
	@echo ""
	@echo "[词法] 词法器..."
	MSYS_NO_PATHCONV=1 ./$(COMPILER) --运行 编译器/词法器.心
	@echo ""
	@echo "[语法] 语法器..."
	MSYS_NO_PATHCONV=1 ./$(COMPILER) --运行 编译器/语法器.心
	@echo ""
	@echo "=== 编译器验证完成 (虚拟机) ==="

# 开发工具
工具/格式化工具.exe: 工具/格式化工具.cpp
	$(CXX) $(CXXFLAGS) -o 工具/格式化工具.exe 工具/格式化工具.cpp -lshell32

工具/包管理器.exe: 工具/包管理器.cpp
	$(CXX) $(CXXFLAGS) -o 工具/包管理器.exe 工具/包管理器.cpp -lshell32

工具: 工具/格式化工具.exe 工具/包管理器.exe
	@echo "工具已生成:"
	@echo "  工具/格式化工具.exe  - 代码格式化"
	@echo "  工具/包管理器.exe    - 包管理"

格式化: 工具/格式化工具.exe
	@echo "格式化示例..."
	工具/格式化工具.exe 示例/控制流.心

包初始化: 工具/包管理器.exe
	工具/包管理器.exe 初始化

包列表: 工具/包管理器.exe
	工具/包管理器.exe 列表

.PHONY: all 运行 虚拟机 调试 清理 测试 测试语言 测试语言VM 生成器 生成 编译器 编译器虚拟机 工具 格式化 包初始化 包列表

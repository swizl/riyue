CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -finput-charset=UTF-8 -fexec-charset=UTF-8
SRCS = 主程序.cpp 解析器生成器.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = 解析器生成器.exe

all: $(TARGET)
	@echo "解析器生成器已生成: $(TARGET)"

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

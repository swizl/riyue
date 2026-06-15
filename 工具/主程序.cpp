#include "解析器生成器.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "用法: 解析器生成器 <语法文件> <输出目录>" << std::endl;
        std::cerr << "示例: 解析器生成器 日月.语法 生成" << std::endl;
        return 1;
    }

    std::string 语法文件 = argv[1];
    std::string 输出目录 = argv[2];

    解析器生成器 生成器;

    if (!生成器.解析(语法文件)) {
        std::cerr << "解析语法文件失败" << std::endl;
        return 1;
    }

    生成器.打印信息();
    生成器.生成(输出目录);

    std::cout << "生成完成!" << std::endl;
    return 0;
}

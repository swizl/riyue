// 日月包管理器
// 用法:
//   包管理器 安装 <包名>    - 安装包
//   包管理器 列表          - 列出已安装的包
//   包管理器 移除 <包名>    - 移除包

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace fs = std::filesystem;

// 包管理器配置
const std::string 包目录 = ".日月包";
const std::string 索引文件 = 包目录 + "/索引.json";

// 获取用户主目录
std::string 获取主目录() {
#ifdef _WIN32
    char* 用户配置 = getenv("USERPROFILE");
    return 用户配置 ? 用户配置 : ".";
#else
    char* 用户配置 = getenv("HOME");
    return 用户配置 ? 用户配置 : ".";
#endif
}

// 获取包存储目录
std::string 获取包目录() {
    return 获取主目录() + "/" + 包目录;
}

// 确保包目录存在
bool 确保目录存在(const std::string& 路径) {
    if (!fs::exists(路径)) {
        return fs::create_directories(路径);
    }
    return true;
}

// 列出已安装的包
void 列出已安装的包() {
    std::string 目录 = 获取包目录();
    if (!fs::exists(目录)) {
        std::cout << "没有已安装的包" << std::endl;
        return;
    }

    std::cout << "已安装的包:" << std::endl;
    int 计数 = 0;
    for (const auto& 入口 : fs::directory_iterator(目录)) {
        if (入口.is_directory()) {
            std::string 包名 = 入口.path().filename().string();
            std::cout << "  - " << 包名 << std::endl;
            计数++;
        }
    }
    if (计数 == 0) {
        std::cout << "  (无)" << std::endl;
    }
}

// 安装包（从本地目录或文件）
bool 安装包(const std::string& 包名) {
    std::string 源路径 = 包名;
    bool 是单文件 = false;
    
    // 检查是否是单个文件
    if (fs::exists(源路径) && fs::is_regular_file(源路径)) {
        是单文件 = true;
    } else if (!fs::exists(源路径) || !fs::is_directory(源路径)) {
        // 尝试添加.心后缀
        源路径 = 包名 + ".心";
        if (fs::exists(源路径) && fs::is_regular_file(源路径)) {
            是单文件 = true;
        } else {
            // 尝试在示例目录中查找
            源路径 = "示例/" + 包名;
            if (fs::exists(源路径) && fs::is_directory(源路径)) {
                是单文件 = false;
            } else {
                源路径 = "示例/" + 包名 + ".心";
                if (fs::exists(源路径) && fs::is_regular_file(源路径)) {
                    是单文件 = true;
                } else {
                    std::cerr << "找不到包: " << 包名 << std::endl;
                    return false;
                }
            }
        }
    }

    std::string 目标目录 = 获取包目录() + "/" + 包名;
    if (fs::exists(目标目录)) {
        std::cout << "包已存在: " << 包名 << std::endl;
        return true;
    }

    try {
        fs::create_directories(目标目录);
        if (是单文件) {
            // 复制单个文件
            fs::copy_file(源路径, 目标目录 + "/" + fs::path(源路径).filename().string());
        } else {
            // 复制目录中的所有.心文件
            for (const auto& 入口 : fs::directory_iterator(源路径)) {
                if (入口.is_regular_file() && 入口.path().extension() == ".心") {
                    fs::copy_file(入口.path(), 目标目录 + "/" + 入口.path().filename().string());
                }
            }
        }
        std::cout << "安装成功: " << 包名 << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "安装失败: " << e.what() << std::endl;
        return false;
    }
}

// 移除包
bool 移除包(const std::string& 包名) {
    std::string 目标目录 = 获取包目录() + "/" + 包名;
    if (!fs::exists(目标目录)) {
        std::cerr << "包不存在: " << 包名 << std::endl;
        return false;
    }

    try {
        fs::remove_all(目标目录);
        std::cout << "移除成功: " << 包名 << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "移除失败: " << e.what() << std::endl;
        return false;
    }
}

void 显示用法(const std::string& 程序名) {
    std::cerr << "用法:" << std::endl;
    std::cerr << "  " << 程序名 << " 安装 <包名>    - 安装包" << std::endl;
    std::cerr << "  " << 程序名 << " 列表          - 列出已安装的包" << std::endl;
    std::cerr << "  " << 程序名 << " 移除 <包名>    - 移除包" << std::endl;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    // 获取UTF-8命令行参数
    int wargc;
    wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    std::vector<std::string> 参数;
    for (int i = 0; i < wargc; i++) {
        int len = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
        std::string arg(len - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, &arg[0], len, nullptr, nullptr);
        参数.push_back(arg);
    }
    LocalFree(wargv);
#else
    std::vector<std::string> 参数;
    for (int i = 0; i < argc; i++) {
        参数.push_back(argv[i]);
    }
#endif

    if (参数.size() < 2) {
        显示用法(参数[0]);
        return 1;
    }

    std::string 命令 = 参数[1];

    if (命令 == "列表") {
        列出已安装的包();
        return 0;
    }

    if (命令 == "安装") {
        if (参数.size() < 3) {
            std::cerr << "用法: " << 参数[0] << " 安装 <包名>" << std::endl;
            return 1;
        }
        return 安装包(参数[2]) ? 0 : 1;
    }

    if (命令 == "移除") {
        if (参数.size() < 3) {
            std::cerr << "用法: " << 参数[0] << " 移除 <包名>" << std::endl;
            return 1;
        }
        return 移除包(参数[2]) ? 0 : 1;
    }

    std::cerr << "未知命令: " << 命令 << std::endl;
    显示用法(参数[0]);
    return 1;
}

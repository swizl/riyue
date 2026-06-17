// 日月包管理器
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <windows.h>
#include <shellapi.h>

namespace fs = std::filesystem;

bool 调试模式 = false;

static std::string 宽字符转UTF8(const std::wstring& 宽串) {
    if (宽串.empty()) return "";
    int 长度 = WideCharToMultiByte(CP_UTF8, 0, 宽串.c_str(), (int)宽串.size(), nullptr, 0, nullptr, nullptr);
    std::string 结果(长度, 0);
    WideCharToMultiByte(CP_UTF8, 0, 宽串.c_str(), (int)宽串.size(), &结果[0], 长度, nullptr, nullptr);
    return 结果;
}

static std::vector<std::string> 获取UTF8参数() {
    int 参数数量 = 0;
    LPWSTR* 宽参数 = CommandLineToArgvW(GetCommandLineW(), &参数数量);
    std::vector<std::string> 结果;
    for (int i = 0; i < 参数数量; i++) {
        结果.push_back(宽字符转UTF8(宽参数[i]));
    }
    LocalFree(宽参数);
    return 结果;
}

void 显示用法(const std::string& 程序名) {
    std::cerr << "日月包管理器" << std::endl;
    std::cerr << "用法:" << std::endl;
    std::cerr << "  " << 程序名 << " 初始化                # 初始化项目" << std::endl;
    std::cerr << "  " << 程序名 << " 安装 <包名>            # 安装包" << std::endl;
    std::cerr << "  " << 程序名 << " 卸载 <包名>            # 卸载包" << std::endl;
    std::cerr << "  " << 程序名 << " 列表                  # 列出已安装的包" << std::endl;
    std::cerr << "  " << 程序名 << " 搜索 <关键词>          # 搜索包" << std::endl;
    std::cerr << "  " << 程序名 << " 更新                  # 更新所有包" << std::endl;
}

// 包配置文件结构
struct 包配置 {
    std::string 名称;
    std::string 版本;
    std::string 描述;
    std::string 作者;
    std::vector<std::string> 依赖;
};

// 读取配置文件
包配置 读取配置(const std::string& 文件路径) {
    包配置 配置;

    // 将UTF-8路径转换为宽字符路径
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 文件路径.c_str(), (int)文件路径.size(), nullptr, 0);
    std::wstring 宽路径(宽长度, 0);
    MultiByteToWideChar(CP_UTF8, 0, 文件路径.c_str(), (int)文件路径.size(), &宽路径[0], 宽长度);

    FILE* 文件 = _wfopen(宽路径.c_str(), L"rb");
    if (!文件) return 配置;

    fseek(文件, 0, SEEK_END);
    long 文件大小 = ftell(文件);
    fseek(文件, 0, SEEK_SET);

    std::string 内容(文件大小, 0);
    fread(&内容[0], 1, 文件大小, 文件);
    fclose(文件);

    std::istringstream 输入(内容);
    std::string 行;
    while (std::getline(输入, 行)) {
        size_t 等号位置 = 行.find('=');
        if (等号位置 == std::string::npos) continue;

        std::string 键 = 行.substr(0, 等号位置);
        std::string 值 = 行.substr(等号位置 + 1);

        // 去除空白
        键.erase(0, 键.find_first_not_of(" \t"));
        键.erase(键.find_last_not_of(" \t") + 1);
        值.erase(0, 值.find_first_not_of(" \t"));
        值.erase(值.find_last_not_of(" \t") + 1);

        // 去除引号
        if (值.front() == '"' && 值.back() == '"') {
            值 = 值.substr(1, 值.size() - 2);
        }

        if (键 == "名称") 配置.名称 = 值;
        else if (键 == "版本") 配置.版本 = 值;
        else if (键 == "描述") 配置.描述 = 值;
        else if (键 == "作者") 配置.作者 = 值;
        else if (键 == "依赖") {
            // 解析依赖列表
            std::istringstream 依赖流(值);
            std::string 依赖项;
            while (std::getline(依赖流, 依赖项, ',')) {
                依赖项.erase(0, 依赖项.find_first_not_of(" \t"));
                依赖项.erase(依赖项.find_last_not_of(" \t") + 1);
                if (!依赖项.empty()) {
                    配置.依赖.push_back(依赖项);
                }
            }
        }
    }

    return 配置;
}

// 写入配置文件
void 写入配置(const std::string& 文件路径, const 包配置& 配置) {
    // 将UTF-8路径转换为宽字符路径
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 文件路径.c_str(), (int)文件路径.size(), nullptr, 0);
    std::wstring 宽路径(宽长度, 0);
    MultiByteToWideChar(CP_UTF8, 0, 文件路径.c_str(), (int)文件路径.size(), &宽路径[0], 宽长度);

    FILE* 文件 = _wfopen(宽路径.c_str(), L"wb");
    if (!文件) {
        std::cerr << "错误: 无法写入配置文件 '" << 文件路径 << "'" << std::endl;
        return;
    }

    std::string 内容;
    内容 += "名称 = \"" + 配置.名称 + "\"\n";
    内容 += "版本 = \"" + 配置.版本 + "\"\n";
    内容 += "描述 = \"" + 配置.描述 + "\"\n";
    内容 += "作者 = \"" + 配置.作者 + "\"\n";

    if (!配置.依赖.empty()) {
        内容 += "依赖 = ";
        for (size_t i = 0; i < 配置.依赖.size(); i++) {
            if (i > 0) 内容 += ", ";
            内容 += 配置.依赖[i];
        }
        内容 += "\n";
    }

    fwrite(内容.c_str(), 1, 内容.size(), 文件);
    fclose(文件);
}

// 初始化项目
int 初始化项目() {
    if (fs::exists("日月.toml")) {
        std::cerr << "错误: 项目已存在" << std::endl;
        return 1;
    }

    包配置 配置;
    配置.名称 = "我的项目";
    配置.版本 = "0.1.0";
    配置.描述 = "一个日月项目";
    配置.作者 = "";

    写入配置("日月.toml", 配置);

    // 创建目录结构
    fs::create_directories("源");
    fs::create_directories("示例");
    fs::create_directories("测试");

    std::cout << "项目初始化完成" << std::endl;
    std::cout << "  日月.toml    - 项目配置" << std::endl;
    std::cout << "  源/          - 源代码" << std::endl;
    std::cout << "  示例/        - 示例代码" << std::endl;
    std::cout << "  测试/        - 测试代码" << std::endl;

    return 0;
}

// 安装包
int 安装包(const std::string& 包名) {
    if (!fs::exists("日月.toml")) {
        std::cerr << "错误: 请先运行 '日月包 初始化' 初始化项目" << std::endl;
        return 1;
    }

    包配置 配置 = 读取配置("日月.toml");

    // 检查是否已安装
    for (const auto& 依赖 : 配置.依赖) {
        if (依赖 == 包名) {
            std::cout << "包 '" << 包名 << "' 已安装" << std::endl;
            return 0;
        }
    }

    // 添加到依赖列表
    配置.依赖.push_back(包名);
    写入配置("日月.toml", 配置);

    std::cout << "安装包: " << 包名 << std::endl;
    std::cout << "  已添加到日月.toml" << std::endl;

    return 0;
}

// 卸载包
int 卸载包(const std::string& 包名) {
    if (!fs::exists("日月.toml")) {
        std::cerr << "错误: 请先运行 '日月包 初始化' 初始化项目" << std::endl;
        return 1;
    }

    包配置 配置 = 读取配置("日月.toml");

    // 查找并移除
    auto it = std::find(配置.依赖.begin(), 配置.依赖.end(), 包名);
    if (it == 配置.依赖.end()) {
        std::cerr << "错误: 包 '" << 包名 << "' 未安装" << std::endl;
        return 1;
    }

    配置.依赖.erase(it);
    写入配置("日月.toml", 配置);

    std::cout << "卸载包: " << 包名 << std::endl;
    std::cout << "  已从日月.toml 移除" << std::endl;

    return 0;
}

// 列出已安装的包
int 列出包() {
    if (!fs::exists("日月.toml")) {
        std::cerr << "错误: 请先运行 '日月包 初始化' 初始化项目" << std::endl;
        return 1;
    }

    包配置 配置 = 读取配置("日月.toml");

    std::cout << "项目: " << 配置.名称 << " v" << 配置.版本 << std::endl;
    if (!配置.描述.empty()) {
        std::cout << "描述: " << 配置.描述 << std::endl;
    }
    if (!配置.作者.empty()) {
        std::cout << "作者: " << 配置.作者 << std::endl;
    }

    if (配置.依赖.empty()) {
        std::cout << "\n没有依赖" << std::endl;
    } else {
        std::cout << "\n依赖 (" << 配置.依赖.size() << "):" << std::endl;
        for (const auto& 依赖 : 配置.依赖) {
            std::cout << "  - " << 依赖 << std::endl;
        }
    }

    return 0;
}

// 搜索包（模拟）
int 搜索包(const std::string& 关键词) {
    std::cout << "搜索包: " << 关键词 << std::endl;
    std::cout << std::endl;
    std::cout << "注意: 包管理器目前为演示版本" << std::endl;
    std::cout << "未来将支持从远程仓库搜索和下载包" << std::endl;

    return 0;
}

// 更新包（模拟）
int 更新包() {
    if (!fs::exists("日月.toml")) {
        std::cerr << "错误: 请先运行 '日月包 初始化' 初始化项目" << std::endl;
        return 1;
    }

    包配置 配置 = 读取配置("日月.toml");

    if (配置.依赖.empty()) {
        std::cout << "没有需要更新的包" << std::endl;
        return 0;
    }

    std::cout << "更新包..." << std::endl;
    for (const auto& 依赖 : 配置.依赖) {
        std::cout << "  检查 " << 依赖 << "..." << std::endl;
    }

    std::cout << std::endl;
    std::cout << "注意: 包管理器目前为演示版本" << std::endl;
    std::cout << "未来将支持从远程仓库更新包" << std::endl;

    return 0;
}

int main() {
    auto 参数 = 获取UTF8参数();
    int argc = (int)参数.size();

    if (argc < 2) {
        显示用法(参数[0]);
        return 1;
    }

    std::string 命令 = 参数[1];

    if (命令 == "初始化") {
        return 初始化项目();
    } else if (命令 == "安装") {
        if (argc < 3) {
            std::cerr << "错误: 请指定包名" << std::endl;
            return 1;
        }
        return 安装包(参数[2]);
    } else if (命令 == "卸载") {
        if (argc < 3) {
            std::cerr << "错误: 请指定包名" << std::endl;
            return 1;
        }
        return 卸载包(参数[2]);
    } else if (命令 == "列表") {
        return 列出包();
    } else if (命令 == "搜索") {
        if (argc < 3) {
            std::cerr << "错误: 请指定搜索关键词" << std::endl;
            return 1;
        }
        return 搜索包(参数[2]);
    } else if (命令 == "更新") {
        return 更新包();
    } else if (命令 == "帮助" || 命令 == "--help" || 命令 == "-h") {
        显示用法(参数[0]);
        return 0;
    } else {
        std::cerr << "错误: 未知命令 '" << 命令 << "'" << std::endl;
        显示用法(参数[0]);
        return 1;
    }

    return 0;
}

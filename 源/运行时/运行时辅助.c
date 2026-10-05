// 日月运行时辅助函数
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#endif

// 用路径打开文件（Windows 下转换为宽字符路径）。
// Windows 传给 main 的 argv 是本地代码页(CP_ACP, 如 GBK)字节，而源码/字符串常量里
// 通常是 UTF-8 字节，故先按 UTF-8 转换打开，失败再按本地代码页重试。
static FILE* UTF8打开文件(const char* 路径, const char* 模式) {
#ifdef _WIN32
    wchar_t 宽路径[1024];
    wchar_t 宽模式[64];
    UINT 代码页表[2] = { CP_UTF8, CP_ACP };
    DWORD 标志表[2] = { MB_ERR_INVALID_CHARS, 0 };
    int 索引;
    if (MultiByteToWideChar(CP_UTF8, 0, 模式, -1, 宽模式, 64) <= 0) return NULL;
    for (索引 = 0; 索引 < 2; 索引++) {
        if (MultiByteToWideChar(代码页表[索引], 标志表[索引], 路径, -1, 宽路径, 1024) > 0) {
            FILE* fp = _wfopen(宽路径, 宽模式);
            if (fp) return fp;
        }
    }
    return NULL;
#else
    return fopen(路径, 模式);
#endif
}

// 将UTF-8字符位置转换为字节位置
int 字符位置到字节位置(const char* str, int char_pos) {
    int byte_pos = 0;
    int current_char = 0;

    while (str[byte_pos] != '\0' && current_char < char_pos) {
        unsigned char c = (unsigned char)str[byte_pos];
        if (c < 0x80) {
            byte_pos += 1;
        } else if ((c & 0xE0) == 0xC0) {
            byte_pos += 2;
        } else if ((c & 0xF0) == 0xE0) {
            byte_pos += 3;
        } else if ((c & 0xF8) == 0xF0) {
            byte_pos += 4;
        } else {
            byte_pos += 1;
        }
        current_char++;
    }
    return byte_pos;
}

// 获取UTF-8字符串的字符数
int 获取字符数(const char* str) {
    int count = 0;
    int i = 0;
    while (str[i] != '\0') {
        unsigned char c = (unsigned char)str[i];
        if (c < 0x80) {
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            i += 4;
        } else {
            i += 1;
        }
        count++;
    }
    return count;
}

// 映射辅助函数
// 键数组和值数组是平行的，大小固定为1024

// 在键数组中查找键，返回索引，未找到返回-1
int 映射查找(const char** keys, int size, const char* key) {
    for (int i = 0; i < size; i++) {
        if (keys[i] != NULL && strcmp(keys[i], key) == 0) {
            return i;
        }
    }
    return -1;
}

// 在映射中设置键值对，返回新大小
int 映射设置(const char** keys, const char** values, int size, const char* key, const char* value) {
    int idx = 映射查找(keys, size, key);
    if (idx >= 0) {
        values[idx] = value;
        return size;
    }
    if (size < 1024) {
        keys[size] = key;
        values[size] = value;
        return size + 1;
    }
    return size;
}

// 从映射中获取值，未找到返回空字符串
const char* 映射获取(const char** keys, const char** values, int size, const char* key) {
    int idx = 映射查找(keys, size, key);
    if (idx >= 0) {
        return values[idx];
    }
    return "";
}

// 数组排序（冒泡排序）
void 数组排序(int* arr, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// 数组反转
void 数组反转(int* arr, int size) {
    for (int i = 0; i < size / 2; i++) {
        int temp = arr[i];
        arr[i] = arr[size - 1 - i];
        arr[size - 1 - i] = temp;
    }
}

// 数组包含检查
int 数组包含(int* arr, int size, int 元素) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == 元素) return 1;
    }
    return 0;
}

// 字符串分割 - 返回分割后的子串数组
// 注意：这个函数会修改输入字符串
const char** 字符串分割(char* str, const char* 分隔符, int* 结果数量) {
    static const char* 结果[256];
    *结果数量 = 0;
    char* token = strtok(str, 分隔符);
    while (token != NULL && *结果数量 < 256) {
        结果[*结果数量] = token;
        (*结果数量)++;
        token = strtok(NULL, 分隔符);
    }
    return 结果;
}

// 字符串连接
char* 文本连接(const char** 字符串数组, int 数量, const char* 分隔符) {
    static char 缓冲区[4096];
    缓冲区[0] = '\0';
    for (int i = 0; i < 数量; i++) {
        if (i > 0) strcat(缓冲区, 分隔符);
        strcat(缓冲区, 字符串数组[i]);
    }
    return 缓冲区;
}

// 字符串格式化（简单版本，支持%d）
char* 字符串格式化(const char* 格式模板, int 参数1, int 参数2, int 参数3) {
    static char 缓冲区[4096];
    缓冲区[0] = '\0';
    int 参数索引 = 0;
    int 参数值[] = {参数1, 参数2, 参数3};
    int i = 0;
    int j = 0;
    while (格式模板[i] != '\0' && j < 4095) {
        if (格式模板[i] == '%' && 格式模板[i + 1] == 'd' && 参数索引 < 3) {
            char 数字[32];
            sprintf(数字, "%d", 参数值[参数索引]);
            strcat(缓冲区, 数字);
            j += strlen(数字);
            i += 2;
            参数索引++;
        } else {
            缓冲区[j++] = 格式模板[i++];
        }
    }
    缓冲区[j] = '\0';
    return 缓冲区;
}

// ==================== 日期时间函数 ====================

// 获取当前时间戳（秒）
long long 获取时间戳() {
    return (long long)time(NULL);
}

// 获取当前年份
int 获取年份() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    return t->tm_year + 1900;
}

// 获取当前月份 (1-12)
int 获取月份() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    return t->tm_mon + 1;
}

// 获取当前日 (1-31)
int 获取日() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    return t->tm_mday;
}

// 获取当前小时 (0-23)
int 获取小时() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    return t->tm_hour;
}

// 获取当前分钟 (0-59)
int 获取分钟() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    return t->tm_min;
}

// 获取当前秒 (0-59)
int 获取秒() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    return t->tm_sec;
}

// 获取星期几 (0=周日, 1=周一, ..., 6=周六)
int 获取星期() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    return t->tm_wday;
}

// 格式化时间字符串
const char* 格式化时间(const char* 格式) {
    static char 缓冲区[256];
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(缓冲区, sizeof(缓冲区), 格式, t);
    return 缓冲区;
}

// 获取日期时间字符串 "YYYY-MM-DD HH:MM:SS"
const char* 获取日期时间() {
    return 格式化时间("%Y-%m-%d %H:%M:%S");
}

// 获取日期字符串 "YYYY-MM-DD"
const char* 获取日期() {
    return 格式化时间("%Y-%m-%d");
}

// 获取时间字符串 "HH:MM:SS"
const char* 获取时间() {
    return 格式化时间("%H:%M:%S");
}

// 计算两个时间戳之间的差值（秒）
long long 时间差(long long 时间戳1, long long 时间戳2) {
    return 时间戳2 - 时间戳1;
}

// 程序休眠（毫秒）
#ifdef _WIN32
void 休眠(int 毫秒) {
    Sleep(毫秒);
}
#else
#include <unistd.h>
void 休眠(int 毫秒) {
    usleep(毫秒 * 1000);
}
#endif

// 测量程序运行时间
static clock_t 开始时间 = 0;
void 开始计时() {
    开始时间 = clock();
}

double 结束计时() {
    if (开始时间 == 0) return 0.0;
    clock_t 结束时间 = clock();
    double 耗时 = (double)(结束时间 - 开始时间) / CLOCKS_PER_SEC;
    开始时间 = 0;
    return 耗时;
}

// ==================== JSON 函数 ====================

// 简单的JSON解析 - 提取字符串值
const char* JSON获取字符串(const char* JSON, const char* 键) {
    static char 缓冲区[4096];
    char 搜索键[256];
    sprintf(搜索键, "\"%s\"", 键);

    const char* 位置 = strstr(JSON, 搜索键);
    if (!位置) return "";

    位置 = strchr(位置 + strlen(搜索键), ':');
    if (!位置) return "";
    位置++;

    while (*位置 == ' ' || *位置 == '\t') 位置++;

    if (*位置 == '"') {
        位置++;
        int i = 0;
        while (*位置 != '"' && *位置 != '\0' && i < 4095) {
            if (*位置 == '\\' && *(位置 + 1) == '"') {
                缓冲区[i++] = '"';
                位置 += 2;
            } else {
                缓冲区[i++] = *位置++;
            }
        }
        缓冲区[i] = '\0';
        return 缓冲区;
    }
    return "";
}

// 简单的JSON解析 - 提取整数值
int JSON获取整数(const char* JSON, const char* 键) {
    char 搜索键[256];
    sprintf(搜索键, "\"%s\"", 键);

    const char* 位置 = strstr(JSON, 搜索键);
    if (!位置) return 0;

    位置 = strchr(位置 + strlen(搜索键), ':');
    if (!位置) return 0;
    位置++;

    while (*位置 == ' ' || *位置 == '\t') 位置++;

    return atoi(位置);
}

// 简单的JSON解析 - 提取浮点数值
double JSON获取浮点(const char* JSON, const char* 键) {
    char 搜索键[256];
    sprintf(搜索键, "\"%s\"", 键);

    const char* 位置 = strstr(JSON, 搜索键);
    if (!位置) return 0.0;

    位置 = strchr(位置 + strlen(搜索键), ':');
    if (!位置) return 0.0;
    位置++;

    while (*位置 == ' ' || *位置 == '\t') 位置++;

    return atof(位置);
}

// 简单的JSON解析 - 提取布尔值
int JSON获取布尔(const char* JSON, const char* 键) {
    char 搜索键[256];
    sprintf(搜索键, "\"%s\"", 键);

    const char* 位置 = strstr(JSON, 搜索键);
    if (!位置) return 0;

    位置 = strchr(位置 + strlen(搜索键), ':');
    if (!位置) return 0;
    位置++;

    while (*位置 == ' ' || *位置 == '\t') 位置++;

    if (strncmp(位置, "true", 4) == 0) return 1;
    return 0;
}

// 生成JSON字符串
const char* JSON创建字符串(const char* 键, const char* 值) {
    static char 缓冲区[4096];
    sprintf(缓冲区, "{\"%s\":\"%s\"}", 键, 值);
    return 缓冲区;
}

// 生成JSON整数
const char* JSON创建整数(const char* 键, int 值) {
    static char 缓冲区[4096];
    sprintf(缓冲区, "{\"%s\":%d}", 键, 值);
    return 缓冲区;
}

// 生成JSON浮点数
const char* JSON创建浮点(const char* 键, double 值) {
    static char 缓冲区[4096];
    sprintf(缓冲区, "{\"%s\":%g}", 键, 值);
    return 缓冲区;
}

// 生成JSON布尔值
const char* JSON创建布尔(const char* 键, int 值) {
    static char 缓冲区[4096];
    sprintf(缓冲区, "{\"%s\":%s}", 键, 值 ? "true" : "false");
    return 缓冲区;
}

// 检查JSON中是否存在键
int JSON存在(const char* JSON, const char* 键) {
    char 搜索键[256];
    sprintf(搜索键, "\"%s\"", 键);
    return strstr(JSON, 搜索键) != NULL;
}

// ==================== 加密/哈希函数 ====================

// 简单的哈希函数（DJB2算法）
unsigned long 简单哈希(const char* 输入) {
    unsigned long 哈希值 = 5381;
    int c;
    while ((c = *输入++)) {
        哈希值 = ((哈希值 << 5) + 哈希值) + c;
    }
    return 哈希值;
}

// 简单的字符串加密（凯撒密码）
const char* 简单加密(const char* 输入, int 密钥) {
    static char 缓冲区[4096];
    int i = 0;
    while (输入[i] != '\0' && i < 4095) {
        if (输入[i] >= 'a' && 输入[i] <= 'z') {
            缓冲区[i] = 'a' + (输入[i] - 'a' + 密钥) % 26;
        } else if (输入[i] >= 'A' && 输入[i] <= 'Z') {
            缓冲区[i] = 'A' + (输入[i] - 'A' + 密钥) % 26;
        } else {
            缓冲区[i] = 输入[i];
        }
        i++;
    }
    缓冲区[i] = '\0';
    return 缓冲区;
}

// 简单的字符串解密（凯撒密码）
const char* 简单解密(const char* 输入, int 密钥) {
    return 简单加密(输入, 26 - (密钥 % 26));
}

// 生成随机数
int 随机数(int 最小值, int 最大值) {
    static int 已初始化 = 0;
    if (!已初始化) {
        srand((unsigned int)time(NULL));
        已初始化 = 1;
    }
    return 最小值 + rand() % (最大值 - 最小值 + 1);
}

// 生成随机浮点数 (0.0 - 1.0)
double 随机浮点() {
    static int 已初始化 = 0;
    if (!已初始化) {
        srand((unsigned int)time(NULL));
        已初始化 = 1;
    }
    return (double)rand() / RAND_MAX;
}

// ==================== 网络函数 ====================

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

static int 网络已初始化 = 0;

void 初始化网络() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    网络已初始化 = 1;
}

void 清理网络() {
#ifdef _WIN32
    WSACleanup();
#endif
    网络已初始化 = 0;
}

// 创建TCP客户端套接字
int 创建TCP客户端() {
    if (!网络已初始化) 初始化网络();
    return (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

// 连接到服务器
int TCP连接(int 套接字, const char* 地址, int 端口) {
    struct sockaddr_in 服务器地址;
    服务器地址.sin_family = AF_INET;
    服务器地址.sin_port = htons(端口);
    inet_pton(AF_INET, 地址, &服务器地址.sin_addr);

    return connect((SOCKET)套接字, (struct sockaddr*)&服务器地址, sizeof(服务器地址));
}

// 发送数据
int TCP发送(int 套接字, const char* 数据) {
    return send((SOCKET)套接字, 数据, (int)strlen(数据), 0);
}

// 接收数据
const char* TCP接收(int 套接字) {
    static char 缓冲区[4096];
    memset(缓冲区, 0, sizeof(缓冲区));
    int 字节数 = recv((SOCKET)套接字, 缓冲区, sizeof(缓冲区) - 1, 0);
    if (字节数 <= 0) return "";
    return 缓冲区;
}

// 关闭套接字
void TCP关闭(int 套接字) {
#ifdef _WIN32
    closesocket((SOCKET)套接字);
#else
    close((SOCKET)套接字);
#endif
}

// 创建TCP服务器
int 创建TCP服务器(int 端口) {
    if (!网络已初始化) 初始化网络();

    int 服务器套接字 = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (服务器套接字 < 0) return -1;

    struct sockaddr_in 服务器地址;
    服务器地址.sin_family = AF_INET;
    服务器地址.sin_addr.s_addr = INADDR_ANY;
    服务器地址.sin_port = htons(端口);

    if (bind((SOCKET)服务器套接字, (struct sockaddr*)&服务器地址, sizeof(服务器地址)) < 0) {
        return -1;
    }

    if (listen((SOCKET)服务器套接字, 5) < 0) {
        return -1;
    }

    return 服务器套接字;
}

// 接受连接
int TCP接受连接(int 服务器套接字) {
    struct sockaddr_in 客户端地址;
    int 地址长度 = sizeof(客户端地址);
    return (int)accept((SOCKET)服务器套接字, (struct sockaddr*)&客户端地址, &地址长度);
}

// 获取客户端IP地址
const char* 获取客户端IP(int 客户端套接字) {
    struct sockaddr_in 客户端地址;
    int 地址长度 = sizeof(客户端地址);
    getpeername((SOCKET)客户端套接字, (struct sockaddr*)&客户端地址, &地址长度);
    return inet_ntoa(客户端地址.sin_addr);
}

// UDP函数
int 创建UDP套接字() {
    if (!网络已初始化) 初始化网络();
    return (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

int UDP绑定(int 套接字, int 端口) {
    struct sockaddr_in 地址;
    地址.sin_family = AF_INET;
    地址.sin_addr.s_addr = INADDR_ANY;
    地址.sin_port = htons(端口);
    return bind((SOCKET)套接字, (struct sockaddr*)&地址, sizeof(地址));
}

int UDP发送到(int 套接字, const char* 数据, const char* 地址, int 端口) {
    struct sockaddr_in 目标地址;
    目标地址.sin_family = AF_INET;
    目标地址.sin_port = htons(端口);
    inet_pton(AF_INET, 地址, &目标地址.sin_addr);
    return sendto((SOCKET)套接字, 数据, (int)strlen(数据), 0, (struct sockaddr*)&目标地址, sizeof(目标地址));
}

const char* UDP接收(int 套接字) {
    static char 缓冲区[4096];
    memset(缓冲区, 0, sizeof(缓冲区));
    struct sockaddr_in 发送者地址;
    int 地址长度 = sizeof(发送者地址);
    int 字节数 = recvfrom((SOCKET)套接字, 缓冲区, sizeof(缓冲区) - 1, 0, (struct sockaddr*)&发送者地址, &地址长度);
    if (字节数 <= 0) return "";
    return 缓冲区;
}

// ==================== 数学扩展函数 ====================

// 幂运算
double 幂运算(double 底数, double 指数) {
    return pow(底数, 指数);
}

// 绝对值（自举编译器统一按双精度处理）
double 绝对值(double 值) {
    return 值 < 0.0 ? -值 : 值;
}

// 最小值（自举编译器内建，整数）
int 内建最小值(int 甲, int 乙) {
    return 甲 < 乙 ? 甲 : 乙;
}

// 最大值（自举编译器内建，整数）
int 内建最大值(int 甲, int 乙) {
    return 甲 > 乙 ? 甲 : 乙;
}

// 查找子串（返回字节偏移，未找到为 -1）
int 查找(const char* 主串, const char* 子串) {
    const char* 位置 = strstr(主串, 子串);
    return 位置 ? (int)(位置 - 主串) : -1;
}

// 平方根
double 平方根(double 数) {
    return sqrt(数);
}

// 正弦函数
double 正弦(double 弧度) {
    return sin(弧度);
}

// 余弦函数
double 余弦(double 弧度) {
    return cos(弧度);
}

// 正切函数
double 正切(double 弧度) {
    return tan(弧度);
}

// 反正弦函数
double 反正弦(double 值) {
    return asin(值);
}

// 反余弦函数
double 反余弦(double 值) {
    return acos(值);
}

// 反正切函数
double 反正切(double 值) {
    return atan(值);
}

// 自然对数
double 自然对数(double 数) {
    return log(数);
}

// 常用对数
double 常用对数(double 数) {
    return log10(数);
}

// 角度转弧度
double 角度转弧度(double 角度) {
    return 角度 * M_PI / 180.0;
}

// 弧度转角度
double 弧度转角度(double 弧度) {
    return 弧度 * 180.0 / M_PI;
}

// ==================== 系统函数 ====================

// 获取环境变量
const char* 获取环境变量(const char* 名称) {
    const char* 值 = getenv(名称);
    return 值 ? 值 : "";
}

// 执行系统命令（命令字符串为 UTF-8；Windows 下转宽字符后用 _wsystem，
// 以免命令行里的中文路径被本地代码页曲解）
int 执行系统命令(const char* 命令) {
#ifdef _WIN32
    int 宽长 = MultiByteToWideChar(CP_UTF8, 0, 命令, -1, NULL, 0);
    if (宽长 <= 0) { return system(命令); }
    wchar_t* 宽命令 = (wchar_t*)malloc((size_t)宽长 * sizeof(wchar_t));
    if (!宽命令) { return system(命令); }
    MultiByteToWideChar(CP_UTF8, 0, 命令, -1, 宽命令, 宽长);
    int 结果 = _wsystem(宽命令);
    free(宽命令);
    return 结果;
#else
    return system(命令);
#endif
}

// 获取当前工作目录
const char* 获取当前目录() {
    static char 缓冲区[1024];
#ifdef _WIN32
    GetCurrentDirectoryA(sizeof(缓冲区), 缓冲区);
#else
    getcwd(缓冲区, sizeof(缓冲区));
#endif
    return 缓冲区;
}

// 检查文件是否存在
int 文件存在(const char* 路径) {
    FILE* 文件 = UTF8打开文件(路径, "r");
    if (文件) {
        fclose(文件);
        return 1;
    }
    return 0;
}

// 获取文件大小
long 获取文件大小(const char* 路径) {
    FILE* 文件 = UTF8打开文件(路径, "rb");
    if (!文件) return -1;
    fseek(文件, 0, SEEK_END);
    long 文件大小 = ftell(文件);
    fclose(文件);
    return 文件大小;
}

// ==================== 命令行参数 ====================

static int 全局参数数量 = 0;
static const char** 全局参数列表 = NULL;

// 将宽字符参数转换为 UTF-8 副本
static char* 宽参数转UTF8(const wchar_t* 宽参数) {
    int 长度 = WideCharToMultiByte(CP_UTF8, 0, 宽参数, -1, NULL, 0, NULL, NULL);
    char* 结果 = (char*)malloc(长度 ? 长度 : 1);
    if (结果) WideCharToMultiByte(CP_UTF8, 0, 宽参数, -1, 结果, 长度, NULL, NULL);
    return 结果;
}

void 设置参数(int argc, const char** argv) {
#ifdef _WIN32
    // 自举产物（一级.exe/二级.exe）的 main 由 IR 发射，其 argv 来自 CRT 窄字符
    // （本地代码页 GBK/ACP 字节）。此处改用宽命令行重新解析并转 UTF-8，
    // 保证与宿主（主程序.cpp 用 CommandLineToArgvW 传 UTF-8）行为一致，
    // 避免「GBK 目录 + UTF-8 模块名」拼接出混合编码路径导致找不到模块。
    (void)argc; (void)argv;
    int 宽数量 = 0;
    LPWSTR* 宽参数 = CommandLineToArgvW(GetCommandLineW(), &宽数量);
    if (宽参数 && 宽数量 > 0) {
        static char** UTF8参数 = NULL;
        static char* 参数缓冲[64];
        if (宽数量 <= 64) {
            int i;
            for (i = 0; i < 宽数量; i++) 参数缓冲[i] = 宽参数转UTF8(宽参数[i]);
            全局参数数量 = 宽数量;
            全局参数列表 = (const char**)参数缓冲;
            LocalFree(宽参数);
            return;
        }
        LocalFree(宽参数);
    }
#endif
    全局参数数量 = argc;
    全局参数列表 = argv;
}

int 获取参数数量() {
    return 全局参数数量;
}

const char* 获取参数(int 索引) {
    if (索引 >= 0 && 索引 < 全局参数数量) {
        return 全局参数列表[索引];
    }
    return "";
}

// ==================== 标准错误输出 ====================

void 输出错误(const char* 消息) {
    fprintf(stderr, "%s\n", 消息);
}

void 输出错误值(int 值) {
    fprintf(stderr, "%d\n", 值);
}

// ==================== 文件系统操作 ====================

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

int 创建目录(const char* 路径) {
#ifdef _WIN32
    // 将UTF-8路径转换为宽字符
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 路径, -1, NULL, 0);
    wchar_t* 宽路径 = (wchar_t*)malloc(宽长度 * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, 路径, -1, 宽路径, 宽长度);
    int 结果 = _wmkdir(宽路径);
    free(宽路径);
    return 结果;
#else
    return mkdir(路径, 0755);
#endif
}

int 目录存在(const char* 路径) {
#ifdef _WIN32
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 路径, -1, NULL, 0);
    wchar_t* 宽路径 = (wchar_t*)malloc(宽长度 * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, 路径, -1, 宽路径, 宽长度);
    DWORD 属性 = GetFileAttributesW(宽路径);
    free(宽路径);
    return (属性 != INVALID_FILE_ATTRIBUTES && (属性 & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    if (stat(路径, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    return 0;
#endif
}

int 删除文件(const char* 路径) {
#ifdef _WIN32
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 路径, -1, NULL, 0);
    wchar_t* 宽路径 = (wchar_t*)malloc(宽长度 * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, 路径, -1, 宽路径, 宽长度);
    int 结果 = _wremove(宽路径);
    free(宽路径);
    return 结果;
#else
    return remove(路径);
#endif
}

int 重命名文件(const char* 旧路径, const char* 新路径) {
#ifdef _WIN32
    int 旧宽长度 = MultiByteToWideChar(CP_UTF8, 0, 旧路径, -1, NULL, 0);
    wchar_t* 旧宽路径 = (wchar_t*)malloc(旧宽长度 * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, 旧路径, -1, 旧宽路径, 旧宽长度);

    int 新宽长度 = MultiByteToWideChar(CP_UTF8, 0, 新路径, -1, NULL, 0);
    wchar_t* 新宽路径 = (wchar_t*)malloc(新宽长度 * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, 新路径, -1, 新宽路径, 新宽长度);

    int 结果 = _wrename(旧宽路径, 新宽路径);
    free(旧宽路径);
    free(新宽路径);
    return 结果;
#else
    return rename(旧路径, 新路径);
#endif
}

// ==================== 目录列表操作 ====================

#ifdef _WIN32
#include <io.h>
#else
#include <dirent.h>
#endif

// 目录项结构
typedef struct {
    char 名称[256];
    int 是目录;
    long long 文件大小;
} 目录项;

// 列出目录内容，返回目录项数组
// 注意：返回的是静态数组，每次调用会覆盖
#define 最大目录项数 1024
static 目录项 目录项列表[最大目录项数];
static int 目录项数量 = 0;

int 列出目录(const char* 路径) {
    目录项数量 = 0;
    
#ifdef _WIN32
    // Windows: 使用 FindFirstFile/FindNextFile
    char 搜索路径[512];
    snprintf(搜索路径, sizeof(搜索路径), "%s\\*", 路径);
    
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 搜索路径, -1, NULL, 0);
    wchar_t* 宽路径 = (wchar_t*)malloc(宽长度 * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, 搜索路径, -1, 宽路径, 宽长度);
    
    WIN32_FIND_DATAW 查找数据;
    HANDLE 句柄 = FindFirstFileW(宽路径, &查找数据);
    free(宽路径);
    
    if (句柄 == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    do {
        // 跳过 . 和 ..
        if (wcscmp(查找数据.cFileName, L".") == 0 || wcscmp(查找数据.cFileName, L"..") == 0) {
            continue;
        }
        
        if (目录项数量 >= 最大目录项数) break;
        
        // 转换文件名为UTF-8
        int 名称长度 = WideCharToMultiByte(CP_UTF8, 0, 查找数据.cFileName, -1, NULL, 0, NULL, NULL);
        WideCharToMultiByte(CP_UTF8, 0, 查找数据.cFileName, -1, 目录项列表[目录项数量].名称, 名称长度, NULL, NULL);
        
        目录项列表[目录项数量].是目录 = (查找数据.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
        目录项列表[目录项数量].文件大小 = ((long long)查找数据.nFileSizeHigh << 32) | 查找数据.nFileSizeLow;
        
        目录项数量++;
    } while (FindNextFileW(句柄, &查找数据));
    
    FindClose(句柄);
#else
    // Linux/macOS: 使用 opendir/readdir
    DIR* 目录 = opendir(路径);
    if (!目录) {
        return 0;
    }
    
    struct dirent* 入口;
    while ((入口 = readdir(目录)) != NULL) {
        // 跳过 . 和 ..
        if (strcmp(入口->d_name, ".") == 0 || strcmp(入口->d_name, "..") == 0) {
            continue;
        }
        
        if (目录项数量 >= 最大目录项数) break;
        
        strncpy(目录项列表[目录项数量].名称, 入口->d_name, 255);
        目录项列表[目录项数量].名称[255] = '\0';
        目录项列表[目录项数量].是目录 = (入口->d_type == DT_DIR) ? 1 : 0;
        目录项列表[目录项数量].文件大小 = 0;
        
        目录项数量++;
    }
    
    closedir(目录);
#endif
    
    return 目录项数量;
}

// 获取目录项名称
const char* 获取目录项名称(int 索引) {
    if (索引 >= 0 && 索引 < 目录项数量) {
        return 目录项列表[索引].名称;
    }
    return "";
}

// 获取目录项是否为目录
int 获取目录项是否目录(int 索引) {
    if (索引 >= 0 && 索引 < 目录项数量) {
        return 目录项列表[索引].是目录;
    }
    return 0;
}

// 获取目录项大小
long 获取目录项大小(int 索引) {
    if (索引 >= 0 && 索引 < 目录项数量) {
        return 目录项列表[索引].文件大小;
    }
    return 0;
}

// 递归遍历目录，收集所有文件到目录项列表
int 递归遍历目录(const char* 路径, int 深度) {
    int 开始索引 = 目录项数量;
    
#ifdef _WIN32
    char 搜索路径[512];
    snprintf(搜索路径, sizeof(搜索路径), "%s\\*", 路径);
    
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 搜索路径, -1, NULL, 0);
    wchar_t* 宽路径 = (wchar_t*)malloc(宽长度 * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, 搜索路径, -1, 宽路径, 宽长度);
    
    WIN32_FIND_DATAW 查找数据;
    HANDLE 句柄 = FindFirstFileW(宽路径, &查找数据);
    free(宽路径);
    
    if (句柄 == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    do {
        if (wcscmp(查找数据.cFileName, L".") == 0 || wcscmp(查找数据.cFileName, L"..") == 0) {
            continue;
        }
        
        if (目录项数量 >= 最大目录项数) break;
        
        int 名称长度 = WideCharToMultiByte(CP_UTF8, 0, 查找数据.cFileName, -1, NULL, 0, NULL, NULL);
        WideCharToMultiByte(CP_UTF8, 0, 查找数据.cFileName, -1, 目录项列表[目录项数量].名称, 名称长度, NULL, NULL);
        
        目录项列表[目录项数量].是目录 = (查找数据.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
        目录项列表[目录项数量].文件大小 = ((long long)查找数据.nFileSizeHigh << 32) | 查找数据.nFileSizeLow;
        
        目录项数量++;
        
        // 如果是目录，递归遍历
        if ((查找数据.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 深度 > 0) {
            char 子路径[512];
            snprintf(子路径, sizeof(子路径), "%s/%s", 路径, 目录项列表[目录项数量 - 1].名称);
            递归遍历目录(子路径, 深度 - 1);
        }
    } while (FindNextFileW(句柄, &查找数据));
    
    FindClose(句柄);
#else
    DIR* 目录 = opendir(路径);
    if (!目录) return 0;
    
    struct dirent* 入口;
    while ((入口 = readdir(目录)) != NULL) {
        if (strcmp(入口->d_name, ".") == 0 || strcmp(入口->d_name, "..") == 0) continue;
        if (目录项数量 >= 最大目录项数) break;
        
        strncpy(目录项列表[目录项数量].名称, 入口->d_name, 255);
        目录项列表[目录项数量].名称[255] = '\0';
        目录项列表[目录项数量].是目录 = (入口->d_type == DT_DIR) ? 1 : 0;
        目录项列表[目录项数量].文件大小 = 0;
        目录项数量++;
        
        if (入口->d_type == DT_DIR && 深度 > 0) {
            char 子路径[512];
            snprintf(子路径, sizeof(子路径), "%s/%s", 路径, 入口->d_name);
            递归遍历目录(子路径, 深度 - 1);
        }
    }
    closedir(目录);
#endif
    
    return 目录项数量 - 开始索引;
}

// 删除目录（递归）
int 删除目录(const char* 路径) {
#ifdef _WIN32
    int 宽长度 = MultiByteToWideChar(CP_UTF8, 0, 路径, -1, NULL, 0);
    wchar_t* 宽路径 = (wchar_t*)malloc(宽长度 * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, 路径, -1, 宽路径, 宽长度);
    
    // 使用 SHFileOperation 递归删除
    SHFILEOPSTRUCTW 文件操作;
    memset(&文件操作, 0, sizeof(文件操作));
    文件操作.wFunc = FO_DELETE;
    文件操作.pFrom = 宽路径;
    文件操作.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION;
    
    int 结果 = SHFileOperationW(&文件操作);
    free(宽路径);
    return 结果;
#else
    // 递归删除目录
    DIR* 目录 = opendir(路径);
    if (!目录) return -1;
    
    struct dirent* 入口;
    char 子路径[512];
    
    while ((入口 = readdir(目录)) != NULL) {
        if (strcmp(入口->d_name, ".") == 0 || strcmp(入口->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(子路径, sizeof(子路径), "%s/%s", 路径, 入口->d_name);
        
        struct stat st;
        if (stat(子路径, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                删除目录(子路径);
            } else {
                remove(子路径);
            }
        }
    }
    
    closedir(目录);
    return rmdir(路径);
#endif
}

// 复制文件
int 复制文件(const char* 源路径, const char* 目标路径) {
    FILE* 源文件 = UTF8打开文件(源路径, "rb");
    if (!源文件) return -1;
    
    FILE* 目标文件 = UTF8打开文件(目标路径, "wb");
    if (!目标文件) {
        fclose(源文件);
        return -1;
    }
    
    char 缓冲区[4096];
    size_t 读取大小;
    
    while ((读取大小 = fread(缓冲区, 1, sizeof(缓冲区), 源文件)) > 0) {
        fwrite(缓冲区, 1, 读取大小, 目标文件);
    }
    
    fclose(源文件);
    fclose(目标文件);
    return 0;
}

// 获取文件扩展名
const char* 获取文件扩展名(const char* 路径) {
    const char* 点位置 = strrchr(路径, '.');
    if (点位置) {
        return 点位置 + 1;
    }
    return "";
}

// 获取文件名（不含路径）
const char* 获取文件名(const char* 路径) {
    const char* 分隔符1 = strrchr(路径, '/');
    const char* 分隔符2 = strrchr(路径, '\\');
    const char* 分隔符 = 分隔符1 > 分隔符2 ? 分隔符1 : 分隔符2;
    if (分隔符) {
        return 分隔符 + 1;
    }
    return 路径;
}

// 获取目录路径（不含文件名）
const char* 获取目录路径(const char* 路径) {
    static char 目录缓冲区[512];
    strncpy(目录缓冲区, 路径, sizeof(目录缓冲区) - 1);
    目录缓冲区[sizeof(目录缓冲区) - 1] = '\0';
    
    char* 分隔符1 = strrchr(目录缓冲区, '/');
    char* 分隔符2 = strrchr(目录缓冲区, '\\');
    char* 分隔符 = 分隔符1 > 分隔符2 ? 分隔符1 : 分隔符2;
    
    if (分隔符) {
        *分隔符 = '\0';
    } else {
        strcpy(目录缓冲区, ".");
    }
    
    return 目录缓冲区;
}

// 连接路径
const char* 连接路径(const char* 路径1, const char* 路径2) {
    static char 连接缓冲区[512];
    snprintf(连接缓冲区, sizeof(连接缓冲区), "%s/%s", 路径1, 路径2);
    return 连接缓冲区;
}

// ==================== 字符串流操作 ====================

// 将字符串按行分割，返回行数
int 分割行数(const char* 文本) {
    int 行数 = 0;
    const char* p = 文本;
    while (*p) {
        if (*p == '\n') 行数++;
        p++;
    }
    if (p > 文本 && *(p-1) != '\n') 行数++;  // 最后一行没有换行
    return 行数;
}

// 获取指定行的内容（从0开始）
const char* 获取行(const char* 文本, int 行号) {
    static char 缓冲区[4096];
    缓冲区[0] = '\0';

    int 当前行 = 0;
    const char* 开始 = 文本;
    const char* p = 文本;

    while (*p) {
        if (*p == '\n') {
            if (当前行 == 行号) {
                int 长度 = (int)(p - 开始);
                if (长度 >= 4096) 长度 = 4095;
                strncpy(缓冲区, 开始, 长度);
                缓冲区[长度] = '\0';
                return 缓冲区;
            }
            当前行++;
            开始 = p + 1;
        }
        p++;
    }

    // 最后一行
    if (当前行 == 行号 && p > 开始) {
        int 长度 = (int)(p - 开始);
        if (长度 >= 4096) 长度 = 4095;
        strncpy(缓冲区, 开始, 长度);
        缓冲区[长度] = '\0';
        return 缓冲区;
    }

    return "";
}

// 字符串分割（按分隔符）
const char** 按分隔符分割(const char* 文本, const char* 分隔符, int* 结果数量) {
    static const char* 结果[256];
    *结果数量 = 0;

    char* 复制 = strdup(文本);
    char* token = strtok(复制, 分隔符);
    while (token != NULL && *结果数量 < 256) {
        结果[*结果数量] = strdup(token);
        (*结果数量)++;
        token = strtok(NULL, 分隔符);
    }
    free(复制);
    return 结果;
}

// 去除字符串首尾空白
const char* 去除空白(const char* 文本) {
    static char 缓冲区[4096];
    int 长度 = (int)strlen(文本);
    if (长度 >= 4096) 长度 = 4095;

    // 跳过开头空白
    int 开始 = 0;
    while (开始 < 长度 && (文本[开始] == ' ' || 文本[开始] == '\t' ||
           文本[开始] == '\n' || 文本[开始] == '\r')) {
        开始++;
    }

    // 跳过结尾空白
    int 结束 = 长度 - 1;
    while (结束 >= 开始 && (文本[结束] == ' ' || 文本[结束] == '\t' ||
           文本[结束] == '\n' || 文本[结束] == '\r')) {
        结束--;
    }

    int 新长度 = 结束 - 开始 + 1;
    if (新长度 <= 0) {
        缓冲区[0] = '\0';
        return 缓冲区;
    }

    strncpy(缓冲区, 文本 + 开始, 新长度);
    缓冲区[新长度] = '\0';
    return 缓冲区;
}

// 字符串转小写
const char* 文本转小写(const char* 文本) {
    static char 缓冲区[4096];
    int i = 0;
    while (文本[i] && i < 4095) {
        if (文本[i] >= 'A' && 文本[i] <= 'Z') {
            缓冲区[i] = 文本[i] + 32;
        } else {
            缓冲区[i] = 文本[i];
        }
        i++;
    }
    缓冲区[i] = '\0';
    return 缓冲区;
}

// 字符串转大写
const char* 文本转大写(const char* 文本) {
    static char 缓冲区[4096];
    int i = 0;
    while (文本[i] && i < 4095) {
        if (文本[i] >= 'a' && 文本[i] <= 'z') {
            缓冲区[i] = 文本[i] - 32;
        } else {
            缓冲区[i] = 文本[i];
        }
        i++;
    }
    缓冲区[i] = '\0';
    return 缓冲区;
}

// 检查字符串是否以指定前缀开头
int 字符串开头(const char* 文本, const char* 前缀) {
    return strncmp(文本, 前缀, strlen(前缀)) == 0;
}

// 检查字符串是否以指定后缀结尾
int 字符串结尾(const char* 文本, const char* 后缀) {
    int 文本长度 = (int)strlen(文本);
    int 后缀长度 = (int)strlen(后缀);
    if (后缀长度 > 文本长度) return 0;
    return strcmp(文本 + 文本长度 - 后缀长度, 后缀) == 0;
}

// ==================== 字符操作 ====================

// 获取字符串指定位置字符的码点
int 字符码(const char* 文本, int 位置) {
    int 字节位置 = 0;
    int 当前字符 = 0;

    while (文本[字节位置] != '\0' && 当前字符 < 位置) {
        unsigned char c = (unsigned char)文本[字节位置];
        if (c < 0x80) 字节位置 += 1;
        else if ((c & 0xE0) == 0xC0) 字节位置 += 2;
        else if ((c & 0xF0) == 0xE0) 字节位置 += 3;
        else if ((c & 0xF8) == 0xF0) 字节位置 += 4;
        else 字节位置 += 1;
        当前字符++;
    }

    if (文本[字节位置] == '\0') return -1;

    unsigned char c = (unsigned char)文本[字节位置];
    if (c < 0x80) return c;
    if ((c & 0xE0) == 0xC0) return ((c & 0x1F) << 6) | (文本[字节位置+1] & 0x3F);
    if ((c & 0xF0) == 0xE0) return ((c & 0x0F) << 12) | ((文本[字节位置+1] & 0x3F) << 6) | (文本[字节位置+2] & 0x3F);
    if ((c & 0xF8) == 0xF0) return ((c & 0x07) << 18) | ((文本[字节位置+1] & 0x3F) << 12) | ((文本[字节位置+2] & 0x3F) << 6) | (文本[字节位置+3] & 0x3F);
    return c;
}

// 从码点创建UTF-8字符串
const char* 字符序列(int 码点) {
    static char 缓冲区[5];
    if (码点 < 0x80) {
        缓冲区[0] = (char)码点;
        缓冲区[1] = '\0';
    } else if (码点 < 0x800) {
        缓冲区[0] = (char)(0xC0 | (码点 >> 6));
        缓冲区[1] = (char)(0x80 | (码点 & 0x3F));
        缓冲区[2] = '\0';
    } else if (码点 < 0x10000) {
        缓冲区[0] = (char)(0xE0 | (码点 >> 12));
        缓冲区[1] = (char)(0x80 | ((码点 >> 6) & 0x3F));
        缓冲区[2] = (char)(0x80 | (码点 & 0x3F));
        缓冲区[3] = '\0';
    } else {
        缓冲区[0] = (char)(0xF0 | (码点 >> 18));
        缓冲区[1] = (char)(0x80 | ((码点 >> 12) & 0x3F));
        缓冲区[2] = (char)(0x80 | ((码点 >> 6) & 0x3F));
        缓冲区[3] = (char)(0x80 | (码点 & 0x3F));
        缓冲区[4] = '\0';
    }
    return 缓冲区;
}

// ==================== 动态数组 ====================

#define 动态数组初始大小 16

typedef struct {
    int* 数据;
    int 数组大小;
    int 容量;
} 动态数组;

动态数组* 创建动态数组() {
    动态数组* arr = (动态数组*)malloc(sizeof(动态数组));
    arr->数据 = (int*)malloc(动态数组初始大小 * sizeof(int));
    arr->数组大小 = 0;
    arr->容量 = 动态数组初始大小;
    return arr;
}

void 动态数组添加(动态数组* arr, int 值) {
    if (arr->数组大小 >= arr->容量) {
        arr->容量 *= 2;
        arr->数据 = (int*)realloc(arr->数据, arr->容量 * sizeof(int));
    }
    arr->数据[arr->数组大小++] = 值;
}

int 动态数组获取(动态数组* arr, int 索引) {
    if (索引 >= 0 && 索引 < arr->数组大小) {
        return arr->数据[索引];
    }
    return 0;
}

void 动态数组设置(动态数组* arr, int 索引, int 值) {
    if (索引 >= 0 && 索引 < arr->数组大小) {
        arr->数据[索引] = 值;
    }
}

int 动态数组大小(动态数组* arr) {
    return arr->数组大小;
}

void 动态数组删除(动态数组* arr, int 索引) {
    if (索引 >= 0 && 索引 < arr->数组大小) {
        for (int i = 索引; i < arr->数组大小 - 1; i++) {
            arr->数据[i] = arr->数据[i + 1];
        }
        arr->数组大小--;
    }
}

void 动态数组插入(动态数组* arr, int 索引, int 值) {
    if (索引 < 0 || 索引 > arr->数组大小) return;
    动态数组添加(arr, 0);  // 扩展
    for (int i = arr->数组大小 - 1; i > 索引; i--) {
        arr->数据[i] = arr->数据[i - 1];
    }
    arr->数据[索引] = 值;
}

void 释放动态数组(动态数组* arr) {
    free(arr->数据);
    free(arr);
}

// 动态字符串数组
typedef struct {
    const char** 数据;
    int 数组大小;
    int 容量;
} 动态字符串数组;

动态字符串数组* 创建动态字符串数组() {
    动态字符串数组* arr = (动态字符串数组*)malloc(sizeof(动态字符串数组));
    arr->数据 = (const char**)malloc(动态数组初始大小 * sizeof(const char*));
    arr->数组大小 = 0;
    arr->容量 = 动态数组初始大小;
    return arr;
}

void 动态字符串数组添加(动态字符串数组* arr, const char* 值) {
    if (arr->数组大小 >= arr->容量) {
        arr->容量 *= 2;
        arr->数据 = (const char**)realloc(arr->数据, arr->容量 * sizeof(const char*));
    }
    arr->数据[arr->数组大小++] = strdup(值);
}

const char* 动态字符串数组获取(动态字符串数组* arr, int 索引) {
    if (索引 >= 0 && 索引 < arr->数组大小) {
        return arr->数据[索引];
    }
    return "";
}

void 动态字符串数组设置(动态字符串数组* arr, int 索引, const char* 值) {
    if (索引 >= 0 && 索引 < arr->数组大小) {
        free((void*)arr->数据[索引]);
        arr->数据[索引] = strdup(值);
    }
}

void 动态字符串数组删除(动态字符串数组* arr, int 索引) {
    if (索引 >= 0 && 索引 < arr->数组大小) {
        free((void*)arr->数据[索引]);
        for (int i = 索引; i < arr->数组大小 - 1; i++) {
            arr->数据[i] = arr->数据[i + 1];
        }
        arr->数组大小--;
    }
}

void 动态字符串数组插入(动态字符串数组* arr, int 索引, const char* 值) {
    if (索引 >= 0 && 索引 <= arr->数组大小) {
        if (arr->数组大小 >= arr->容量) {
            arr->容量 *= 2;
            arr->数据 = (const char**)realloc(arr->数据, arr->容量 * sizeof(const char*));
        }
        for (int i = arr->数组大小; i > 索引; i--) {
            arr->数据[i] = arr->数据[i - 1];
        }
        arr->数据[索引] = strdup(值);
        arr->数组大小++;
    }
}

int 动态字符串数组大小(动态字符串数组* arr) {
    return arr->数组大小;
}

void 释放动态字符串数组(动态字符串数组* arr) {
    for (int i = 0; i < arr->数组大小; i++) {
        free((void*)arr->数据[i]);
    }
    free(arr->数据);
    free(arr);
}

// 通道实现
typedef struct {
    int* 数据;
    int 数组大小;
    int 容量;
    int 头;
    int 尾;
    int 计数;
    int 已关闭;
} 通道数据;

通道数据* 创建通道函数(int 容量) {
    通道数据* ch = (通道数据*)malloc(sizeof(通道数据));
    ch->容量 = 容量 > 0 ? 容量 : 1;
    ch->数据 = (int*)malloc(sizeof(int) * ch->容量);
    ch->数组大小 = ch->容量;
    ch->头 = 0;
    ch->尾 = 0;
    ch->计数 = 0;
    ch->已关闭 = 0;
    return ch;
}

// 返回: 0=成功, -1=通道已关闭, -2=通道已满
int 发送到通道函数(通道数据* ch, int 值) {
    if (ch->已关闭) return -1;
    if (ch->计数 >= ch->容量) return -2;
    ch->数据[ch->尾] = 值;
    ch->尾 = (ch->尾 + 1) % ch->容量;
    ch->计数++;
    return 0;
}

// 返回: 接收到的值, 如果通道已关闭且为空返回0
int 从通道接收函数(通道数据* ch) {
    if (ch->计数 <= 0) return 0;
    int 值 = ch->数据[ch->头];
    ch->头 = (ch->头 + 1) % ch->容量;
    ch->计数--;
    return 值;
}

// 返回: 通道是否为空
int 通道是否为空(通道数据* ch) {
    return ch->计数 <= 0;
}

// 返回: 通道是否已关闭
int 通道是否已关闭(通道数据* ch) {
    return ch->已关闭;
}

void 关闭通道函数(通道数据* ch) {
    ch->已关闭 = 1;
}

void 释放通道函数(通道数据* ch) {
    free(ch->数据);
    free(ch);
}

// 调试器支持
static int 调试器已启用 = 0;
static int 调试器单步模式 = 0;

void 设置调试器(int 启用) {
    调试器已启用 = 启用;
}

void 设置单步模式(int 启用) {
    调试器单步模式 = 启用;
}

int 是调试器启用() {
    return 调试器已启用;
}

int 是单步模式() {
    return 调试器单步模式;
}

void 断点命中(const char* 文件, int 行号, const char* 函数名) {
    if (!调试器已启用) return;
    printf("[调试器] 断点: %s (行 %d, 函数 %s)\n", 文件, 行号, 函数名);
    调试器单步模式 = 1;
}

// 测试覆盖率支持
#define 最大行数 10000
static int 覆盖率已启用 = 0;
static int 已执行行数[最大行数];
static int 总行数 = 0;
static const char* 覆盖率文件名 = NULL;

void 设置覆盖率(int 启用) {
    覆盖率已启用 = 启用;
    if (启用) {
        memset(已执行行数, 0, sizeof(已执行行数));
    }
}

void 记录行执行(int 行号) {
    if (!覆盖率已启用) return;
    if (行号 > 0 && 行号 < 最大行数) {
        已执行行数[行号] = 1;
        if (行号 > 总行数) 总行数 = 行号;
    }
}

void 设置覆盖率文件(const char* 文件名) {
    覆盖率文件名 = 文件名;
}

void 输出覆盖率报告() {
    if (!覆盖率已启用) return;
    int 已覆盖 = 0;
    for (int i = 1; i <= 总行数; i++) {
        if (已执行行数[i]) 已覆盖++;
    }
    printf("\n=== 覆盖率报告 ===\n");
    printf("文件: %s\n", 覆盖率文件名 ? 覆盖率文件名 : "未知");
    printf("总行数: %d\n", 总行数);
    printf("已覆盖: %d\n", 已覆盖);
    if (总行数 > 0) {
        printf("覆盖率: %.1f%%\n", 100.0 * 已覆盖 / 总行数);
    }
    printf("\n未覆盖的行:\n");
    int 未覆盖数 = 0;
    for (int i = 1; i <= 总行数; i++) {
        if (!已执行行数[i]) {
            printf("  行 %d\n", i);
            未覆盖数++;
            if (未覆盖数 >= 20) {
                printf("  ... (还有更多)\n");
                break;
            }
        }
    }
    printf("==================\n");
}

// RTTI 支持 — 异常类型注册表
#define 最大异常类型 256
static void* 异常指针表[最大异常类型];
static int 异常类型表[最大异常类型];
static int 异常类型数量 = 0;

void 设置异常类型(void* 指针, int 类型) {
    for (int i = 0; i < 异常类型数量; i++) {
        if (异常指针表[i] == 指针) {
            异常类型表[i] = 类型;
            return;
        }
    }
    if (异常类型数量 < 最大异常类型) {
        异常指针表[异常类型数量] = 指针;
        异常类型表[异常类型数量] = 类型;
        异常类型数量++;
    }
}

int 获取异常类型(void* 指针) {
    for (int i = 0; i < 异常类型数量; i++) {
        if (异常指针表[i] == 指针) {
            return 异常类型表[i];
        }
    }
    return 0;  // 默认为整数类型
}

void 清除异常类型(void* 指针) {
    for (int i = 0; i < 异常类型数量; i++) {
        if (异常指针表[i] == 指针) {
            异常指针表[i] = 异常指针表[异常类型数量 - 1];
            异常类型表[i] = 异常类型表[异常类型数量 - 1];
            异常类型数量--;
            return;
        }
    }
}

// HTTP GET 支持（简化版，使用系统命令）
static char http缓冲区[65536];

const char* HTTP获取(const char* url) {
    char 命令[1024];
#ifdef _WIN32
    snprintf(命令, sizeof(命令), "curl -s \"%s\" 2>/dev/null", url);
#else
    snprintf(命令, sizeof(命令), "curl -s \"%s\" 2>/dev/null", url);
#endif
    FILE* fp = popen(命令, "r");
    if (!fp) {
        http缓冲区[0] = '\0';
        return http缓冲区;
    }
    size_t 总读取 = 0;
    while (总读取 < sizeof(http缓冲区) - 1) {
        size_t 读取 = fread(http缓冲区 + 总读取, 1, sizeof(http缓冲区) - 1 - 总读取, fp);
        if (读取 == 0) break;
        总读取 += 读取;
    }
    http缓冲区[总读取] = '\0';
    pclose(fp);
    return http缓冲区;
}

// 文件写入
int 写入文件(const char* 路径, const char* 内容) {
    FILE* fp = UTF8打开文件(路径, "w");
    if (!fp) return -1;
    fwrite(内容, 1, strlen(内容), fp);
    fclose(fp);
    return 0;
}

// 读取整个文件（返回 malloc 的缓冲区，调用方拥有；失败返回 NULL）
static char* 读取整文件(const char* 路径) {
    FILE* fp = UTF8打开文件(路径, "r");
    size_t 容量 = 65536;
    size_t 长度 = 0;
    char* 缓冲;
    if (!fp) return NULL;
    缓冲 = (char*)malloc(容量);
    if (!缓冲) { fclose(fp); return NULL; }
    for (;;) {
        if (长度 + 1 >= 容量) {
            char* 新缓冲;
            容量 = 容量 * 2;
            新缓冲 = (char*)realloc(缓冲, 容量);
            if (!新缓冲) { free(缓冲); fclose(fp); return NULL; }
            缓冲 = 新缓冲;
        }
        size_t n = fread(缓冲 + 长度, 1, 容量 - 1 - 长度, fp);
        长度 += n;
        if (n == 0) break;
    }
    缓冲[长度] = '\0';
    fclose(fp);
    return 缓冲;
}

// 文件读取（静态缓冲，调用方无需释放）
const char* 读取文件(const char* 路径) {
    static char* 缓冲 = NULL;
    char* 临时 = 读取整文件(路径);
    if (!临时) return "";
    free(缓冲);
    缓冲 = 临时;
    return 缓冲;
}

// 异步IO支持
// 真异步IO：基于线程的异步执行
#ifdef _WIN32
#include <process.h>
typedef HANDLE 线程句柄;
#else
#include <pthread.h>
typedef pthread_t 线程句柄;
#endif

typedef struct {
    char* 路径;
    char* 内容;
    int 模式;  // 0=读取, 1=写入
    volatile int 完成;
    int 结果;
    char* 读取结果;
    线程句柄 线程;
} IO任务;

#define 最大IO任务 64
static IO任务 IO任务表[最大IO任务];
static int IO任务数量 = 0;

#ifdef _WIN32
static unsigned __stdcall 异步写入线程(void* arg) {
    IO任务* 任务 = (IO任务*)arg;
    FILE* fp = UTF8打开文件(任务->路径, "w");
    if (fp) {
        fwrite(任务->内容, 1, strlen(任务->内容), fp);
        fclose(fp);
        任务->结果 = 0;
    } else {
        任务->结果 = -1;
    }
    任务->完成 = 1;
    return 0;
}

static unsigned __stdcall 异步读取线程(void* arg) {
    IO任务* 任务 = (IO任务*)arg;
    char* 读取缓冲区 = 读取整文件(任务->路径);
    if (读取缓冲区) {
        任务->读取结果 = 读取缓冲区;
        任务->结果 = 0;
    } else {
        任务->读取结果 = NULL;
        任务->结果 = -1;
    }
    任务->完成 = 1;
    return 0;
}
#else
static void* 异步写入线程(void* arg) {
    IO任务* 任务 = (IO任务*)arg;
    FILE* fp = UTF8打开文件(任务->路径, "w");
    if (fp) {
        fwrite(任务->内容, 1, strlen(任务->内容), fp);
        fclose(fp);
        任务->结果 = 0;
    } else {
        任务->结果 = -1;
    }
    任务->完成 = 1;
    return NULL;
}

static void* 异步读取线程(void* arg) {
    IO任务* 任务 = (IO任务*)arg;
    char* 读取缓冲区 = 读取整文件(任务->路径);
    if (读取缓冲区) {
        任务->读取结果 = 读取缓冲区;
        任务->结果 = 0;
    } else {
        任务->读取结果 = NULL;
        任务->结果 = -1;
    }
    任务->完成 = 1;
    return NULL;
}
#endif

int 异步写入文件(const char* 路径, const char* 内容) {
    if (IO任务数量 >= 最大IO任务) return -1;
    IO任务* 任务 = &IO任务表[IO任务数量++];
    任务->路径 = strdup(路径);
    任务->内容 = strdup(内容);
    任务->模式 = 1;
    任务->完成 = 0;
    任务->结果 = 0;
    任务->读取结果 = NULL;
#ifdef _WIN32
    任务->线程 = (HANDLE)_beginthreadex(NULL, 0, 异步写入线程, 任务, 0, NULL);
#else
    pthread_create(&任务->线程, NULL, 异步写入线程, 任务);
#endif
    return IO任务数量 - 1;
}

int 异步读取文件(const char* 路径) {
    if (IO任务数量 >= 最大IO任务) return -1;
    IO任务* 任务 = &IO任务表[IO任务数量++];
    任务->路径 = strdup(路径);
    任务->内容 = NULL;
    任务->模式 = 0;
    任务->完成 = 0;
    任务->结果 = 0;
    任务->读取结果 = NULL;
#ifdef _WIN32
    任务->线程 = (HANDLE)_beginthreadex(NULL, 0, 异步读取线程, 任务, 0, NULL);
#else
    pthread_create(&任务->线程, NULL, 异步读取线程, 任务);
#endif
    return IO任务数量 - 1;
}

int IO是否完成(int 任务ID) {
    if (任务ID >= 0 && 任务ID < IO任务数量) {
        return IO任务表[任务ID].完成;
    }
    return -1;
}

int IO获取结果(int 任务ID) {
    if (任务ID >= 0 && 任务ID < IO任务数量) {
        return IO任务表[任务ID].结果;
    }
    return -1;
}

const char* IO获取读取结果(int 任务ID) {
    if (任务ID >= 0 && 任务ID < IO任务数量) {
        return IO任务表[任务ID].读取结果 ? IO任务表[任务ID].读取结果 : "";
    }
    return "";
}

// 协程异步IO集成：等待IO完成并返回结果
const char* 异步IO等待(int 任务ID) {
    if (任务ID >= 0 && 任务ID < IO任务数量) {
        IO任务* 任务 = &IO任务表[任务ID];
        // 等待线程完成
#ifdef _WIN32
        if (任务->线程) WaitForSingleObject(任务->线程, INFINITE);
#else
        if (任务->线程) pthread_join(任务->线程, NULL);
#endif
        return 任务->读取结果 ? 任务->读取结果 : "";
    }
    return "";
}

int 等待IO完成(int 任务ID) {
    if (任务ID >= 0 && 任务ID < IO任务数量) {
        IO任务* 任务 = &IO任务表[任务ID];
#ifdef _WIN32
        if (任务->线程) WaitForSingleObject(任务->线程, INFINITE);
#else
        if (任务->线程) pthread_join(任务->线程, NULL);
#endif
        return 任务->完成 ? 1 : 0;
    }
    return 0;
}
#define 最大GC对象 1024
static void* GC对象表[最大GC对象];
static int GC对象数量 = 0;
static int GC标记[最大GC对象];
static int GC启用 = 0;
static int GC阈值 = 100;  // 每分配N个对象触发一次GC

void 执行GC(void);  // 前向声明

void 注册GC对象(void* 对象) {
    if (GC对象数量 < 最大GC对象) {
        GC对象表[GC对象数量] = 对象;
        GC标记[GC对象数量] = 0;
        GC对象数量++;
        if (GC启用 && GC对象数量 >= GC阈值) {
            执行GC();
        }
    }
}

void 标记对象(void* 对象) {
    for (int i = 0; i < GC对象数量; i++) {
        if (GC对象表[i] == 对象) {
            GC标记[i] = 1;
            return;
        }
    }
}

void 清除所有标记() {
    for (int i = 0; i < GC对象数量; i++) {
        GC标记[i] = 0;
    }
}

void 执行GC() {
    清除所有标记();
    // 标记阶段：保守扫描栈寻找可能的指针值
    void* 栈顶 = __builtin_frame_address(0);
    // 扫描栈帧，检查每个可能的指针值
    volatile void** 栈指针 = (volatile void**)栈顶;
    for (int i = 0; i < 4096; i++) {
        void* 候选指针 = *栈指针;
        // 检查候选指针是否匹配任何已注册对象
        for (int j = 0; j < GC对象数量; j++) {
            if (GC对象表[j] == 候选指针) {
                GC标记[j] = 1;
                break;
            }
        }
        栈指针++;
    }

    // 清除阶段：释放未标记的对象
    int 清除数量 = 0;
    for (int i = GC对象数量 - 1; i >= 0; i--) {
        if (!GC标记[i]) {
            free(GC对象表[i]);
            GC对象表[i] = GC对象表[GC对象数量 - 1];
            GC标记[i] = GC标记[GC对象数量 - 1];
            GC对象数量--;
            清除数量++;
        }
    }
    if (清除数量 > 0) {
        printf("[GC] 清除了 %d 个对象, 剩余 %d\n", 清除数量, GC对象数量);
    }
}

void 启用GC() {
    GC启用 = 1;
}

void 设置GC阈值(int 阈值) {
    GC阈值 = 阈值;
}

int 获取GC对象数量() {
    return GC对象数量;
}

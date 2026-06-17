// 日月运行时辅助函数
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

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
// 键数组和值数组是平行的，大小固定为256

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
    if (size < 256) {
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
char* 字符串连接(const char** 字符串数组, int 数量, const char* 分隔符) {
    static char 缓冲区[4096];
    缓冲区[0] = '\0';
    for (int i = 0; i < 数量; i++) {
        if (i > 0) strcat(缓冲区, 分隔符);
        strcat(缓冲区, 字符串数组[i]);
    }
    return 缓冲区;
}

// 字符串格式化（简单版本，支持%d）
char* 字符串格式化(const char* 模板, int 参数1, int 参数2, int 参数3) {
    static char 缓冲区[4096];
    缓冲区[0] = '\0';
    int 参数索引 = 0;
    int 参数值[] = {参数1, 参数2, 参数3};
    int i = 0;
    int j = 0;
    while (模板[i] != '\0' && j < 4095) {
        if (模板[i] == '%' && 模板[i + 1] == 'd' && 参数索引 < 3) {
            char 数字[32];
            sprintf(数字, "%d", 参数值[参数索引]);
            strcat(缓冲区, 数字);
            j += strlen(数字);
            i += 2;
            参数索引++;
        } else {
            缓冲区[j++] = 模板[i++];
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
#include <windows.h>
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

// 执行系统命令
int 执行系统命令(const char* 命令) {
    return system(命令);
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
    FILE* 文件 = fopen(路径, "r");
    if (文件) {
        fclose(文件);
        return 1;
    }
    return 0;
}

// 获取文件大小
long 获取文件大小(const char* 路径) {
    FILE* 文件 = fopen(路径, "rb");
    if (!文件) return -1;
    fseek(文件, 0, SEEK_END);
    long 大小 = ftell(文件);
    fclose(文件);
    return 大小;
}

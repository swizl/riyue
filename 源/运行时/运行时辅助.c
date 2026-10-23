// 日月运行时辅助函数
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#include <process.h>
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

// T5：关键字 / 文本标志 的权威 token id 表与完美哈希（宿主与自举共用同一实现）
#include "../共享/标记标识.h"
#include "../共享/标记标识表.h"

// FNV-1a 完美哈希：按生成器选定的偏移/质数/模数取桶，桶内单次比较。
static int 完美哈希查表(const char* 文本,
                        const char* const* 表, const signed char* 桶, int 桶长,
                        unsigned int 偏移, unsigned int 质数, int id起) {
    unsigned int h = 偏移;
    const unsigned char* p = (const unsigned char*)文本;
    while (*p) { h ^= (unsigned int)*p++; h *= 质数; }
    int 位 = (int)(h % (unsigned int)桶长);
    int 项 = (int)桶[位];
    if (项 >= 0 && strcmp(文本, 表[项]) == 0) return 项 + id起;
    return 0;
}

int 关键字标识(const char* 标识符) {
    return 完美哈希查表(标识符, 关键字表, 关键字桶, 关键字桶长,
                        关键字桶偏移, 关键字桶质数, 1);
}

int 文本标志标识(const char* 文本) {
    return 完美哈希查表(文本, 文本标志表, 文本标志桶, 文本标志桶长,
                        文本标志桶偏移, 文本标志桶质数, 101);
}

// ── 静态缓冲登记表 ─────────────────────────────────────────
// 运行时存在多处「地址固定、内容可变」的 static char 缓冲（获取行 / 读取文件 / 路径拼接 /
// 文件头部缓冲等）。字符码 的「按指针缓存 字符索引→字节偏移」绝不能缓存这些指针，否则
// 同一地址换了内容后会命中陈旧索引、解析出错。故写静态缓冲的函数须调用 登记静态缓冲(...)
// 把缓冲地址登记进来；字符码 对已登记地址退化为按需扫描（这些串通常很短，代价可忽略）。
static const char* 静态缓冲表[256];
static int 静态缓冲数 = 0;

static void 登记静态缓冲(const char* 文本) {
    int i;
    if (文本 == NULL) return;
    for (i = 0; i < 静态缓冲数; i++) if (静态缓冲表[i] == 文本) return;
    if (静态缓冲数 < 256) 静态缓冲表[静态缓冲数++] = 文本;
}

static int 是静态缓冲(const char* 文本) {
    int i;
    for (i = 0; i < 静态缓冲数; i++) if (静态缓冲表[i] == 文本) return 1;
    return 0;
}

// ── 字符索引缓存 ───────────────────────────────────────────
// 朴素实现每次取字符位置都要从串首重扫（O(位置)），而 PEG 分词在每个字符位置要对
// 数十条模式反复取字符码、并按字符位置切子串，整体退化为 O(n²)——这是自举编译
// （阶段一/阶段二）「卡半小时」的主因。此处为「长串」按指针缓存一张
// 字符索引→字节偏移 映射，命中后 O(1)；「短串」（前 64 字节内即出现 NUL，如正则
// 模式串）与静态缓冲仍从串首扫描（代价可忽略）。日月堆串从不释放，故以指针为键安全。
typedef struct {
    const char* 文本;   // NULL 表示空槽/构建中；非 NULL 表示索引已建好
    int 字符数;
    int* 偏移;          // 长度 字符数+1；偏移[字符数] == 字节长度
} 字符索引项;

static 字符索引项 字符索引缓存[4];

static 字符索引项* 查字符索引(const char* 文本) {
    int i;
    for (i = 0; i < 4; i++) {
        if (字符索引缓存[i].文本 == 文本) return &字符索引缓存[i];
    }
    return NULL;
}

static 字符索引项* 建字符索引(const char* 文本) {
    static int 轮 = 0;
    字符索引项* 项 = &字符索引缓存[轮];
    轮 = (轮 + 1) & 3;
    free(项->偏移);
    项->偏移 = NULL;
    项->文本 = NULL;
    int 字节数 = (int)strlen(文本);
    int 字符数 = 0;
    int b = 0;
    while (b < 字节数) {
        unsigned char c = (unsigned char)文本[b];
        if (c < 0x80) b += 1;
        else if ((c & 0xE0) == 0xC0) b += 2;
        else if ((c & 0xF0) == 0xE0) b += 3;
        else if ((c & 0xF8) == 0xF0) b += 4;
        else b += 1;
        字符数++;
    }
    项->偏移 = (int*)malloc((字符数 + 1) * sizeof(int));
    b = 0;
    int i;
    for (i = 0; i < 字符数; i++) {
        项->偏移[i] = b;
        unsigned char c = (unsigned char)文本[b];
        if (c < 0x80) b += 1;
        else if ((c & 0xE0) == 0xC0) b += 2;
        else if ((c & 0xF0) == 0xE0) b += 3;
        else if ((c & 0xF8) == 0xF0) b += 4;
        else b += 1;
    }
    项->偏移[字符数] = b;
    项->字符数 = 字符数;
    项->文本 = 文本;
    return 项;
}

static int 是短串(const char* 文本) {
    int i;
    for (i = 0; i < 64; i++) {
        if (文本[i] == '\0') return 1;
    }
    return 0;
}

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

// 将UTF-8字符位置转换为字节位置。长串走 字符索引缓存（O(1)）；短串/静态缓冲从串首扫描。
// 越界（char_pos ≥ 字符数）返回串尾字节位置（即字节长度），与原实现一致。
int 字符位置到字节位置(const char* str, int char_pos) {
    if (str == NULL) return 0;
    if (char_pos < 0) char_pos = 0;
    if (!是短串(str) && !是静态缓冲(str)) {
        字符索引项* 项 = 查字符索引(str);
        if (项 == NULL) 项 = 建字符索引(str);
        if (char_pos >= 项->字符数) return 项->偏移[项->字符数];
        return 项->偏移[char_pos];
    }
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
    登记静态缓冲(缓冲区);
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
    登记静态缓冲(缓冲区);
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
    登记静态缓冲(缓冲区);
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
    登记静态缓冲(缓冲区);
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
    登记静态缓冲(缓冲区);
    sprintf(缓冲区, "{\"%s\":\"%s\"}", 键, 值);
    return 缓冲区;
}

// 生成JSON整数
const char* JSON创建整数(const char* 键, int 值) {
    static char 缓冲区[4096];
    登记静态缓冲(缓冲区);
    sprintf(缓冲区, "{\"%s\":%d}", 键, 值);
    return 缓冲区;
}

// 生成JSON浮点数
const char* JSON创建浮点(const char* 键, double 值) {
    static char 缓冲区[4096];
    登记静态缓冲(缓冲区);
    sprintf(缓冲区, "{\"%s\":%g}", 键, 值);
    return 缓冲区;
}

// 生成JSON布尔值
const char* JSON创建布尔(const char* 键, int 值) {
    static char 缓冲区[4096];
    登记静态缓冲(缓冲区);
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
    登记静态缓冲(缓冲区);
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
    登记静态缓冲(缓冲区);
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
    登记静态缓冲(缓冲区);
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

// 设置环境变量（成功返回 0，失败返回 -1）
//
// Windows（MSVCRT）：值含多字节字符（如中文 UTF-8）时，`_putenv_s` 在非 UTF-8
// 本地代码页下会返回 EILSEQ，但变量实际已按原字节写入——窄 `getenv`（与窄 `获取环境变量`
// 配对）可原样读回。故将 EILSEQ 归一为成功，其余非零码视为失败。
// 不采用宽字符 `_wputenv_s`：其写入后窄 `getenv` 读回会经 ANSI 转换而失真。
int 设置环境变量(const char* 名称, const char* 值) {
#ifdef _WIN32
    int 结果 = _putenv_s(名称, 值);
    if (结果 == 0 || 结果 == EILSEQ) { return 0; }
    return -1;
#else
    return setenv(名称, 值, 1);
#endif
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

// 获取当前进程标识（PID）
int 获取进程标识() {
#ifdef _WIN32
    return (int)_getpid();
#else
    return (int)getpid();
#endif
}

// 创建子进程（异步）：经平台命令解释器执行 命令，成功后返回子进程标识（>0），失败返回 -1。
// Windows：`_spawnlp(_P_NOWAIT, ...)` 返回进程句柄（窄接口，供窄 `等待进程` 用）；
// POSIX：`fork` + `sh -c`，返回 pid。
int 创建进程(const char* 命令) {
#ifdef _WIN32
    intptr_t 句柄 = _spawnlp(_P_NOWAIT, "cmd.exe", "cmd.exe", "/c", 命令, (char*)NULL);
    if (句柄 == -1) { return -1; }
    return (int)句柄;
#else
    pid_t 子进程 = fork();
    if (子进程 < 0) { return -1; }
    if (子进程 == 0) {
        execl("/bin/sh", "sh", "-c", 命令, (char*)NULL);
        _exit(127);
    }
    return (int)子进程;
#endif
}

// 等待子进程结束：返回其退出码，失败返回 -1。
int 等待进程(int 标识) {
#ifdef _WIN32
    int 状态 = 0;
    if (_cwait(&状态, (intptr_t)标识, 0) == -1) { return -1; }
    return 状态;
#else
    int 状态 = 0;
    if (waitpid((pid_t)标识, &状态, 0) < 0) { return -1; }
    if (WIFEXITED(状态)) { return WEXITSTATUS(状态); }
    return -1;
#endif
}

// 退出当前进程（状态码即进程退出码）；不返回。
void 退出进程(int 状态码) {
    exit(状态码);
}

// 忽略指定信号（设为 SIG_IGN）：成功返回 0，失败返回 -1。
int 忽略信号(int 信号号) {
    if (signal(信号号, SIG_IGN) == SIG_ERR) { return -1; }
    return 0;
}

// 恢复指定信号的默认处理（设为 SIG_DFL）：成功返回 0，失败返回 -1。
int 默认信号处理(int 信号号) {
    if (signal(信号号, SIG_DFL) == SIG_ERR) { return -1; }
    return 0;
}

// 向当前进程发送信号：成功返回 0，失败返回非 0。
int 发送信号(int 信号号) {
    return raise(信号号);
}

// 获取当前工作目录
const char* 获取当前目录() {
    static char 缓冲区[1024];
    登记静态缓冲(缓冲区);
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
    登记静态缓冲(目录缓冲区);
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
    登记静态缓冲(连接缓冲区);
    snprintf(连接缓冲区, sizeof(连接缓冲区), "%s/%s", 路径1, 路径2);
    return 连接缓冲区;
}

// ==================== 字符串流操作 ====================

// 将字符串按行分割，返回行数
int 分割行数(const char* 文本) {
    int 行数 = 0;
    const char* p = 文本;
    while (*p) { if (*p == '\n') 行数++; p++; }
    if (p > 文本 && *(p - 1) != '\n') 行数++;
    return 行数;
}

// 行游标：获取行 若每次都从串首重扫到目标行，则「逐行遍历全文」为 O(n²)。
// 这里按 文本指针 记住「下一行的起始字节偏移」；顺序取行（行号+1）时从游标续扫，
// 使整趟遍历降为 O(n)。非顺序取行则退回从串首重扫（语义不变）。静态缓冲（地址固定、
// 内容可变）不参与游标，避免同一地址换内容后偏移失效。
typedef struct { const char* 文本; int 下一行; int 下一偏移; } 行游标项;
static 行游标项 行游标表[16];
static int 行游标轮 = 0;

static 行游标项* 查行游标(const char* 文本) {
    int i;
    for (i = 0; i < 16; i++) if (行游标表[i].文本 == 文本) return &行游标表[i];
    return NULL;
}

static 行游标项* 建行游标(const char* 文本) {
    行游标项* 项 = &行游标表[行游标轮];
    行游标轮 = (行游标轮 + 1) & 15;
    项->文本 = 文本; 项->下一行 = -1; 项->下一偏移 = 0;
    return 项;
}

// 获取指定行的内容（从0开始）
const char* 获取行(const char* 文本, int 行号) {
    static char 缓冲区[4096];
    登记静态缓冲(缓冲区);
    缓冲区[0] = '\0';
    int 当前行 = 0;
    const char* 开始 = 文本;
    if (!是静态缓冲(文本)) {
        行游标项* 项 = 查行游标(文本);
        if (项 != NULL && 行号 == 项->下一行) { 当前行 = 行号; 开始 = 文本 + 项->下一偏移; }
    }
    const char* p = 开始;
    while (*p) {
        if (*p == '\n') {
            if (当前行 == 行号) {
                int 长度 = (int)(p - 开始);
                if (长度 >= 4096) 长度 = 4095;
                strncpy(缓冲区, 开始, 长度);
                缓冲区[长度] = '\0';
                if (!是静态缓冲(文本)) {
                    行游标项* 项 = 查行游标(文本);
                    if (项 == NULL) 项 = 建行游标(文本);
                    项->下一行 = 行号 + 1; 项->下一偏移 = (int)(p + 1 - 文本);
                }
                return 缓冲区;
            }
            当前行++;
            开始 = p + 1;
        }
        p++;
    }
    if (当前行 == 行号 && p > 开始) {
        int 长度 = (int)(p - 开始);
        if (长度 >= 4096) 长度 = 4095;
        strncpy(缓冲区, 开始, 长度);
        缓冲区[长度] = '\0';
        if (!是静态缓冲(文本)) {
            行游标项* 项 = 查行游标(文本);
            if (项 == NULL) 项 = 建行游标(文本);
            项->下一行 = 行号 + 1; 项->下一偏移 = (int)(p - 文本);
        }
        return 缓冲区;
    }
    return "";
}

// ==================== 文本缓冲 ====================
// 发射层用「整数句柄」指向 C 侧可增长缓冲，以 O(1) 摊薄追加替代日月字符串的
// 整串拼接（日月 `+` 每次都 malloc 新串，导致发射期 O(n²)）。句柄从 1 开始；
// 句柄 0 视为「空缓冲」：追加无效、取整体返回 ""、作追加源无效。
typedef struct { char* 数据; long 长度; long 容量; } 文本缓冲;
static 文本缓冲* 文本缓冲表 = NULL;
static int 文本缓冲数 = 0;
static int 文本缓冲上限 = 0;

static 文本缓冲* 取文本缓冲(int 句柄) {
    if (句柄 <= 0 || 句柄 > 文本缓冲数) return NULL;
    文本缓冲* b = &文本缓冲表[句柄 - 1];
    return b->数据 ? b : NULL;
}

static void 文本缓冲预留(文本缓冲* b, long 额外) {
    long 需求 = b->长度 + 额外 + 1;
    if (需求 <= b->容量) return;
    long 新容量 = b->容量 ? b->容量 : 256;
    while (新容量 < 需求) 新容量 *= 2;
    char* 新数据 = (char*)realloc(b->数据, (size_t)新容量);
    if (!新数据) return;
    b->数据 = 新数据;
    b->容量 = 新容量;
}

int 新建文本缓冲(void) {
    if (文本缓冲数 >= 文本缓冲上限) {
        int 新上限 = 文本缓冲上限 ? 文本缓冲上限 * 2 : 1024;
        文本缓冲* 新表 = (文本缓冲*)realloc(文本缓冲表, (size_t)新上限 * sizeof(文本缓冲));
        if (!新表) return 0;
        文本缓冲表 = 新表;
        文本缓冲上限 = 新上限;
    }
    文本缓冲数++;
    文本缓冲* b = &文本缓冲表[文本缓冲数 - 1];
    b->数据 = NULL;
    b->长度 = 0;
    b->容量 = 0;
    文本缓冲预留(b, 0);
    if (b->数据) b->数据[0] = '\0';
    return 文本缓冲数;
}

void 文本缓冲追加(int 句柄, const char* 文本) {
    文本缓冲* b = 取文本缓冲(句柄);
    if (!b || !文本) return;
    long 长 = (long)strlen(文本);
    if (长 == 0) return;
    文本缓冲预留(b, 长);
    if (!b->数据) return;
    memcpy(b->数据 + b->长度, 文本, (size_t)长);
    b->长度 += 长;
    b->数据[b->长度] = '\0';
}

void 文本缓冲追加换行(int 句柄) {
    文本缓冲追加(句柄, "\n");
}

void 文本缓冲追加缓冲(int 目标, int 源) {
    文本缓冲* 目标b = 取文本缓冲(目标);
    文本缓冲* 源b = 取文本缓冲(源);
    if (!目标b || !源b || 源b->长度 == 0) return;
    文本缓冲预留(目标b, 源b->长度);
    if (!目标b->数据) return;
    memcpy(目标b->数据 + 目标b->长度, 源b->数据, (size_t)源b->长度);
    目标b->长度 += 源b->长度;
    目标b->数据[目标b->长度] = '\0';
}

void 文本缓冲从文本(int 句柄, const char* 文本) {
    文本缓冲* b = 取文本缓冲(句柄);
    if (!b) return;
    b->长度 = 0;
    if (b->数据) b->数据[0] = '\0';
    文本缓冲追加(句柄, 文本);
}

const char* 文本缓冲取整体(int 句柄) {
    文本缓冲* b = 取文本缓冲(句柄);
    if (!b || !b->数据) return "";
    return b->数据;
}

// ==================== 哈希表基建（T1-B / T5） ====================
// 自举编译器需要「名→值」登记表。旧实现把表存成 "名:值\n" 文本串，每次查表都要
// 分割行数 + 逐行 获取行：单次 O(行数 × 串长)，表一大就退化到 O(n²) 卡死。
// 这里用 C 侧开放寻址哈希表 + 整数句柄替代：查/插/删均 O(1) 摊薄。
// 句柄从 1 开始；句柄 0 视为「空表」：一切读写均为空操作。

// ── 文本表：字符串 → 字符串 ──
typedef struct {
    char** 键;
    char** 值;
    unsigned char* 占用;   // 0=空 1=占用 2=墓碑
    long 容量;
    long 数量;
    long 墓碑;
    char** 序键;           // 按插入顺序保存的键，供 O(1) 遍历
    long 序数;
    long 序容量;
} 文本表;

static 文本表* 文本表池 = NULL;
static long 文本表池数 = 0;
static long 文本表池上限 = 0;

static 文本表* 取文本表(int 句柄) {
    if (句柄 <= 0 || 句柄 > 文本表池数) return NULL;
    return &文本表池[句柄 - 1];
}

// FNV-1a
static unsigned long 文本哈希(const char* s) {
    unsigned long h = 1469598103934665603UL;
    while (*s) { h ^= (unsigned char)*s++; h *= 1099511628211UL; }
    return h;
}

// 重建为 新容量（丢弃墓碑）
static void 文本表重建(文本表* t, long 新容量) {
    char** 新键 = (char**)calloc((size_t)新容量, sizeof(char*));
    char** 新值 = (char**)calloc((size_t)新容量, sizeof(char*));
    unsigned char* 新占用 = (unsigned char*)calloc((size_t)新容量, 1);
    if (!新键 || !新值 || !新占用) {
        free(新键); free(新值); free(新占用);
        return;
    }
    long 掩码 = 新容量 - 1;
    for (long i = 0; i < t->容量; i++) {
        if (t->占用[i] != 1) continue;
        unsigned long h = 文本哈希(t->键[i]);
        long j = (long)(h & (unsigned long)掩码);
        while (新占用[j]) j = (j + 1) & 掩码;
        新键[j] = t->键[i];
        新值[j] = t->值[i];
        新占用[j] = 1;
    }
    free(t->键); free(t->值); free(t->占用);
    t->键 = 新键; t->值 = 新值; t->占用 = 新占用;
    t->容量 = 新容量; t->墓碑 = 0;
}

int 新建文本表(void) {
    if (文本表池数 >= 文本表池上限) {
        long 新上限 = 文本表池上限 ? 文本表池上限 * 2 : 256;
        文本表* 新池 = (文本表*)realloc(文本表池, (size_t)新上限 * sizeof(文本表));
        if (!新池) return 0;
        文本表池 = 新池;
        文本表池上限 = 新上限;
    }
    文本表池数++;
    文本表* t = &文本表池[文本表池数 - 1];
    t->键 = NULL; t->值 = NULL; t->占用 = NULL;
    t->容量 = 0; t->数量 = 0; t->墓碑 = 0;
    t->序键 = NULL; t->序数 = 0; t->序容量 = 0;
    return 文本表池数;
}

static void 文本表记序(文本表* t, const char* 键) {
    if (t->序数 >= t->序容量) {
        long 新容量 = t->序容量 ? t->序容量 * 2 : 16;
        char** 新序 = (char**)realloc(t->序键, (size_t)新容量 * sizeof(char*));
        if (!新序) return;
        t->序键 = 新序; t->序容量 = 新容量;
    }
    t->序键[t->序数] = (char*)键;
    t->序数++;
}

// 插入（覆盖=1 时替换已存在值；覆盖=0 时保留原值，即「只增不改」）
static void 文本表写入(int 句柄, const char* 键, const char* 值, int 覆盖) {
    文本表* t = 取文本表(句柄);
    if (!t || !键) return;
    if ((t->数量 + t->墓碑 + 1) * 4 >= t->容量 * 3) {
        文本表重建(t, t->容量 ? t->容量 * 2 : 16);
        if (t->容量 == 0) return;
    }
    unsigned long h = 文本哈希(键);
    long 掩码 = t->容量 - 1;
    long i = (long)(h & (unsigned long)掩码);
    long 墓碑 = -1;
    while (1) {
        unsigned char s = t->占用[i];
        if (s == 0) {
            long 目标 = (墓碑 >= 0) ? 墓碑 : i;
            t->键[目标] = strdup(键);
            t->值[目标] = strdup(值 ? 值 : "");
            t->占用[目标] = 1;
            t->数量++;
            if (墓碑 >= 0) t->墓碑--;
            文本表记序(t, t->键[目标]);
            return;
        }
        if (s == 1 && t->键[i] && strcmp(t->键[i], 键) == 0) {
            if (覆盖) {
                free(t->值[i]);
                t->值[i] = strdup(值 ? 值 : "");
            }
            return;
        }
        if (s == 2 && 墓碑 < 0) 墓碑 = i;
        i = (i + 1) & 掩码;
    }
}

void 文本表插入(int 句柄, const char* 键, const char* 值) {
    文本表写入(句柄, 键, 值, 0);
}

void 文本表置(int 句柄, const char* 键, const char* 值) {
    文本表写入(句柄, 键, 值, 1);
}

static long 文本表找(文本表* t, const char* 键) {
    if (!t || t->容量 == 0 || !键) return -1;
    unsigned long h = 文本哈希(键);
    long 掩码 = t->容量 - 1;
    long i = (long)(h & (unsigned long)掩码);
    while (t->占用[i]) {
        if (t->占用[i] == 1 && t->键[i] && strcmp(t->键[i], 键) == 0) return i;
        i = (i + 1) & 掩码;
    }
    return -1;
}

const char* 文本表取(int 句柄, const char* 键) {
    long i = 文本表找(取文本表(句柄), 键);
    if (i < 0) return "";
    return 文本表池[句柄 - 1].值[i];
}

int 文本表含(int 句柄, const char* 键) {
    return 文本表找(取文本表(句柄), 键) >= 0 ? 1 : 0;
}

void 文本表删(int 句柄, const char* 键) {
    文本表* t = 取文本表(句柄);
    long i = 文本表找(t, 键);
    if (i < 0) return;
    // 同步从 序键 移除该项（序号指向同一 strdup 指针），避免遍历时取到已释放指针
    for (long j = 0; j < t->序数; j++) {
        if (t->序键[j] == t->键[i]) {
            for (long k = j; k + 1 < t->序数; k++) t->序键[k] = t->序键[k + 1];
            t->序数--;
            break;
        }
    }
    free(t->键[i]); t->键[i] = NULL;
    free(t->值[i]); t->值[i] = NULL;
    t->占用[i] = 2;
    t->数量--; t->墓碑++;
}

int 文本表键数(int 句柄) {
    文本表* t = 取文本表(句柄);
    return t ? t->序数 : 0;
}

const char* 文本表键(int 句柄, int 序号) {
    文本表* t = 取文本表(句柄);
    if (!t || 序号 < 0 || 序号 >= t->序数) return "";
    return t->序键[序号];
}

void 文本表清(int 句柄) {
    文本表* t = 取文本表(句柄);
    if (!t) return;
    for (long i = 0; i < t->容量; i++) {
        if (t->占用[i] == 1) { free(t->键[i]); free(t->值[i]); }
    }
    free(t->键); free(t->值); free(t->占用);
    t->键 = NULL; t->值 = NULL; t->占用 = NULL;
    t->容量 = 0; t->数量 = 0; t->墓碑 = 0;
    t->序数 = 0;
}

// ── 整数表：整数 → 整数（T5：以 token/标识符 编号为键） ──
typedef struct {
    long* 键;
    long* 值;
    unsigned char* 占用;
    long 容量;
    long 数量;
    long 墓碑;
} 整数表;

static 整数表* 整数表池 = NULL;
static long 整数表池数 = 0;
static long 整数表池上限 = 0;

static 整数表* 取整数表(int 句柄) {
    if (句柄 <= 0 || 句柄 > 整数表池数) return NULL;
    return &整数表池[句柄 - 1];
}

int 新建整数表(void) {
    if (整数表池数 >= 整数表池上限) {
        long 新上限 = 整数表池上限 ? 整数表池上限 * 2 : 256;
        整数表* 新池 = (整数表*)realloc(整数表池, (size_t)新上限 * sizeof(整数表));
        if (!新池) return 0;
        整数表池 = 新池;
        整数表池上限 = 新上限;
    }
    整数表池数++;
    整数表* t = &整数表池[整数表池数 - 1];
    t->键 = NULL; t->值 = NULL; t->占用 = NULL;
    t->容量 = 0; t->数量 = 0; t->墓碑 = 0;
    return 整数表池数;
}

static unsigned long 整数哈希(long k) {
    unsigned long h = (unsigned long)k * 1099511628211UL;
    return h ^ (h >> 29);
}

static void 整数表重建(整数表* t, long 新容量) {
    long* 新键 = (long*)calloc((size_t)新容量, sizeof(long));
    long* 新值 = (long*)calloc((size_t)新容量, sizeof(long));
    unsigned char* 新占用 = (unsigned char*)calloc((size_t)新容量, 1);
    if (!新键 || !新值 || !新占用) {
        free(新键); free(新值); free(新占用);
        return;
    }
    long 掩码 = 新容量 - 1;
    for (long i = 0; i < t->容量; i++) {
        if (t->占用[i] != 1) continue;
        long j = (long)(整数哈希(t->键[i]) & (unsigned long)掩码);
        while (新占用[j]) j = (j + 1) & 掩码;
        新键[j] = t->键[i]; 新值[j] = t->值[i]; 新占用[j] = 1;
    }
    free(t->键); free(t->值); free(t->占用);
    t->键 = 新键; t->值 = 新值; t->占用 = 新占用;
    t->容量 = 新容量; t->墓碑 = 0;
}

void 整数表置(int 句柄, long 键, long 值) {
    整数表* t = 取整数表(句柄);
    if (!t) return;
    if ((t->数量 + t->墓碑 + 1) * 4 >= t->容量 * 3) {
        整数表重建(t, t->容量 ? t->容量 * 2 : 16);
        if (t->容量 == 0) return;
    }
    long 掩码 = t->容量 - 1;
    long i = (long)(整数哈希(键) & (unsigned long)掩码);
    long 墓碑 = -1;
    while (1) {
        unsigned char s = t->占用[i];
        if (s == 0) {
            long 目标 = (墓碑 >= 0) ? 墓碑 : i;
            t->键[目标] = 键; t->值[目标] = 值; t->占用[目标] = 1;
            t->数量++;
            if (墓碑 >= 0) t->墓碑--;
            return;
        }
        if (s == 1 && t->键[i] == 键) { t->值[i] = 值; return; }
        if (s == 2 && 墓碑 < 0) 墓碑 = i;
        i = (i + 1) & 掩码;
    }
}

static long 整数表找(整数表* t, int 键) {
    if (!t || t->容量 == 0) return -1;
    long 掩码 = t->容量 - 1;
    long i = (long)(整数哈希(键) & (unsigned long)掩码);
    while (t->占用[i]) {
        if (t->占用[i] == 1 && t->键[i] == 键) return i;
        i = (i + 1) & 掩码;
    }
    return -1;
}

int 整数表取(int 句柄, int 键) {
    long i = 整数表找(取整数表(句柄), 键);
    return i < 0 ? 0 : 整数表池[句柄 - 1].值[i];
}

int 整数表含(int 句柄, int 键) {
    return 整数表找(取整数表(句柄), 键) >= 0 ? 1 : 0;
}

void 整数表删(int 句柄, int 键) {
    整数表* t = 取整数表(句柄);
    long i = 整数表找(t, 键);
    if (i < 0) return;
    t->占用[i] = 2;
    t->数量--; t->墓碑++;
}

// ── 标识符 intern：同一文本恒得同一编号（自 1 递增） ──
static char** 内部键 = NULL;
static long* 内部号 = NULL;
static unsigned char* 内部占用 = NULL;
static long 内部容量 = 0;
static long 内部数 = 0;

static void 内部重建(long 新容量) {
    char** 新键 = (char**)calloc((size_t)新容量, sizeof(char*));
    long* 新号 = (long*)calloc((size_t)新容量, sizeof(long));
    unsigned char* 新占用 = (unsigned char*)calloc((size_t)新容量, 1);
    if (!新键 || !新号 || !新占用) {
        free(新键); free(新号); free(新占用);
        return;
    }
    long 掩码 = 新容量 - 1;
    for (long i = 0; i < 内部容量; i++) {
        if (内部占用[i] != 1) continue;
        long j = (long)(文本哈希(内部键[i]) & (unsigned long)掩码);
        while (新占用[j]) j = (j + 1) & 掩码;
        新键[j] = 内部键[i]; 新号[j] = 内部号[i]; 新占用[j] = 1;
    }
    free(内部键); free(内部号); free(内部占用);
    内部键 = 新键; 内部号 = 新号; 内部占用 = 新占用;
    内部容量 = 新容量;
}

int 标识符编号(const char* 名) {
    if (!名) return 0;
    if ((内部数 + 1) * 4 >= 内部容量 * 3) {
        内部重建(内部容量 ? 内部容量 * 2 : 256);
        if (内部容量 == 0) return 0;
    }
    long 掩码 = 内部容量 - 1;
    long i = (long)(文本哈希(名) & (unsigned long)掩码);
    while (内部占用[i]) {
        if (strcmp(内部键[i], 名) == 0) return 内部号[i];
        i = (i + 1) & 掩码;
    }
    内部键[i] = strdup(名);
    内部号[i] = 内部数 + 1;
    内部占用[i] = 1;
    内部数++;
    return 内部数;
}

// ── 关键字 / 文本标志 token id：统一委托共享完美哈希实现 ──
// 权威表与桶见 ../共享/标记标识表.h（由 工具/gen_hash.py 生成）；
// 宿主 C++ 前端（源/前端/词法分析器.cpp）调用同一实现，杜绝两份漂移。
int 关键字编号(const char* s) { return 关键字标识(s); }
int 文本标志编号(const char* s) { return 文本标志标识(s); }

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
    登记静态缓冲(缓冲区);
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
    登记静态缓冲(缓冲区);
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
    登记静态缓冲(缓冲区);
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

int 字符码(const char* 文本, int 位置) {
    if (文本 == NULL) return -1;
    if (位置 < 0) 位置 = 0;   // 与原实现一致：负下标按串首处理
    int 字节位置;
    字符索引项* 项 = 查字符索引(文本);
    if (项 != NULL) {
        if (位置 >= 项->字符数) return -1;
        字节位置 = 项->偏移[位置];
    } else if (是短串(文本) || 是静态缓冲(文本)) {
        字节位置 = 0;
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
    } else {
        项 = 建字符索引(文本);
        if (位置 >= 项->字符数) return -1;
        字节位置 = 项->偏移[位置];
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
    登记静态缓冲(缓冲区);
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

// K11：`(码)字符` 的**独立副本**版本——每次调用新分配，多次结果互不覆盖，
// 语义与自举链（胖字符串包装按值拷贝）一致。宿主链 `字符` 内建改调本函数。
// 不登记静态缓冲表：副本内容不可变，走通用字符索引路径即可；且登记表仅 256 槽，
// 逐次登记会挤占真正会原地改写的静态缓冲，反而破坏索引缓存假设。
const char* 字符序列拷贝(int 码点) {
    char* 缓冲区 = (char*)malloc(5);
    if (缓冲区 == NULL) return "";
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

// ==================== 运行时安全检查（内存安全 §4.1） ====================
// 边界/空指针检查辅助，供运行时与（后续 §4.2）生成的代码调用。
//
// 内建 动态数组* 越界报错默认**关闭**：自举编译器（自举/解析器.心）依赖
// `动态数组获取` 越界返回 0 表达 EOF，默认开启会破坏自举链。需要时设环境
// 变量 `日月边界检查=1` 打开（可开关，见 文档/内存安全方案.md §4.1）。

static int 边界检查开启(void) {
    static int 缓存 = -1;
    if (缓存 < 0) {
#ifdef _WIN32
        // 必须用宽字符版 getenv：Windows 的窄环境块按 ANSI 码页编码，
        // 中文变量名会与源码里的 UTF-8 字面量不匹配，只能走 UTF-16 路径。
        const wchar_t* 值 = _wgetenv(L"日月边界检查");
        缓存 = (值 && 值[0] && 值[0] != L'0') ? 1 : 0;
#else
        const char* 值 = getenv("日月边界检查");
        缓存 = (值 && 值[0] && 值[0] != '0') ? 1 : 0;
#endif
    }
    return 缓存;
}

void 运行时越界报告(const char* 位置, int 索引, int 长度) {
    fflush(stdout);
    fprintf(stderr, "\n运行时越界: %s 索引 %d 超出 [0, %d)\n", 位置, 索引, 长度);
    fflush(stderr);
    exit(1);
}

void 运行时边界检查(int 索引, int 长度, const char* 位置) {
    if (索引 < 0 || 索引 >= 长度) 运行时越界报告(位置, 索引, 长度);
}

// 定长数组下标越界陷阱（内存安全 §4.2）：由代码生成器在 `名[下标]` 读写前插入
// 显式比较后调用，越界即中止。默认开启（用户决策：默认开），无需环境变量。
void 运行时数组越界陷阱(int 索引, int 长度) {
    运行时越界报告("数组下标", 索引, 长度);
}

// 阶段7（G7）：编译期契约（@前置/@后置）违例——打印诊断并以非零码中止。
void 运行时契约失败(const char* 诊断) {
    fflush(stdout);
    fprintf(stderr, "%s\n", 诊断);
    fflush(stderr);
    exit(1);
}

void 运行时空检查(const void* 指针, const char* 位置) {
    if (!指针) {
        fflush(stdout);
        fprintf(stderr, "\n运行时空指针: %s\n", 位置);
        fflush(stderr);
        exit(1);
    }
}

// ==================== 字符串胖指针（G3 选项2） ====================
// 语言层「字符串」值 = 指向本结构的指针；数据恒为 NUL 结尾的字节串，长度 = 字符数（UTF-8）。
// C 边界：生产型 C 函数返回裸 char* → 字符串包装(...) 补长度（拷贝，避免静态缓冲/切片别名）；
//         消费型 C 函数要裸 char* → 字符串数据(...) 解包。
typedef struct {
    const char* 数据;
    int 长度;
} 字符串对象;

字符串对象* 胖字符串包装(const char* 数据) {
    const char* d = (数据 != NULL) ? 数据 : "";
    size_t n = strlen(d);
    char* 拷贝 = (char*)malloc(n + 1);
    if (拷贝 == NULL) return NULL;
    memcpy(拷贝, d, n + 1);
    字符串对象* s = (字符串对象*)malloc(sizeof(字符串对象));
    if (s == NULL) { free(拷贝); return NULL; }
    s->数据 = 拷贝;
    s->长度 = 获取字符数(拷贝);
    return s;
}

const char* 胖字符串数据(字符串对象* s) {
    return (s != NULL && s->数据 != NULL) ? s->数据 : "";
}

int 胖字符串长度(字符串对象* s) {
    return (s != NULL) ? s->长度 : 0;
}

int 胖字符串字符数(字符串对象* s) {
    return (s != NULL) ? s->长度 : 0;
}

int 胖字符串比较(字符串对象* a, 字符串对象* b) {
    return strcmp(胖字符串数据(a), 胖字符串数据(b));
}

int 胖字符串字符码(字符串对象* s, int 索引) {
    return 字符码(胖字符串数据(s), 索引);
}

字符串对象* 胖字符串拼接(字符串对象* a, 字符串对象* b) {
    const char* da = 胖字符串数据(a);
    const char* db = 胖字符串数据(b);
    long long la = (long long)strlen(da);
    long long lb = (long long)strlen(db);
    char* r = (char*)malloc((size_t)(la + lb + 1));
    if (r == NULL) return NULL;
    memcpy(r, da, (size_t)la);
    memcpy(r + la, db, (size_t)lb);
    r[la + lb] = '\0';
    字符串对象* s = (字符串对象*)malloc(sizeof(字符串对象));
    if (s == NULL) { free(r); return NULL; }
    s->数据 = r;
    s->长度 = 获取字符数(r);
    return s;
}

字符串对象* 胖字符串子串(字符串对象* s, int 起, int 长) {
    const char* d = 胖字符串数据(s);
    int 字节起 = 字符位置到字节位置(d, 起);
    int 字节止 = 字符位置到字节位置(d, 起 + 长);
    int 字节长 = 字节止 - 字节起;
    if (字节长 < 0) 字节长 = 0;
    char* r = (char*)malloc((size_t)(字节长 + 1));
    if (r == NULL) return NULL;
    memcpy(r, d + 字节起, (size_t)字节长);
    r[字节长] = '\0';
    字符串对象* o = (字符串对象*)malloc(sizeof(字符串对象));
    if (o == NULL) { free(r); return NULL; }
    o->数据 = r;
    o->长度 = 获取字符数(r);
    return o;
}

// 字符串下标（UTF-8 字符级）：越界即中止
字符串对象* 胖字符串取字符(字符串对象* s, int 索引) {
    int n = 胖字符串长度(s);
    if (索引 < 0 || 索引 >= n) 运行时越界报告("字符串下标", 索引, n);
    return 胖字符串子串(s, 索引, 1);
}

// 字符串下标（UTF-8 字符级·带界检查，产出码点/文字；内存安全 §4.3 A.9）：越界即中止。
// 宿主链（裸 char*）用。
int 字符串字符码检查(const char* 文本, int 索引) {
    if (文本 == NULL) 文本 = "";
    int n = 获取字符数(文本);
    if (索引 < 0 || 索引 >= n) 运行时越界报告("字符串下标", 索引, n);
    return 字符码(文本, 索引);
}

// 字符串下标（UTF-8 字符级·带界检查，产出码点/文字；内存安全 §4.3 A.9）：越界即中止。
// 自举链（胖字符串 字符串对象*）用。
int 胖字符串字符码检查(字符串对象* s, int 索引) {
    int n = 胖字符串长度(s);
    if (索引 < 0 || 索引 >= n) 运行时越界报告("字符串下标", 索引, n);
    return 胖字符串字符码(s, 索引);
}

int 胖字符串转整数(字符串对象* s) {
    return atoi(胖字符串数据(s));
}

// ==================== FFI：动态库外部函数（阶段8 G10） ====================
// `外部 函数 "库" "符号" 名(参数)(结果: 整数);` 解糖为对 外部调用N 的调用。
// 两链一致：库名/符号名均以裸 char*（NUL 结尾字节串）传入；参数与返回值须同型，
// 全「整数」（i32）走 外部调用N，全「浮点」（统一双精度）走 外部调用浮点N；最多 4 参。
// 按（库, 符号）缓存已解析地址，重复调用不再查找。
#ifdef _WIN32
typedef void* 动态库句柄;
static 动态库句柄 动态库打开(const char* 库名) {
    return (动态库句柄)LoadLibraryA(库名);
}
static void* 动态库取符号(动态库句柄 库, const char* 符号名) {
    return (void*)GetProcAddress((HMODULE)库, 符号名);
}
#else
#include <dlfcn.h>
typedef void* 动态库句柄;
static 动态库句柄 动态库打开(const char* 库名) {
    return dlopen(库名, RTLD_NOW | RTLD_LOCAL);
}
static void* 动态库取符号(动态库句柄 库, const char* 符号名) {
    return dlsym(库, 符号名);
}
#endif

typedef struct {
    const char* 库名;
    const char* 符号名;
    void* 地址;
} 外部符号项;

static 外部符号项 外部符号缓存[128];
static int 外部符号缓存数 = 0;

// 解析（库, 符号）→ 函数地址；失败即报错中止（外部函数无法以语言层异常表达）。
static void* 外部符号解析(const char* 库名, const char* 符号名) {
    int i;
    if (库名 == NULL) 库名 = "";
    if (符号名 == NULL) 符号名 = "";
    for (i = 0; i < 外部符号缓存数; i++) {
        if (strcmp(外部符号缓存[i].库名, 库名) == 0 &&
            strcmp(外部符号缓存[i].符号名, 符号名) == 0) {
            return 外部符号缓存[i].地址;
        }
    }
    动态库句柄 库 = 动态库打开(库名);
    if (库 == NULL) {
        fprintf(stderr, "外部函数加载失败: %s: %s\n", 库名, 符号名);
        fflush(stderr);
        exit(1);
    }
    void* 地址 = 动态库取符号(库, 符号名);
    if (地址 == NULL) {
        fprintf(stderr, "外部函数加载失败: %s: %s\n", 库名, 符号名);
        fflush(stderr);
        exit(1);
    }
    if (外部符号缓存数 < 128) {
        外部符号缓存[外部符号缓存数].库名 = 库名;
        外部符号缓存[外部符号缓存数].符号名 = 符号名;
        外部符号缓存[外部符号缓存数].地址 = 地址;
        外部符号缓存数++;
    }
    return 地址;
}

typedef int (*外部函数指针0)(void);
typedef int (*外部函数指针1)(int);
typedef int (*外部函数指针2)(int, int);
typedef int (*外部函数指针3)(int, int, int);
typedef int (*外部函数指针4)(int, int, int, int);

int 外部调用0(const char* 库名, const char* 符号名) {
    return ((外部函数指针0)外部符号解析(库名, 符号名))();
}

int 外部调用1(const char* 库名, const char* 符号名, int 参1) {
    return ((外部函数指针1)外部符号解析(库名, 符号名))(参1);
}

int 外部调用2(const char* 库名, const char* 符号名, int 参1, int 参2) {
    return ((外部函数指针2)外部符号解析(库名, 符号名))(参1, 参2);
}

int 外部调用3(const char* 库名, const char* 符号名, int 参1, int 参2, int 参3) {
    return ((外部函数指针3)外部符号解析(库名, 符号名))(参1, 参2, 参3);
}

int 外部调用4(const char* 库名, const char* 符号名, int 参1, int 参2, int 参3, int 参4) {
    return ((外部函数指针4)外部符号解析(库名, 符号名))(参1, 参2, 参3, 参4);
}

// 阶段P4 浮点增量：参数与返回值均为双精度（浮点 ABI 走 XMM 寄存器）。
typedef double (*外部函数浮点指针0)(void);
typedef double (*外部函数浮点指针1)(double);
typedef double (*外部函数浮点指针2)(double, double);
typedef double (*外部函数浮点指针3)(double, double, double);
typedef double (*外部函数浮点指针4)(double, double, double, double);

double 外部调用浮点0(const char* 库名, const char* 符号名) {
    return ((外部函数浮点指针0)外部符号解析(库名, 符号名))();
}

double 外部调用浮点1(const char* 库名, const char* 符号名, double 参1) {
    return ((外部函数浮点指针1)外部符号解析(库名, 符号名))(参1);
}

double 外部调用浮点2(const char* 库名, const char* 符号名, double 参1, double 参2) {
    return ((外部函数浮点指针2)外部符号解析(库名, 符号名))(参1, 参2);
}

double 外部调用浮点3(const char* 库名, const char* 符号名, double 参1, double 参2, double 参3) {
    return ((外部函数浮点指针3)外部符号解析(库名, 符号名))(参1, 参2, 参3);
}

double 外部调用浮点4(const char* 库名, const char* 符号名, double 参1, double 参2, double 参3, double 参4) {
    return ((外部函数浮点指针4)外部符号解析(库名, 符号名))(参1, 参2, 参3, 参4);
}

// 阶段P4 回调增量：末位参数为 C 函数指针（语言层 `函数` 值 = 代码地址），
// 其余参数为 整数；返回值 整数。回调签名按 int (*)(void) 调用（无参回调）。
typedef int (*外部回调无参)(void);
typedef int (*外部函数回调指针1)(外部回调无参);
typedef int (*外部函数回调指针2)(int, 外部回调无参);
typedef int (*外部函数回调指针3)(int, int, 外部回调无参);
typedef int (*外部函数回调指针4)(int, int, int, 外部回调无参);

int 外部调用回调1(const char* 库名, const char* 符号名, 外部回调无参 回调) {
    return ((外部函数回调指针1)外部符号解析(库名, 符号名))(回调);
}

int 外部调用回调2(const char* 库名, const char* 符号名, int 参1, 外部回调无参 回调) {
    return ((外部函数回调指针2)外部符号解析(库名, 符号名))(参1, 回调);
}

int 外部调用回调3(const char* 库名, const char* 符号名, int 参1, int 参2, 外部回调无参 回调) {
    return ((外部函数回调指针3)外部符号解析(库名, 符号名))(参1, 参2, 回调);
}

int 外部调用回调4(const char* 库名, const char* 符号名, int 参1, int 参2, int 参3, 外部回调无参 回调) {
    return ((外部函数回调指针4)外部符号解析(库名, 符号名))(参1, 参2, 参3, 回调);
}

// ==================== 切片胖指针（内存安全 §4.2/§4.3） ====================
// 语言层「切片」值 = 指向本结构的指针；数据 = 指向原数组第「起」个元素的指针（零拷贝
// 视图），长度 = 元素个数（止-起）。分片 arr[起:止] 产出切片；切片[i] 越界即中止。
typedef struct {
    void* 数据;
    int 长度;
} 切片对象;

切片对象* 切片创建(void* 数据, int 长度) {
    切片对象* s = (切片对象*)malloc(sizeof(切片对象));
    if (s == NULL) return NULL;
    s->数据 = 数据;
    s->长度 = (长度 > 0) ? 长度 : 0;
    return s;
}

void* 切片数据(切片对象* s) {
    return (s != NULL) ? s->数据 : NULL;
}

int 切片长度(切片对象* s) {
    return (s != NULL) ? s->长度 : 0;
}

// 整型切片取元素（越界即中止）
int 切片取整数(切片对象* s, int 索引) {
    运行时空检查(s, "切片取整数");
    if (索引 < 0 || 索引 >= s->长度) 运行时越界报告("切片下标", 索引, s->长度);
    return ((int*)s->数据)[索引];
}

// 指针型（字符串等）切片取元素（越界即中止）
void* 切片取指针(切片对象* s, int 索引) {
    运行时空检查(s, "切片取指针");
    if (索引 < 0 || 索引 >= s->长度) 运行时越界报告("切片下标", 索引, s->长度);
    return ((void**)s->数据)[索引];
}

// 整型切片写元素（越界即中止；内存安全 §八 O3-2c3：可变视图写入直达借用源存储）
void 切片设置整数(切片对象* s, int 索引, int 值) {
    运行时空检查(s, "切片设置整数");
    if (索引 < 0 || 索引 >= s->长度) 运行时越界报告("切片下标", 索引, s->长度);
    ((int*)s->数据)[索引] = 值;
}

// 指针型（字符串等）切片写元素（越界即中止；内存安全 §八 O3-2c3）
void 切片设置指针(切片对象* s, int 索引, void* 值) {
    运行时空检查(s, "切片设置指针");
    if (索引 < 0 || 索引 >= s->长度) 运行时越界报告("切片下标", 索引, s->长度);
    ((void**)s->数据)[索引] = 值;
}

// ==================== 动态数组 ====================

#define 动态数组初始大小 16

typedef struct {
    int* 数据;
    int 数组大小;
    int 容量;
} 动态数组;

// 阶段3C：保守 GC 接线（实现见文件末尾 GC 段）。默认不登记（GC 未启用时零开销）。
void 注销GC对象(void* 对象);
static void GC登记容器(void* 对象, void (*析构)(void*));
static void GC析构动态数组(void* 对象);
static void GC析构动态字符串数组(void* 对象);

动态数组* 创建动态数组() {
    // calloc 而非 malloc：堆对象零初始化（内存安全 §4.1），消除潜在脏读；
    // 容量内未被写入的槽位也保证为 0，而非未定义值。
    动态数组* arr = (动态数组*)calloc(1, sizeof(动态数组));
    if (!arr) return NULL;
    arr->数据 = (int*)calloc(动态数组初始大小, sizeof(int));
    arr->数组大小 = 0;
    arr->容量 = 动态数组初始大小;
    // 阶段3C：GC 启用时登记（清除时连同 arr->数据 一并释放）
    GC登记容器(arr, GC析构动态数组);
    return arr;
}

void 动态数组添加(动态数组* arr, int 值) {
    if (arr->数组大小 >= arr->容量) {
        int 旧容量 = arr->容量;
        int 新容量 = 旧容量 * 2;
        int* 新数据 = (int*)realloc(arr->数据, (size_t)新容量 * sizeof(int));
        if (!新数据) return;
        memset(新数据 + 旧容量, 0, (size_t)(新容量 - 旧容量) * sizeof(int));
        arr->数据 = 新数据;
        arr->容量 = 新容量;
    }
    arr->数据[arr->数组大小++] = 值;
}

int 动态数组获取(动态数组* arr, int 索引) {
    if (索引 >= 0 && 索引 < arr->数组大小) {
        return arr->数据[索引];
    }
    if (边界检查开启()) 运行时边界检查(索引, arr->数组大小, "动态数组获取");
    return 0;
}

void 动态数组设置(动态数组* arr, int 索引, int 值) {
    if (索引 >= 0 && 索引 < arr->数组大小) {
        arr->数据[索引] = 值;
        return;
    }
    if (边界检查开启()) 运行时边界检查(索引, arr->数组大小, "动态数组设置");
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
        return;
    }
    if (边界检查开启()) 运行时边界检查(索引, arr->数组大小, "动态数组删除");
}

void 动态数组插入(动态数组* arr, int 索引, int 值) {
    if (索引 < 0 || 索引 > arr->数组大小) {
        if (边界检查开启()) 运行时边界检查(索引, arr->数组大小 + 1, "动态数组插入");
        return;
    }
    动态数组添加(arr, 0);  // 扩展
    for (int i = arr->数组大小 - 1; i > 索引; i--) {
        arr->数据[i] = arr->数据[i - 1];
    }
    arr->数据[索引] = 值;
}

void 释放动态数组(动态数组* arr) {
    // 阶段3C：先注销再释放，避免 GC 表残留悬垂表项（重复 free）
    注销GC对象(arr);
    free(arr->数据);
    free(arr);
}

// 语言层 arr[下标] / arr[下标]=值 专用入口（内存安全 §4.2 动态数组胖指针）：
// 与内建 动态数组获取/设置（默认返回 0，供自举解析器表达 EOF）不同，
// 这里**恒开启**越界陷阱——下标语法必须安全。长度取自胖指针（结构体内 数组大小）。
int 动态数组索引获取(动态数组* arr, int 索引) {
    if (索引 < 0 || 索引 >= arr->数组大小) 运行时越界报告("动态数组下标", 索引, arr->数组大小);
    return arr->数据[索引];
}

void 动态数组索引设置(动态数组* arr, int 索引, int 值) {
    if (索引 < 0 || 索引 >= arr->数组大小) 运行时越界报告("动态数组下标赋值", 索引, arr->数组大小);
    arr->数据[索引] = 值;
}

// 语言层 动态数组[起:止] 专用入口（内存安全 §4.2 / §S1）：长度与元素基址都取自运行期胖指针，
// 止 < 0 视作「到末尾」（供 [起:] / [:] 缺省止界）；越界即中止，返回胖切片（零拷贝视图）。
切片对象* 动态数组分片(动态数组* arr, int 起, int 止) {
    int 长度 = arr->数组大小;
    if (止 < 0) 止 = 长度;
    if (起 < 0 || 起 > 长度) 运行时越界报告("动态数组分片起", 起, 长度);
    if (止 < 起 || 止 > 长度) 运行时越界报告("动态数组分片止", 止, 长度);
    return 切片创建(arr->数据 + 起, 止 - 起);
}

// ==================== 字节缓冲收发（带长度，含 0x00） ====================
// 语言层以「整数数组（每元素 0..255）」承载二进制（同 标准库/二进制.心、DNS.心、WebSocket.心）。
// 既有 TCP发送/TCP接收 以 文本(char*) 传数据、按 strlen 计长 ⇒ 二进制载荷含 0x00 会被截断。
// 下列入口按**整数数组长度**收发，规避截断（接收逐字节追加到调用方传入的数组）。
int TCP发送字节(int 套接字, void* 字节数组) {
    动态数组* arr = (动态数组*)字节数组;
    if (!arr || arr->数组大小 <= 0) return 0;
    int 字节数 = arr->数组大小;
    char* 缓冲 = (char*)malloc((size_t)字节数);
    if (!缓冲) return -1;
    for (int i = 0; i < 字节数; i++) 缓冲[i] = (char)(arr->数据[i] & 0xFF);
    int 结果 = send((SOCKET)套接字, 缓冲, 字节数, 0);
    free(缓冲);
    return 结果;
}

int TCP接收字节(int 套接字, void* 字节数组) {
    static char 缓冲[8192];
    登记静态缓冲(缓冲);
    int 字节数 = recv((SOCKET)套接字, 缓冲, (int)sizeof(缓冲), 0);
    if (字节数 <= 0) return 字节数;
    动态数组* arr = (动态数组*)字节数组;
    if (arr) {
        for (int i = 0; i < 字节数; i++) 动态数组添加(arr, (int)(unsigned char)缓冲[i]);
    }
    return 字节数;
}

// UDP 字节缓冲收发（带长度，含 0x00；同 TCP发送字节/TCP接收字节，规避文本 strlen 截断）
int UDP发送字节到(int 套接字, void* 字节数组, const char* 地址, int 端口) {
    动态数组* arr = (动态数组*)字节数组;
    if (!arr || arr->数组大小 <= 0) return 0;
    int 字节数 = arr->数组大小;
    char* 缓冲 = (char*)malloc((size_t)字节数);
    if (!缓冲) return -1;
    for (int i = 0; i < 字节数; i++) 缓冲[i] = (char)(arr->数据[i] & 0xFF);
    struct sockaddr_in 目标地址;
    目标地址.sin_family = AF_INET;
    目标地址.sin_port = htons(端口);
    inet_pton(AF_INET, 地址, &目标地址.sin_addr);
    int 结果 = sendto((SOCKET)套接字, 缓冲, 字节数, 0, (struct sockaddr*)&目标地址, sizeof(目标地址));
    free(缓冲);
    return 结果;
}

int UDP接收字节(int 套接字, void* 字节数组) {
    static char 缓冲[8192];
    登记静态缓冲(缓冲);
    struct sockaddr_in 发送者地址;
    int 地址长度 = sizeof(发送者地址);
    int 字节数 = recvfrom((SOCKET)套接字, 缓冲, (int)sizeof(缓冲), 0, (struct sockaddr*)&发送者地址, &地址长度);
    if (字节数 <= 0) return 字节数;
    动态数组* arr = (动态数组*)字节数组;
    if (arr) {
        for (int i = 0; i < 字节数; i++) 动态数组添加(arr, (int)(unsigned char)缓冲[i]);
    }
    return 字节数;
}

// 动态字符串数组
typedef struct {
    const char** 数据;
    int 数组大小;
    int 容量;
} 动态字符串数组;

动态字符串数组* 创建动态字符串数组() {
    动态字符串数组* arr = (动态字符串数组*)calloc(1, sizeof(动态字符串数组));
    if (!arr) return NULL;
    arr->数据 = (const char**)calloc(动态数组初始大小, sizeof(const char*));
    arr->数组大小 = 0;
    arr->容量 = 动态数组初始大小;
    // 阶段3C：GC 启用时登记（清除时连同 arr->数据 一并释放）
    GC登记容器(arr, GC析构动态字符串数组);
    return arr;
}

void 动态字符串数组添加(动态字符串数组* arr, const char* 值) {
    if (arr->数组大小 >= arr->容量) {
        int 旧容量 = arr->容量;
        int 新容量 = 旧容量 * 2;
        const char** 新数据 = (const char**)realloc(arr->数据, (size_t)新容量 * sizeof(const char*));
        if (!新数据) return;
        memset(新数据 + 旧容量, 0, (size_t)(新容量 - 旧容量) * sizeof(const char*));
        arr->数据 = 新数据;
        arr->容量 = 新容量;
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
            int 旧容量 = arr->容量;
            int 新容量 = 旧容量 * 2;
            const char** 新数据 = (const char**)realloc(arr->数据, (size_t)新容量 * sizeof(const char*));
            if (!新数据) return;
            memset(新数据 + 旧容量, 0, (size_t)(新容量 - 旧容量) * sizeof(const char*));
            arr->数据 = 新数据;
            arr->容量 = 新容量;
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
    // 阶段3C：先注销再释放，避免 GC 表残留悬垂表项（重复 free）
    注销GC对象(arr);
    for (int i = 0; i < arr->数组大小; i++) {
        free((void*)arr->数据[i]);
    }
    free(arr->数据);
    free(arr);
}

// 语言层 sarr[下标] / sarr[下标]=值 专用入口（内存安全 §4.2 动态字符串数组胖指针）：
// 恒开启越界陷阱；语义与内建 动态字符串数组设置 一致（赋值复制并释放旧值）。
const char* 动态字符串数组索引获取(动态字符串数组* arr, int 索引) {
    if (索引 < 0 || 索引 >= arr->数组大小) 运行时越界报告("动态字符串数组下标", 索引, arr->数组大小);
    return arr->数据[索引];
}

void 动态字符串数组索引设置(动态字符串数组* arr, int 索引, const char* 值) {
    if (索引 < 0 || 索引 >= arr->数组大小) 运行时越界报告("动态字符串数组下标赋值", 索引, arr->数组大小);
    free((void*)arr->数据[索引]);
    arr->数据[索引] = strdup(值);
}

// 语言层 动态字符串数组[起:止] 专用入口（内存安全 §4.2 / §S1）：语义同 动态数组分片。
切片对象* 动态字符串数组分片(动态字符串数组* arr, int 起, int 止) {
    int 长度 = arr->数组大小;
    if (止 < 0) 止 = 长度;
    if (起 < 0 || 起 > 长度) 运行时越界报告("动态字符串数组分片起", 起, 长度);
    if (止 < 起 || 止 > 长度) 运行时越界报告("动态字符串数组分片止", 止, 长度);
    return 切片创建((void*)(arr->数据 + 起), 止 - 起);
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
    通道数据* ch = (通道数据*)calloc(1, sizeof(通道数据));
    if (!ch) return NULL;
    ch->容量 = 容量 > 0 ? 容量 : 1;
    ch->数据 = (int*)calloc((size_t)ch->容量, sizeof(int));
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

// 协程实现（协作式单线程：基于 Windows 纤程，不引入原子/同步原语）。
// 语义：创建协程时不执行；首次「恢复」才开始执行，运行至「让出」暂停并把让出值交给恢复者；
// 协程体返回后标记已完成，之后「恢复」归返 0。让出值以 整数（int）槽传递（语言层为 整数）。
typedef struct {
    void (*入口)(void*);   // 协程体函数（签名 空类型(指针)）
    void* 帧;              // 形参帧（各形参按 8 字节槽存放）
    void* 纤程;            // 承载协程体的纤程
    volatile int 已完成;   // 抢占式调度下由工作线程写入、调度线程读取
    int 让出值;
} 协程对象;

// 阶段10 G11-B：二者改为线程局部——抢占式调度让每个任务跑在自己的 OS 线程上，
// 各线程需独立记录「我正在跑哪个协程」，否则会互相覆盖。
// 用 C11 的 _Thread_local（clang/gcc 在 Windows 与 POSIX 目标上均支持）。
#define 日月线程局部 _Thread_local

static 日月线程局部 协程对象* 当前协程 = NULL;
static 日月线程局部 void* 协程宿主纤程 = NULL;

static void 协程入口包装(void* 参数) {
    协程对象* 协 = (协程对象*)参数;
    当前协程 = 协;
    协->入口(协->帧);
    协->已完成 = 1;
    SwitchToFiber(协程宿主纤程);
}

void* 创建协程对象(void* 入口, void* 帧) {
    if (!协程宿主纤程) 协程宿主纤程 = ConvertThreadToFiber(NULL);
    协程对象* 协 = (协程对象*)calloc(1, sizeof(协程对象));
    if (!协) return NULL;
    协->入口 = (void (*)(void*))入口;
    协->帧 = 帧;
    协->已完成 = 0;
    协->让出值 = 0;
    协->纤程 = CreateFiber(0, 协程入口包装, 协);
    return 协;
}

void 协程让出(int 值) {
    if (!当前协程) return;
    当前协程->让出值 = 值;
    // 阶段10 G11-B：抢占式调度的任务跑在独立 OS 线程上（本线程无宿主纤程），
    // 此时「让出」解释为让出 CPU 时间片；协作式纤程模式下仍切回宿主纤程。
    if (!协程宿主纤程) {
#ifdef _WIN32
        SwitchToThread();
#endif
        return;
    }
    SwitchToFiber(协程宿主纤程);
}

int 协程恢复(void* 句柄) {
    协程对象* 协 = (协程对象*)句柄;
    if (!协 || 协->已完成) return 0;
    协程对象* 先前 = 当前协程;
    当前协程 = 协;
    SwitchToFiber(协->纤程);
    当前协程 = 先前;
    if (协->已完成) return 0;
    return 协->让出值;
}

// 轻量协作式调度器（goroutine 式，协作式单线程）：
// 把若干协程注册为「任务」，按注册顺序轮转恢复，直至全部任务完成。
// 依赖同一宿主纤程：任务内「让出」即切回本调度器所在纤程，故为确定性单线程序列。
typedef struct 调度任务 {
    void* 协程;
    struct 调度任务* 下一个;
} 调度任务;

static 调度任务* 调度头 = NULL;
static 调度任务* 调度尾 = NULL;
static int 调度任务数 = 0;

// 注册一个协程句柄为调度任务（按注册顺序轮转）。忽略空句柄。
void 调度注册(void* 句柄) {
    if (!句柄) return;
    调度任务* 节点 = (调度任务*)calloc(1, sizeof(调度任务));
    if (!节点) return;
    节点->协程 = 句柄;
    节点->下一个 = NULL;
    if (调度尾) 调度尾->下一个 = 节点; else 调度头 = 节点;
    调度尾 = 节点;
    调度任务数++;
}

// 轮转运行：每趟对尚未完成的任务各恢复一次；当某趟无人让出（全部完成）时停止。
// 返回已注册任务总数。注意：若任务永不完成且持续让出，则本函数不返回（协作式固有）。
int 调度运行(void) {
    int 本轮让出数;
    do {
        本轮让出数 = 0;
        调度任务* 节点 = 调度头;
        while (节点) {
            协程对象* 协 = (协程对象*)节点->协程;
            if (协 && !协->已完成) {
                协程恢复(节点->协程);
                if (!协->已完成) 本轮让出数++;
            }
            节点 = 节点->下一个;
        }
    } while (本轮让出数 > 0);
    return 调度任务数;
}

/* ============================================================================
 * 阶段 10 G11-B：抢占式调度（每个任务一个 OS 线程 + 时间片轮转）
 * ----------------------------------------------------------------------------
 * 与协作式 调度运行 的区别：
 *   - 协程体在**独立线程**上直接执行（不再用纤程，也就不会在「让出」处切回）；
 *   - 调度线程按 时间片 毫秒做轮转：ResumeThread 恢复一个任务，Sleep 一个时间片，
 *     SuspendThread 挂起它，再轮到下一个——即便任务不让出也会被强制切走（抢占）；
 *   - 任务内「让出」退化为「让出本时间片剩余 CPU」（SwitchToThread），语义仍安全。
 * 已知限制（明记）：
 *   - SuspendThread 可能打断持有堆锁/IO 锁的线程，故**任务内应避免长时间占用分配器**；
 *   - 若某任务永不结束且不分配内存，本函数不返回（与抢占语义本身一致）；
 *   - 任务之间共享数据须自行用互斥锁/原子原语（本片已提供）。
 * ========================================================================== */

typedef struct 抢占任务 {
    void* 协程;
    void* 线程;
    struct 抢占任务* 下一个;
} 抢占任务;

static 抢占任务* 抢占头 = NULL;
static 抢占任务* 抢占尾 = NULL;

// 任务主体：在「本线程」上直接跑协程体（不经纤程），结束时置已完成。
static void 抢占任务主体(void* 参数) {
    协程对象* 协 = (协程对象*)参数;
    if (!协) return;
    当前协程 = 协;
    协->入口(协->帧);
    协->已完成 = 1;
}

#ifdef _WIN32
static DWORD WINAPI 抢占任务执行(LPVOID 参数) {
    抢占任务主体(参数);
    return 0;
}
#endif

// 时间片轮转运行抢占式任务。返回已注册任务总数（与 调度运行 一致，不重置任务表）。
int 抢占式调度运行(int 时间片毫秒) {
    if (时间片毫秒 < 1) 时间片毫秒 = 1;
    // 上次运行留下的节点先回收（任务表本身不重置，与 调度运行 语义一致）。
    while (抢占头) {
        抢占任务* 下 = 抢占头->下一个;
        free(抢占头);
        抢占头 = 下;
    }
    抢占尾 = NULL;
    调度任务* 节点 = 调度头;
    if (!节点) return 0;
    // 每个未完成任务建一个工作线程（挂起态启动，由调度线程决定何时开跑）。
    while (节点) {
        协程对象* 协 = (协程对象*)节点->协程;
        抢占任务* 项 = (抢占任务*)calloc(1, sizeof(抢占任务));
        if (!项) break;
        项->协程 = 节点->协程;
        项->线程 = NULL;
        if (抢占尾) 抢占尾->下一个 = 项; else 抢占头 = 项;
        抢占尾 = 项;
#ifdef _WIN32
        if (协 && !协->已完成) {
            项->线程 = CreateThread(NULL, 0, 抢占任务执行, 协, CREATE_SUSPENDED, NULL);
            if (!项->线程) 抢占任务主体(协);   // 线程创建失败：就地串行跑完
        }
#else
        // 非 Windows 无 SuspendThread 等价原语：退化为就地串行执行（仍正确完成）。
        if (协 && !协->已完成) 抢占任务主体(协);
#endif
        节点 = 节点->下一个;
    }
    // 轮转：每趟让每个未完成任务各跑一个时间片，直到全部完成。
    int 仍有未完成 = 1;
    while (仍有未完成) {
        仍有未完成 = 0;
        抢占任务* 项 = 抢占头;
        while (项) {
            协程对象* 协 = (协程对象*)项->协程;
            if (协 && !协->已完成) {
#ifdef _WIN32
                if (项->线程) {
                    ResumeThread((HANDLE)项->线程);
                    Sleep((DWORD)时间片毫秒);
                    SuspendThread((HANDLE)项->线程);
                }
#endif
                if (!协->已完成) 仍有未完成 = 1;
            }
            项 = 项->下一个;
        }
    }
    // 全部完成：归还线程句柄。
#ifdef _WIN32
    抢占任务* 收 = 抢占头;
    while (收) {
        if (收->线程) {
            WaitForSingleObject((HANDLE)收->线程, INFINITE);
            CloseHandle((HANDLE)收->线程);
            收->线程 = NULL;
        }
        收 = 收->下一个;
    }
#endif
    return 调度任务数;
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

// ── 自举链异常：同线程异常帧栈 + setjmp/longjmp ──
// 「尝试」入口压入异常帧，紧接由 IR 直接调用 _setjmp 保存返回点；
// 「抛出」把异常值（统一 整数64）写入栈顶帧后 longjmp 回该帧；
// 「捕获」先取走值再弹帧（使 捕获 体内的抛出继续向外传播）。
// 无帧可抛（未被任何 尝试 捕获）→ 打印诊断并以非零码退出。
#include <setjmp.h>

typedef struct 异常帧 {
    struct 异常帧* 上一帧;
    long long 抛出值;
    jmp_buf 环境;
} 异常帧;

// 日月线程局部 宏已在协程块（阶段10 G11-B）统一定义，此处复用。

static 日月线程局部 异常帧* 当前异常帧 = 0;

void* 日月_异常帧入栈(void) {
    异常帧* 帧 = (异常帧*)malloc(sizeof(异常帧));
    if (!帧) {
        fprintf(stderr, "未捕获异常: 异常帧分配失败\n");
        exit(1);
    }
    帧->上一帧 = 当前异常帧;
    帧->抛出值 = 0;
    当前异常帧 = 帧;
    return (void*)&帧->环境;
}

void 日月_异常帧出栈(void) {
    异常帧* 帧 = 当前异常帧;
    if (!帧) return;
    当前异常帧 = 帧->上一帧;
    free(帧);
}

long long 日月_取抛出值(void) {
    if (!当前异常帧) return 0;
    return 当前异常帧->抛出值;
}

void 日月_抛出(long long 值) {
    if (!当前异常帧) {
        fprintf(stderr, "未捕获异常: %lld\n", 值);
        exit(1);
    }
    当前异常帧->抛出值 = 值;
    longjmp(当前异常帧->环境, 1);
}

// HTTP GET 支持（简化版，使用系统命令）
static char http缓冲区[65536];

const char* HTTP获取(const char* url) {
    char 命令[1024];
    登记静态缓冲(http缓冲区);
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

// 从已打开的流读取全部内容（不接管/不关闭 fp，调用方负责关闭）。
// 返回 malloc 的缓冲区（调用方拥有；失败返回 NULL）。
// K7：n==0 既可能是正常 EOF，也可能是读错误（如读取中被截断/IO 故障）。
// 后者返回半截数据会误导调用方（如编译器的源码读取），故显式判 ferror。
// 独立出来的动因：故障注入自测（测试/运行时读取自测.c）可直接对失效句柄调用本函数。
static char* 从流读取整文件(FILE* fp) {
    size_t 容量 = 65536;
    size_t 长度 = 0;
    char* 缓冲 = (char*)malloc(容量);
    if (!缓冲) return NULL;
    for (;;) {
        if (长度 + 1 >= 容量) {
            char* 新缓冲;
            容量 = 容量 * 2;
            新缓冲 = (char*)realloc(缓冲, 容量);
            if (!新缓冲) { free(缓冲); return NULL; }
            缓冲 = 新缓冲;
        }
        size_t n = fread(缓冲 + 长度, 1, 容量 - 1 - 长度, fp);
        长度 += n;
        if (n == 0) {
            if (ferror(fp)) { free(缓冲); return NULL; }
            break;
        }
    }
    缓冲[长度] = '\0';
    return 缓冲;
}

// 读取整个文件（返回 malloc 的缓冲区，调用方拥有；失败返回 NULL）
static char* 读取整文件(const char* 路径) {
    FILE* fp = UTF8打开文件(路径, "r");
    char* 缓冲;
    if (!fp) return NULL;
    缓冲 = 从流读取整文件(fp);
    fclose(fp);
    return 缓冲;
}

// 判断字节串是否为合法 UTF-8（用长度限定，避免读到越界）
static int 是合法UTF8(const char* 数据, size_t 长度) {
    size_t i = 0;
    while (i < 长度) {
        unsigned char c = (unsigned char)数据[i];
        if (c < 0x80) {
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 >= 长度 || ((unsigned char)数据[i + 1] & 0xC0) != 0x80) return 0;
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 >= 长度 || ((unsigned char)数据[i + 1] & 0xC0) != 0x80 ||
                ((unsigned char)数据[i + 2] & 0xC0) != 0x80) return 0;
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            if (i + 3 >= 长度 || ((unsigned char)数据[i + 1] & 0xC0) != 0x80 ||
                ((unsigned char)数据[i + 2] & 0xC0) != 0x80 ||
                ((unsigned char)数据[i + 3] & 0xC0) != 0x80) return 0;
            i += 4;
        } else {
            return 0;
        }
    }
    return 1;
}

#ifdef _WIN32
// 将本地 ANSI(CP_ACP，如 GBK) 编码文本转换为 UTF-8，返回 malloc 缓冲区（调用方拥有）
static char* ANSI转UTF8(const char* 输入, size_t 输入长度, size_t* 输出长度) {
    int 宽长度 = MultiByteToWideChar(CP_ACP, 0, 输入, (int)输入长度, NULL, 0);
    wchar_t* 宽缓冲;
    int 转换长度;
    char* 结果;
    if (宽长度 <= 0) return NULL;
    宽缓冲 = (wchar_t*)malloc((size_t)宽长度 * sizeof(wchar_t));
    if (!宽缓冲) return NULL;
    MultiByteToWideChar(CP_ACP, 0, 输入, (int)输入长度, 宽缓冲, 宽长度);
    转换长度 = WideCharToMultiByte(CP_UTF8, 0, 宽缓冲, 宽长度, NULL, 0, NULL, NULL);
    if (转换长度 <= 0) { free(宽缓冲); return NULL; }
    结果 = (char*)malloc((size_t)转换长度 + 1);
    if (!结果) { free(宽缓冲); return NULL; }
    WideCharToMultiByte(CP_UTF8, 0, 宽缓冲, 宽长度, 结果, 转换长度, NULL, NULL);
    free(宽缓冲);
    结果[转换长度] = '\0';
    *输出长度 = (size_t)转换长度;
    return 结果;
}
#endif

// 文件读取（静态缓冲，调用方无需释放；自动兼容 UTF-8/GBK 源码）
const char* 读取文件(const char* 路径) {
    static char* 缓冲 = NULL;
    char* 临时 = 读取整文件(路径);
    if (!临时) return "";
    // 去除 UTF-8 BOM（EF BB BF）
    if (strlen(临时) >= 3 && (unsigned char)临时[0] == 0xEF &&
        (unsigned char)临时[1] == 0xBB && (unsigned char)临时[2] == 0xBF) {
        memmove(临时, 临时 + 3, strlen(临时 + 3) + 1);
    }
    // 非合法 UTF-8 时按本地 ANSI(GBK) 编码转换为 UTF-8
    if (!是合法UTF8(临时, strlen(临时))) {
#ifdef _WIN32
        size_t 新长度 = 0;
        char* 转换 = ANSI转UTF8(临时, strlen(临时), &新长度);
        if (转换) { free(临时); 临时 = 转换; }
#endif
    }
    free(缓冲);
    缓冲 = 临时;
    登记静态缓冲(缓冲);
    return 缓冲;
}

// ===== 文件句柄 IO（供自举编译器内建 打开/关闭/写入/读取/读取行） =====
static FILE* 文件句柄缓冲[256];
static int 文件句柄计数 = 0;

int 文件打开(const char* 路径, const char* 模式) {
    FILE* fp;
    if (文件句柄计数 >= 256) return -1;
    fp = UTF8打开文件(路径, 模式);
    if (!fp) return -1;
    文件句柄缓冲[文件句柄计数] = fp;
    return 文件句柄计数++;
}

int 文件关闭(int 句柄) {
    if (句柄 < 0 || 句柄 >= 文件句柄计数 || !文件句柄缓冲[句柄]) return -1;
    fclose(文件句柄缓冲[句柄]);
    文件句柄缓冲[句柄] = NULL;
    return 0;
}

int 文件写入(int 句柄, const char* 内容) {
    if (句柄 < 0 || 句柄 >= 文件句柄计数 || !文件句柄缓冲[句柄]) return -1;
    if (!内容) return -1;
    fputs(内容, 文件句柄缓冲[句柄]);
    return 0;
}

// 读取一行并剥离行尾换行；返回每次新分配的副本（调用方拥有）
const char* 文件读取行(int 句柄) {
    static char 行缓冲[8192];
    size_t 长;
    char* 副本;
    if (句柄 < 0 || 句柄 >= 文件句柄计数 || !文件句柄缓冲[句柄]) return "";
    if (!fgets(行缓冲, sizeof(行缓冲), 文件句柄缓冲[句柄])) return "";
    长 = strlen(行缓冲);
    while (长 > 0 && (行缓冲[长 - 1] == '\n' || 行缓冲[长 - 1] == '\r')) 行缓冲[--长] = '\0';
    副本 = (char*)malloc(长 + 1);
    if (!副本) return "";
    memcpy(副本, 行缓冲, 长 + 1);
    return 副本;
}

int 文件读取(int 句柄) {
    int 值 = 0;
    if (句柄 < 0 || 句柄 >= 文件句柄计数 || !文件句柄缓冲[句柄]) return 0;
    if (fscanf(文件句柄缓冲[句柄], "%d", &值) != 1) return 0;
    return 值;
}

// ===== 标准流 IO（阶段9 G9：LSP 服务端 stdio 帧收发） =====
// Windows 下标准流默认文本模式：输出 \n 会被翻译为 \r\n、输入 \r\n 会被折叠为 \n，
// 两者都会破坏 LSP 的 Content-Length 字节计数；首次使用即切换为二进制模式。
static void 标准流二进制(void) {
    static int 已设 = 0;
    if (已设) return;
    已设 = 1;
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif
}

// 读取标准输入一行（剥离行尾 CR/LF；EOF 返回 ""）
const char* 读取标准输入行(void) {
    static char 行缓冲[8192];
    size_t 长;
    标准流二进制();
    if (!fgets(行缓冲, sizeof(行缓冲), stdin)) return "";
    长 = strlen(行缓冲);
    while (长 > 0 && (行缓冲[长 - 1] == '\n' || 行缓冲[长 - 1] == '\r')) 行缓冲[--长] = '\0';
    登记静态缓冲(行缓冲);
    return 行缓冲;
}

// 读取标准输入指定字节数（静态缓冲；不足时返回实际读到的内容）
const char* 读取标准输入字节(int 数量) {
    static char* 缓冲 = NULL;
    static int 容量 = 0;
    size_t 读入;
    if (数量 <= 0) return "";
    标准流二进制();
    if (数量 + 1 > 容量) {
        char* 新缓冲 = (char*)realloc(缓冲, (size_t)(数量 + 1));
        if (!新缓冲) return "";
        缓冲 = 新缓冲;
        容量 = 数量 + 1;
    }
    读入 = fread(缓冲, 1, (size_t)数量, stdin);
    缓冲[读入] = '\0';
    登记静态缓冲(缓冲);
    return 缓冲;
}

// 写入标准输出（不追加换行、立即刷新；LSP 帧按字节原样写出）
int 写入标准输出(const char* 文本) {
    标准流二进制();
    if (!文本) return -1;
    fputs(文本, stdout);
    fflush(stdout);
    return 0;
}

// 文本的 UTF-8 字节长度（LSP Content-Length 按字节计，不能用字符数）
int 字节长度(const char* 文本) {
    if (!文本) return 0;
    return (int)strlen(文本);
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
/* ============================================================================
 * 阶段3C：保守 GC —— 安全化与接线
 * ----------------------------------------------------------------------------
 * 3C 前的问题（内存安全方案 §五 第 5 条）：对象表定长 1024、标记只扫固定 4096
 * 个栈字、`注册GC对象` 无任何调用点，GC 实为「死」路径。
 * 3C 做法：
 *  1. 对象表 / 标记表按需倍增扩容，取消 1024 上限；扩容失败则放弃登记（宁可不回收，绝不误 free）。
 *  2. 标记范围 = 执行GC 的局部变量（向下 8 字节对齐）→ 本线程栈顶（TIB.StackBase），
 *     上限 64MiB 防御；寄存器经 setjmp 快照刷入内存一并按字扫描（保守近似）。
 *  3. 容器登记携带析构：数组清除时连同内部数据缓冲一并 free；内部缓冲不单独入表，
 *     避免 realloc 后表项悬垂。
 *  4. 显式释放（释放动态数组 / 释放动态字符串数组）先注销，杜绝二次 free。
 *  5. 与 ARC 表互斥：共享对象不入本表（见 3B 段）。
 *  6. 默认关闭；环境变量 日月GC=1（非 "0"）或语言层 `()启用GC` 显式开启；
 *     阈值默认 100，可由 `(N)设置GC阈值` 或 环境变量 日月GC阈值=N 覆盖。
 *  7. 回收信息写 stderr，不污染标准输出。
 * ========================================================================== */
typedef struct GC条目 {
    void* 指针;
    void (*析构)(void*);
} GC条目;

static GC条目* GC对象表 = NULL;
static unsigned char* GC标记表 = NULL;
static int GC对象数量 = 0;
static int GC容量 = 0;
static int GC启用 = 0;
static int GC阈值 = 100;  // 登记数达到阈值即触发一次回收
static int GC惰性已查 = 0;
static char* GC栈基址 = NULL;

void 执行GC(void);  // 前向声明

static int GC确保容量(int 需要) {
    if (需要 <= GC容量) return 1;
    int 新容量 = GC容量 > 0 ? GC容量 : 256;
    while (新容量 < 需要) 新容量 *= 2;
    GC条目* 新表 = (GC条目*)realloc(GC对象表, (size_t)新容量 * sizeof(GC条目));
    if (!新表) return 0;
    GC对象表 = 新表;
    unsigned char* 新标记 = (unsigned char*)realloc(GC标记表, (size_t)新容量);
    if (!新标记) return 0;
    GC标记表 = 新标记;
    GC容量 = 新容量;
    return 1;
}

// 环境变量只在首次登记时读一次（默认关闭，不影响既有默认路径）
static void GC惰性初始化(void) {
    if (GC惰性已查) return;
    GC惰性已查 = 1;
    const char* 环境开关 = getenv("日月GC");
    if (环境开关 && 环境开关[0] != '\0' && strcmp(环境开关, "0") != 0) GC启用 = 1;
    const char* 阈值文本 = getenv("日月GC阈值");
    if (阈值文本 && 阈值文本[0] != '\0') {
        int 值 = atoi(阈值文本);
        if (值 > 0) GC阈值 = 值;
    }
}

// 栈顶：直接读 TIB 的 StackBase（x64 为 gs:0x08），即本线程栈的最高地址，
// 与调用深度无关。构造函数里取锚点只能拿到「构造函数那一层」的地址，
// 而 main 的帧比构造函数帧更高（构造函数由 CRT 在更深的 _initterm 中调用），
// 会漏扫 主程序 的整个帧 —— 这正是 3C 首版误回收的根因。
static char* GC当前线程栈顶(void) {
#if defined(__x86_64__) && (defined(_WIN32) || defined(_WIN64))
    void* 栈顶 = NULL;
    __asm__ volatile("movq %%gs:0x08, %0" : "=r"(栈顶));
    return (char*)栈顶;
#else
    return NULL;
#endif
}

__attribute__((constructor))
static void GC记录初始栈顶(void) {
    GC栈基址 = GC当前线程栈顶();
}

static void GC登记容器(void* 对象, void (*析构)(void*)) {
    if (!对象) return;
    GC惰性初始化();
    if (!GC启用) return;  // 默认关闭：不登记、不回收
    if (!GC确保容量(GC对象数量 + 1)) return;
    GC对象表[GC对象数量].指针 = 对象;
    GC对象表[GC对象数量].析构 = 析构;
    GC标记表[GC对象数量] = 0;
    GC对象数量++;
    if (GC对象数量 >= GC阈值) {
        执行GC();
    }
}

void 注册GC对象(void* 对象) {
    GC登记容器(对象, NULL);
}

// 注销（显式释放前调用）：对象不在表内则无操作
void 注销GC对象(void* 对象) {
    if (!对象 || GC对象数量 == 0) return;
    for (int i = 0; i < GC对象数量; i++) {
        if (GC对象表[i].指针 == 对象) {
            GC对象表[i] = GC对象表[GC对象数量 - 1];
            GC标记表[i] = GC标记表[GC对象数量 - 1];
            GC对象数量--;
            return;
        }
    }
}

static void GC标记候选(void* 候选) {
    if (!候选) return;
    for (int i = 0; i < GC对象数量; i++) {
        if (GC对象表[i].指针 == 候选) {
            if (GC标记表[i] == 0) GC标记表[i] = 1;
            return;
        }
    }
}

void 标记对象(void* 对象) {
    GC标记候选(对象);
}

void 清除所有标记() {
    for (int i = 0; i < GC对象数量; i++) {
        GC标记表[i] = 0;
    }
}

// 按字保守扫描 [起, 止) 区间内的候选指针并标记（64MiB 上限防御越界读）
static void GC扫描区间(const char* 起, const char* 止) {
    if (!起 || !止 || 止 <= 起) return;
    size_t 字节 = (size_t)(止 - 起);
    if (字节 > (size_t)64 * 1024 * 1024) 字节 = (size_t)64 * 1024 * 1024;
    for (size_t 偏移 = 0; 偏移 + sizeof(void*) <= 字节; 偏移 += sizeof(void*)) {
        void* 候选 = NULL;
        memcpy(&候选, 起 + 偏移, sizeof(void*));
        GC标记候选(候选);
    }
}

void 执行GC() {
    GC惰性初始化();
    if (GC对象数量 == 0) return;
    清除所有标记();
    // 标记阶段：寄存器经 setjmp 刷入快照 + 扫描 [栈低界, 栈顶)
    jmp_buf 寄存器快照;
    memset(&寄存器快照, 0, sizeof(寄存器快照));
    if (setjmp(寄存器快照) == 0) {
        // 仅取寄存器快照，不跳转
    }
    // 低界用本函数内局部变量的地址：它必然落在执行GC 自己的帧内，
    // 一定低于调用者（主程序）的整个帧，故 [低界, 栈顶) 必覆盖全部活帧。
    // 不能用 __builtin_frame_address(0)：该目标上对无帧函数会返回调用者帧。
    // 且 1 字节局部变量的地址不保证 8 字节对齐，按字扫描必须向下对齐，
    // 否则窗口整体错位一字节，任何指针都匹配不上。
    char 栈锚 = 0;
    const char* 栈低界 =
        (const char*)((size_t)&栈锚 & ~(size_t)(sizeof(void*) - 1));
    char* 栈顶 = GC当前线程栈顶();
    if (!栈顶) 栈顶 = GC栈基址;
    if (!栈顶 || 栈低界 >= 栈顶) return;  // 栈界未知时放弃本次回收，宁可不回收也不误 free
    GC扫描区间(栈低界, 栈顶);
    GC扫描区间((const char*)&寄存器快照,
               (const char*)&寄存器快照 + sizeof(寄存器快照));

    // 清除阶段：释放未标记的对象（先跑析构，再释放对象本体）
    int 清除数量 = 0;
    for (int i = GC对象数量 - 1; i >= 0; i--) {
        if (GC标记表[i] == 0) {
            if (GC对象表[i].析构) GC对象表[i].析构(GC对象表[i].指针);
            free(GC对象表[i].指针);
            GC对象表[i] = GC对象表[GC对象数量 - 1];
            GC标记表[i] = GC标记表[GC对象数量 - 1];
            GC对象数量--;
            清除数量++;
        }
    }
    if (清除数量 > 0) {
        fprintf(stderr, "[GC] 清除了 %d 个对象, 剩余 %d\n", 清除数量, GC对象数量);
    }
}

void 启用GC() {
    GC启用 = 1;
}

void 设置GC阈值(int 阈值) {
    if (阈值 > 0) GC阈值 = 阈值;
}

int 获取GC对象数量() {
    return GC对象数量;
}

// 容器析构：对象本体由 执行GC 统一 free，这里只释放其内部持有的缓冲
static void GC析构动态数组(void* 对象) {
    动态数组* arr = (动态数组*)对象;
    if (arr && arr->数据) free(arr->数据);
}

static void GC析构动态字符串数组(void* 对象) {
    动态字符串数组* arr = (动态字符串数组*)对象;
    if (arr && arr->数据) free(arr->数据);
}

/* ============================================================================
 * 阶段 3B：ARC 引用计数共享对象（显式共享「逃生舱」）
 * ----------------------------------------------------------------------------
 * 设计要点：
 *  1. 句柄用「整数32 槽号」而非裸指针：与语言层 `整数` 直接兼容，双链无 ABI 风险；
 *  2. 槽表按需扩容（不设 1024 上限），对象归零释放后槽位可复用；
 *  3. **不注册进保守 GC 表**：ARC 与 GC 互斥，避免「ARC 已 free 又被 GC 二次 free」；
 *  4. 默认路径（阶段2 所有权 + 确定性 Drop）不受影响，本组入口仅显式调用时生效。
 * ========================================================================== */
typedef struct 共享头 {
    int 引用计数;
    int 元素数量;
} 共享头;

static 共享头** 共享槽表 = NULL;
static int 共享槽容量 = 0;
static int 共享槽最高 = 0;   /* 曾分配到的槽上界（含空洞） */
static int 共享存活 = 0;
static int 共享分配累计 = 0;
static int 共享释放累计 = 0;

static int 共享扩容(void) {
    int 新容量 = 共享槽容量 == 0 ? 64 : 共享槽容量 * 2;
    共享头** 新表 = (共享头**)realloc(共享槽表, (size_t)新容量 * sizeof(共享头*));
    if (!新表) return 0;
    for (int i = 共享槽容量; i < 新容量; i++) 新表[i] = NULL;
    共享槽表 = 新表;
    共享槽容量 = 新容量;
    return 1;
}

static 共享头* 共享取头(int 句柄) {
    if (句柄 <= 0 || 句柄 > 共享槽最高) return NULL;
    return 共享槽表[句柄 - 1];
}

int 共享新建(int 元素数量) {
    if (元素数量 < 1) 元素数量 = 1;
    if (共享槽容量 == 0 && !共享扩容()) return 0;
    共享头* 头 = (共享头*)calloc(1, sizeof(共享头) + (size_t)元素数量 * sizeof(int));
    if (!头) return 0;
    头->引用计数 = 1;
    头->元素数量 = 元素数量;
    int 槽 = -1;
    for (int i = 0; i < 共享槽最高; i++) {
        if (!共享槽表[i]) { 槽 = i; break; }
    }
    if (槽 < 0) {
        if (共享槽最高 >= 共享槽容量 && !共享扩容()) { free(头); return 0; }
        槽 = 共享槽最高;
        共享槽最高++;
    }
    共享槽表[槽] = 头;
    共享存活++;
    共享分配累计++;
    return 槽 + 1;   /* 0 保留为「无效句柄」 */
}

int 共享保留(int 句柄) {
    共享头* 头 = 共享取头(句柄);
    if (!头) return 0;
    头->引用计数++;
    return 句柄;   /* 返回句柄本身，便于 `(甲)共享保留 = 乙` 直接得到可用句柄 */
}

int 共享引用数(int 句柄) {
    共享头* 头 = 共享取头(句柄);
    return 头 ? 头->引用计数 : 0;
}

int 共享容量(int 句柄) {
    共享头* 头 = 共享取头(句柄);
    return 头 ? 头->元素数量 : 0;
}

int 共享释放(int 句柄) {
    共享头* 头 = 共享取头(句柄);
    if (!头) return 0;
    if (头->引用计数 > 1) {
        头->引用计数--;
        return 头->引用计数;
    }
    共享槽表[句柄 - 1] = NULL;
    free(头);
    共享存活--;
    共享释放累计++;
    return 0;
}

int 共享取整数(int 句柄, int 序号) {
    共享头* 头 = 共享取头(句柄);
    if (!头 || 序号 < 0 || 序号 >= 头->元素数量) return 0;
    return ((int*)(头 + 1))[序号];
}

int 共享存整数(int 句柄, int 序号, int 值) {
    共享头* 头 = 共享取头(句柄);
    if (!头 || 序号 < 0 || 序号 >= 头->元素数量) return 0;
    ((int*)(头 + 1))[序号] = 值;
    return 1;
}

int 共享存活数() {
    return 共享存活;
}

int 共享总分配数() {
    return 共享分配累计;
}

int 共享总释放数() {
    return 共享释放累计;
}

/* ============================================================================
 * 阶段 10 G11：并发原语（互斥锁 / 信号量 / 原子整数 / 并行计算内核）
 * ----------------------------------------------------------------------------
 * 设计要点：
 *  1. 句柄为「整数32 槽号」（0 = 无效），与阶段3B ARC 同风格；槽表按需扩容、空槽复用；
 *  2. 互斥锁 / 信号量用 Windows 内核对象，原子整数用 Interlocked —— 均为**真实线程安全**；
 *  3. 协程与调度器仍为协作式单线程（8AI/8AJ）：语言层共享可变状态默认无数据竞争；
 *     需要跨线程共享时显式取用本组原语，即「数据竞争防护」入口；
 *  4. 并行内核仅在工作线程内读只读数组并归约，不调用任何 GC / ARC 入口；
 *     线程数 ≤ 1 时退化为串行，结果与线程数无关（确定性）。
 * ========================================================================== */
enum { 同步无效 = 0, 同步锁 = 1, 同步信号量 = 2, 同步原子 = 3 };

typedef struct {
    int 种类;
    void* 实体;
} 同步槽;

static 同步槽* 同步槽表 = NULL;
static int 同步槽容量 = 0;
static int 同步槽最高 = 0;
static int 同步存活 = 0;

static int 同步扩容(void) {
    int 新容量 = 同步槽容量 == 0 ? 16 : 同步槽容量 * 2;
    同步槽* 新表 = (同步槽*)realloc(同步槽表, (size_t)新容量 * sizeof(同步槽));
    if (!新表) return 0;
    for (int i = 同步槽容量; i < 新容量; i++) {
        新表[i].种类 = 同步无效;
        新表[i].实体 = NULL;
    }
    同步槽表 = 新表;
    同步槽容量 = 新容量;
    return 1;
}

static int 同步登记(int 种类, void* 实体) {
    if (同步槽容量 == 0 && !同步扩容()) return 0;
    int 槽 = -1;
    for (int i = 0; i < 同步槽最高; i++) {
        if (同步槽表[i].种类 == 同步无效) { 槽 = i; break; }
    }
    if (槽 < 0) {
        if (同步槽最高 >= 同步槽容量 && !同步扩容()) return 0;
        槽 = 同步槽最高;
        同步槽最高++;
    }
    同步槽表[槽].种类 = 种类;
    同步槽表[槽].实体 = 实体;
    同步存活++;
    return 槽 + 1;   /* 0 保留为「无效句柄」 */
}

static void* 同步取(int 句柄, int 种类) {
    if (句柄 <= 0 || 句柄 > 同步槽最高) return NULL;
    if (同步槽表[句柄 - 1].种类 != 种类) return NULL;
    return 同步槽表[句柄 - 1].实体;
}

static void 同步注销(int 句柄) {
    同步槽表[句柄 - 1].种类 = 同步无效;
    同步槽表[句柄 - 1].实体 = NULL;
    同步存活--;
}

/* ---- 互斥锁（临界区；持有计数支持同线程重入） ---- */
typedef struct {
    CRITICAL_SECTION 临界区;
    int 持有;
    int 争用;
} 锁实体;

int 创建互斥锁() {
    锁实体* 锁 = (锁实体*)calloc(1, sizeof(锁实体));
    if (!锁) return 0;
    InitializeCriticalSection(&锁->临界区);
    int 句柄 = 同步登记(同步锁, 锁);
    if (!句柄) {
        DeleteCriticalSection(&锁->临界区);
        free(锁);
        return 0;
    }
    return 句柄;
}

int 互斥锁加锁(int 句柄) {
    锁实体* 锁 = (锁实体*)同步取(句柄, 同步锁);
    if (!锁) return -1;
    if (锁->持有 > 0) 锁->争用++;
    EnterCriticalSection(&锁->临界区);
    锁->持有++;
    return 0;
}

int 互斥锁解锁(int 句柄) {
    锁实体* 锁 = (锁实体*)同步取(句柄, 同步锁);
    if (!锁 || 锁->持有 <= 0) return -1;
    锁->持有--;
    LeaveCriticalSection(&锁->临界区);
    return 0;
}

int 互斥锁是否锁定(int 句柄) {
    锁实体* 锁 = (锁实体*)同步取(句柄, 同步锁);
    return (锁 && 锁->持有 > 0) ? 1 : 0;
}

int 互斥锁争用次数(int 句柄) {
    锁实体* 锁 = (锁实体*)同步取(句柄, 同步锁);
    return 锁 ? 锁->争用 : 0;
}

int 销毁互斥锁(int 句柄) {
    锁实体* 锁 = (锁实体*)同步取(句柄, 同步锁);
    if (!锁) return -1;
    同步注销(句柄);
    DeleteCriticalSection(&锁->临界区);
    free(锁);
    return 0;
}

/* ---- 信号量（内核信号量；当前值为镜像计数，便于语言层观测） ---- */
typedef struct {
    HANDLE 内核;
    volatile LONG 当前值;
    int 等待次数;
    int 释放次数;
} 信号量实体;

int 创建信号量(int 初值) {
    if (初值 < 0) 初值 = 0;
    信号量实体* 量 = (信号量实体*)calloc(1, sizeof(信号量实体));
    if (!量) return 0;
    量->内核 = CreateSemaphore(NULL, (LONG)初值, 0x7FFFFFFF, NULL);
    if (!量->内核) { free(量); return 0; }
    量->当前值 = (LONG)初值;
    int 句柄 = 同步登记(同步信号量, 量);
    if (!句柄) {
        CloseHandle(量->内核);
        free(量);
        return 0;
    }
    return 句柄;
}

int 信号量等待(int 句柄) {
    信号量实体* 量 = (信号量实体*)同步取(句柄, 同步信号量);
    if (!量) return -1;
    if (WaitForSingleObject(量->内核, INFINITE) != WAIT_OBJECT_0) return -1;
    InterlockedDecrement(&量->当前值);
    量->等待次数++;
    return 0;
}

int 信号量释放(int 句柄) {
    信号量实体* 量 = (信号量实体*)同步取(句柄, 同步信号量);
    if (!量) return -1;
    if (!ReleaseSemaphore(量->内核, 1, NULL)) return -1;
    InterlockedIncrement(&量->当前值);
    量->释放次数++;
    return (int)InterlockedCompareExchange(&量->当前值, 0, 0);
}

int 信号量当前值(int 句柄) {
    信号量实体* 量 = (信号量实体*)同步取(句柄, 同步信号量);
    if (!量) return 0;
    return (int)InterlockedCompareExchange(&量->当前值, 0, 0);
}

int 销毁信号量(int 句柄) {
    信号量实体* 量 = (信号量实体*)同步取(句柄, 同步信号量);
    if (!量) return -1;
    同步注销(句柄);
    CloseHandle(量->内核);
    free(量);
    return 0;
}

/* ---- 原子整数（Interlocked；读写/加减均为单指令原子操作） ---- */
typedef struct {
    volatile LONG 值;
} 原子实体;

int 创建原子整数(int 初值) {
    原子实体* 原子 = (原子实体*)calloc(1, sizeof(原子实体));
    if (!原子) return 0;
    原子->值 = (LONG)初值;
    int 句柄 = 同步登记(同步原子, 原子);
    if (!句柄) { free(原子); return 0; }
    return 句柄;
}

int 原子读(int 句柄) {
    原子实体* 原子 = (原子实体*)同步取(句柄, 同步原子);
    if (!原子) return 0;
    return (int)InterlockedCompareExchange(&原子->值, 0, 0);
}

int 原子写(int 句柄, int 值) {
    原子实体* 原子 = (原子实体*)同步取(句柄, 同步原子);
    if (!原子) return 0;
    return (int)InterlockedExchange(&原子->值, (LONG)值);   /* 返回旧值 */
}

int 原子加(int 句柄, int 值) {
    原子实体* 原子 = (原子实体*)同步取(句柄, 同步原子);
    if (!原子) return 0;
    return (int)InterlockedExchangeAdd(&原子->值, (LONG)值) + 值;   /* 返回新值 */
}

int 原子减(int 句柄, int 值) {
    原子实体* 原子 = (原子实体*)同步取(句柄, 同步原子);
    if (!原子) return 0;
    return (int)InterlockedExchangeAdd(&原子->值, -(LONG)值) - 值;   /* 返回新值 */
}

int 销毁原子整数(int 句柄) {
    原子实体* 原子 = (原子实体*)同步取(句柄, 同步原子);
    if (!原子) return -1;
    同步注销(句柄);
    free(原子);
    return 0;
}

int 同步存活数() {
    return 同步存活;
}

/* ---- 并行计算内核（真实多线程） ----
 * 模式：0 = 求和（只读）、1 = 计数正（只读）、2 = 填充（写）、3 = 变换加（写）。
 * 写模式下各线程只触碰**互不相交的区间**，故共享数组写入无数据竞争；
 * 返回值为「本段处理过的元素个数」，与线程数无关（确定性）。 */
typedef struct {
    int* 数据;
    int 起;
    int 止;
    int 模式;
    int 参数;   /* 填充值 / 增量 */
    int 结果;
} 并行片段;

static void 并行片段主体(void* 参数) {
    并行片段* 片段 = (并行片段*)参数;
    int 局部 = 0;
    for (int i = 片段->起; i < 片段->止; i++) {
        int 值 = 片段->数据[i];
        if (片段->模式 == 0) 局部 += 值;
        else if (片段->模式 == 1) { if (值 > 0) 局部++; }
        else if (片段->模式 == 2) { 片段->数据[i] = 片段->参数; 局部++; }
        else { 片段->数据[i] = 值 + 片段->参数; 局部++; }
    }
    片段->结果 = 局部;
}

#ifdef _WIN32
static DWORD WINAPI 并行片段执行(LPVOID 参数) {
    并行片段主体(参数);
    return 0;
}
#endif

static int 并行执行(void* 句柄, int 元素数, int 线程数, int 模式, int 参数) {
    动态数组* 数组 = (动态数组*)句柄;
    if (!数组 || 元素数 <= 0) return 0;
    if (元素数 > 数组->数组大小) 元素数 = 数组->数组大小;
    if (线程数 < 1) 线程数 = 1;
    if (线程数 > 元素数) 线程数 = 元素数;
    if (线程数 > 64) 线程数 = 64;   /* 栈上片段/线程数组上限 */
    if (线程数 <= 1) {
        并行片段 单;
        单.数据 = 数组->数据;
        单.起 = 0;
        单.止 = 元素数;
        单.模式 = 模式;
        单.参数 = 参数;
        单.结果 = 0;
        并行片段主体(&单);
        return 单.结果;
    }
#ifdef _WIN32
    并行片段 片段[64];
    HANDLE 线程[64];
    for (int t = 0; t < 64; t++) 线程[t] = NULL;
    int 每段 = 元素数 / 线程数;
    int 余数 = 元素数 % 线程数;
    int 起 = 0;
    for (int t = 0; t < 线程数; t++) {
        int 本段 = 每段 + (t < 余数 ? 1 : 0);
        片段[t].数据 = 数组->数据;
        片段[t].起 = 起;
        片段[t].止 = 起 + 本段;
        片段[t].模式 = 模式;
        片段[t].参数 = 参数;
        片段[t].结果 = 0;
        起 += 本段;
        线程[t] = CreateThread(NULL, 0, 并行片段执行, &片段[t], 0, NULL);
        if (!线程[t]) 并行片段主体(&片段[t]);   /* 线程创建失败：本段就地串行 */
    }
    for (int t = 0; t < 线程数; t++) {
        if (线程[t]) {
            WaitForSingleObject(线程[t], INFINITE);
            CloseHandle(线程[t]);
        }
    }
    int 结果 = 0;
    for (int t = 0; t < 线程数; t++) 结果 += 片段[t].结果;
    return 结果;
#else
    并行片段 单;
    单.数据 = 数组->数据;
    单.起 = 0;
    单.止 = 元素数;
    单.模式 = 模式;
    单.参数 = 参数;
    单.结果 = 0;
    并行片段主体(&单);
    return 单.结果;
#endif
}

int 并行求和(void* 句柄, int 元素数, int 线程数) {
    return 并行执行(句柄, 元素数, 线程数, 0, 0);
}

int 并行计数正(void* 句柄, int 元素数, int 线程数) {
    return 并行执行(句柄, 元素数, 线程数, 1, 0);
}

// 阶段10 G11-B：写共享数组的并行（分区写入，互不相交 ⇒ 无数据竞争）。
int 并行填充(void* 句柄, int 元素数, int 线程数, int 值) {
    return 并行执行(句柄, 元素数, 线程数, 2, 值);
}

int 并行变换加(void* 句柄, int 元素数, int 线程数, int 增量) {
    return 并行执行(句柄, 元素数, 线程数, 3, 增量);
}

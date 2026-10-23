// ============================================
// 阶段5G 交叉运行时最小子集（无 libc、无操作系统导入）
// 用途：aarch64-unknown-linux-gnu 静态可执行文件 与 aarch64-w64-windows-gnu 动态库
//      在缺少目标 sysroot / 交叉 libc 的前提下，仅靠 llc + ld.lld 完成链接。
// 约束：只在链接期由 clang --target=<三元组> -ffreestanding -nostdlibinc 编译，
//      不进入宿主构建，也不参与 .o 预编译。
// 覆盖：自举链无条件发射的三个辅助函数（辅助子串/辅助拼接/辅助转字符串）所依赖的
//      malloc/memcpy/strlen/sprintf/strdup/字符位置到字节位置，以及入口所需
//      设置参数 / _start（ELF）/ DllMainCRTStartup（PE）。
// 限制：堆为 1 MiB 静态区间（不向操作系统申请）；free 为空操作；
//      sprintf 仅支持 %d/%ld/%s/%c/%%；不含文件、输入输出、动态数组等运行时。
// ============================================

static unsigned char 交叉堆[1048576];
static unsigned long 堆已用 = 0;

// 递增式分配：按 16 字节对齐从静态堆切分；耗尽返回 0
void* malloc(unsigned long 字节数) {
    unsigned long 对齐后 = (字节数 + 15ul) & ~15ul;
    void* 结果;
    if (堆已用 + 对齐后 > sizeof(交叉堆)) return 0;
    结果 = 交叉堆 + 堆已用;
    堆已用 += 对齐后;
    return 结果;
}

// 静态堆不支持回收
void free(void* 指针) { (void)指针; }

void* memcpy(void* 目标, const void* 源, unsigned long 字节数) {
    unsigned char* 出 = (unsigned char*)目标;
    const unsigned char* 入 = (const unsigned char*)源;
    unsigned long 序;
    for (序 = 0; 序 < 字节数; 序++) 出[序] = 入[序];
    return 目标;
}

void* memmove(void* 目标, const void* 源, unsigned long 字节数) {
    unsigned char* 出 = (unsigned char*)目标;
    const unsigned char* 入 = (const unsigned char*)源;
    unsigned long 序;
    if (出 < 入) { for (序 = 0; 序 < 字节数; 序++) 出[序] = 入[序]; }
    else { for (序 = 字节数; 序 > 0; 序--) 出[序 - 1] = 入[序 - 1]; }
    return 目标;
}

void* memset(void* 目标, int 值, unsigned long 字节数) {
    unsigned char* 出 = (unsigned char*)目标;
    unsigned long 序;
    for (序 = 0; 序 < 字节数; 序++) 出[序] = (unsigned char)值;
    return 目标;
}

unsigned long strlen(const char* 文本) {
    unsigned long 长 = 0;
    while (文本[长] != 0) 长++;
    return 长;
}

int strcmp(const char* 甲, const char* 乙) {
    while (*甲 != 0 && *甲 == *乙) { 甲++; 乙++; }
    return (int)(unsigned char)*甲 - (int)(unsigned char)*乙;
}

char* strdup(const char* 文本) {
    unsigned long 长 = strlen(文本) + 1;
    char* 结果 = (char*)malloc(长);
    if (结果 == 0) return 0;
    memcpy(结果, 文本, 长);
    return 结果;
}

int atoi(const char* 文本) {
    int 符号 = 1, 值 = 0;
    if (*文本 == '-') { 符号 = -1; 文本++; }
    while (*文本 >= '0' && *文本 <= '9') { 值 = 值 * 10 + (*文本 - '0'); 文本++; }
    return 符号 * 值;
}

// 极简格式化：%d / %ld / %s / %c / %%，其余按原样输出（%x 等暂不支持）
int sprintf(char* 缓冲, const char* 格式, ...) {
    __builtin_va_list 参数;
    char* 出 = 缓冲;
    __builtin_va_start(参数, 格式);
    while (*格式 != 0) {
        if (*格式 != '%') { *出++ = *格式++; continue; }
        格式++;
        if (*格式 == 'd') {
            int 值 = __builtin_va_arg(参数, int);
            char 临时[16]; int 位数 = 0; int 负 = 0;
            unsigned int 无符号值;
            if (值 < 0) { 负 = 1; 无符号值 = (unsigned int)(-值); } else 无符号值 = (unsigned int)值;
            do { 临时[位数++] = (char)('0' + 无符号值 % 10); 无符号值 /= 10; } while (无符号值 != 0);
            if (负) *出++ = '-';
            while (位数 > 0) *出++ = 临时[--位数];
            格式++;
        } else if (*格式 == 'l' && *(格式 + 1) == 'd') {
            long 值 = __builtin_va_arg(参数, long);
            char 临时[24]; int 位数 = 0; int 负 = 0;
            unsigned long 无符号值;
            if (值 < 0) { 负 = 1; 无符号值 = (unsigned long)(-值); } else 无符号值 = (unsigned long)值;
            do { 临时[位数++] = (char)('0' + 无符号值 % 10); 无符号值 /= 10; } while (无符号值 != 0);
            if (负) *出++ = '-';
            while (位数 > 0) *出++ = 临时[--位数];
            格式 += 2;
        } else if (*格式 == 's') {
            const char* 串 = __builtin_va_arg(参数, const char*);
            while (*串 != 0) *出++ = *串++;
            格式++;
        } else if (*格式 == 'c') {
            *出++ = (char)__builtin_va_arg(参数, int);
            格式++;
        } else if (*格式 == '%') {
            *出++ = '%'; 格式++;
        } else if (*格式 == 0) {
            break;
        } else {
            *出++ = '%'; *出++ = *格式++;
        }
    }
    *出 = 0;
    __builtin_va_end(参数);
    return (int)(出 - 缓冲);
}

// 字符下标（按码点）→ 字节下标：跳过 UTF-8 续字节（与宿主运行时同名同语义）
int 字符位置到字节位置(const char* 文本, int 字符位置) {
    int 字节 = 0, 字符序 = 0;
    while (字符序 < 字符位置 && 文本[字节] != 0) {
        字节++;
        while ((文本[字节] & 0xC0) == 0x80) 字节++;
        字符序++;
    }
    return 字节;
}

// 入口占位：交叉子集不传参数（ELF _start 以 0/0 调 main）
void 设置参数(int 参数数, char** 参数表) { (void)参数数; (void)参数表; }

// PE 动态库入口：ld.lld -m arm64pe --dll 要求该符号
int DllMainCRTStartup(void* 模块, int 原因, void* 保留) {
    (void)模块; (void)原因; (void)保留; return 1;
}

int main(int, char**);

// aarch64 Linux 入口：调用 main 后以 exit(93) 系统调用退出
// （PE 目标不提供 _start：动态库无入口，入口为下方 DllMainCRTStartup；且动态库产物
//   可能不含 main，保留 _start 会引入未定义符号 main 而使链接失败。）
#ifndef _WIN32
__attribute__((noreturn)) void _start(void) {
    int 退出码 = main(0, 0);
    register long x8 __asm__("x8") = 93;
    register long x0 __asm__("x0") = 退出码;
    __asm__ volatile("svc #0" :: "r"(x8), "r"(x0) : "memory");
    __builtin_unreachable();
}
#endif

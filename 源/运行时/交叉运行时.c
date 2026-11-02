// ============================================
// 阶段5G 交叉运行时最小子集（无 libc、无操作系统导入）
// 用途：aarch64-unknown-linux-gnu 静态可执行文件、aarch64-w64-windows-gnu 动态库，
//      以及 wasm32-wasip1 命令模块；在缺少目标 sysroot / 交叉 libc 的前提下，
//      仅靠 llc + ld.lld / wasm-ld 完成链接。
// 约束：只在链接期由 clang --target=<三元组> -ffreestanding -nostdlibinc 编译，
//      不进入宿主构建，也不参与 .o 预编译。
// 覆盖：自举链无条件发射的辅助函数（辅助子串/辅助拼接/辅助转字符串）所依赖的
//      malloc/memcpy/memmove/memset/strlen/sprintf/strdup/atoi、
//      字符位置换算（字符位置到字节位置/获取字符数/字符码/字符序列）、
//      胖字符串（字符串对象）系列、printf，以及入口所需的
//      设置参数 / _start（ELF / WASI）/ DllMainCRTStartup（PE）。
// 限制：堆为 1 MiB 静态区间（不向操作系统申请）；free 为空操作；
//      sprintf/printf 支持 %d/%ld/%lld/%s/%c/%g/%%，无宽度、精度与无符号修饰；
//      输出仅走到 stdout（WASI fd_write / aarch64 write 系统调用 / 其他平台丢弃）；
//      不含文件、输入、动态数组等运行时。
// ============================================

static unsigned char 交叉堆[1048576];
static unsigned long long 堆已用 = 0;

// 递增式分配：按 16 字节对齐从静态堆切分；耗尽返回 0
// 长度参数用 unsigned long long：IR 侧统一声明为 i64（wasm32 的 long 只有 32 位）
void* malloc(unsigned long long 字节数) {
    unsigned long long 对齐后 = (字节数 + 15ull) & ~15ull;
    void* 结果;
    if (堆已用 + 对齐后 > sizeof(交叉堆)) return 0;
    结果 = 交叉堆 + 堆已用;
    堆已用 += 对齐后;
    return 结果;
}

// 静态堆不支持回收
void free(void* 指针) { (void)指针; }

void* memcpy(void* 目标, const void* 源, unsigned long long 字节数) {
    unsigned char* 出 = (unsigned char*)目标;
    const unsigned char* 入 = (const unsigned char*)源;
    unsigned long long 序;
    for (序 = 0; 序 < 字节数; 序++) 出[序] = 入[序];
    return 目标;
}

void* memmove(void* 目标, const void* 源, unsigned long long 字节数) {
    unsigned char* 出 = (unsigned char*)目标;
    const unsigned char* 入 = (const unsigned char*)源;
    unsigned long long 序;
    if (出 < 入) { for (序 = 0; 序 < 字节数; 序++) 出[序] = 入[序]; }
    else { for (序 = 字节数; 序 > 0; 序--) 出[序 - 1] = 入[序 - 1]; }
    return 目标;
}

void* memset(void* 目标, int 值, unsigned long long 字节数) {
    unsigned char* 出 = (unsigned char*)目标;
    unsigned long long 序;
    for (序 = 0; 序 < 字节数; 序++) 出[序] = (unsigned char)值;
    return 目标;
}

unsigned long long strlen(const char* 文本) {
    unsigned long long 长 = 0;
    while (文本[长] != 0) 长++;
    return 长;
}

int strcmp(const char* 甲, const char* 乙) {
    while (*甲 != 0 && *甲 == *乙) { 甲++; 乙++; }
    return (int)(unsigned char)*甲 - (int)(unsigned char)*乙;
}

char* strdup(const char* 文本) {
    unsigned long long 长 = strlen(文本) + 1;
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

// ── 输出通道：WASI fd_write / aarch64 Linux write 系统调用 ──
// 无目标 sysroot 时不能依赖 libc 的 stdio；printf 直接写文件描述符 1。
#if defined(__wasm__)
typedef struct { const void* 缓冲; unsigned int 长度; } 交叉IO向量;
__attribute__((import_module("wasi_snapshot_preview1"), import_name("fd_write")))
int __wasi_fd_write(int 描述符, const 交叉IO向量* 向量, int 向量数, unsigned int* 写出);
__attribute__((import_module("wasi_snapshot_preview1"), import_name("proc_exit")))
void __wasi_proc_exit(int 退出码);
static long 交叉写(int 描述符, const void* 缓冲, unsigned long 长度) {
    交叉IO向量 向量;
    unsigned int 写出 = 0;
    向量.缓冲 = 缓冲;
    向量.长度 = (unsigned int)长度;
    if (__wasi_fd_write(描述符, &向量, 1, &写出) != 0) return -1;
    return (long)写出;
}
#elif defined(__aarch64__)
static long 交叉写(int 描述符, const void* 缓冲, unsigned long 长度) {
    register long x8 __asm__("x8") = 64;   // write
    register long x0 __asm__("x0") = 描述符;
    register long x1 __asm__("x1") = (long)缓冲;
    register long x2 __asm__("x2") = (long)长度;
    __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");
    return x0;
}
#else
static long 交叉写(int 描述符, const void* 缓冲, unsigned long 长度) {
    (void)描述符; (void)缓冲; (void)长度; return -1;
}
#endif

static char* 交叉写无符号(char* 出, char* 起, unsigned long 上限, unsigned long long 值) {
    char 临时[24];
    int 位数 = 0;
    do { 临时[位数++] = (char)('0' + (值 % 10)); 值 /= 10; } while (值 != 0);
    while (位数 > 0) {
        char 本字符 = 临时[--位数];
        if ((unsigned long)(出 - 起) < 上限) *出++ = 本字符;
    }
    return 出;
}

// %g 的最简实现：整数部分 + 最多 6 位小数，去掉尾零（不处理指数记法）
static char* 交叉写浮点(char* 出, char* 起, unsigned long 上限, double 值) {
    if (值 != 值) {
        if ((unsigned long)(出 - 起) < 上限) *出++ = 'n';
        if ((unsigned long)(出 - 起) < 上限) *出++ = 'a';
        if ((unsigned long)(出 - 起) < 上限) *出++ = 'n';
        return 出;
    }
    if (值 < 0) { if ((unsigned long)(出 - 起) < 上限) *出++ = '-'; 值 = -值; }
    unsigned long long 整数部分 = (unsigned long long)值;
    出 = 交叉写无符号(出, 起, 上限, 整数部分);
    double 小数 = 值 - (double)整数部分;
    char 临时[6];
    int 位数 = 0;
    int 序;
    for (序 = 0; 序 < 6; 序++) {
        小数 *= 10.0;
        int 数字 = (int)小数;
        if (数字 > 9) 数字 = 9;
        临时[位数++] = (char)('0' + 数字);
        小数 -= 数字;
    }
    while (位数 > 0 && 临时[位数 - 1] == '0') 位数--;
    if (位数 > 0) {
        if ((unsigned long)(出 - 起) < 上限) *出++ = '.';
        for (序 = 0; 序 < 位数; 序++) {
            if ((unsigned long)(出 - 起) < 上限) *出++ = 临时[序];
        }
    }
    return 出;
}

// 极简格式化：%d / %ld / %lld / %s / %c / %g / %%，其余按原样输出（%x 等暂不支持）
static int 交叉格式化(char* 缓冲, unsigned long 上限, const char* 格式, __builtin_va_list 参数) {
    char* 出 = 缓冲;
    char* 起 = 缓冲;
    while (*格式 != 0) {
        if (*格式 != '%') {
            if ((unsigned long)(出 - 起) < 上限) *出++ = *格式;
            格式++;
            continue;
        }
        格式++;
        int 长 = 0;
        if (*格式 == 'l') { 格式++; 长 = 1; if (*格式 == 'l') { 格式++; 长 = 2; } }
        if (*格式 == 'd') {
            long long 值;
            if (长 == 0) 值 = (long long)__builtin_va_arg(参数, int);
            else if (长 == 1) 值 = (long long)__builtin_va_arg(参数, long);
            else 值 = __builtin_va_arg(参数, long long);
            if (值 < 0) {
                if ((unsigned long)(出 - 起) < 上限) *出++ = '-';
                出 = 交叉写无符号(出, 起, 上限, (unsigned long long)(-值));
            } else {
                出 = 交叉写无符号(出, 起, 上限, (unsigned long long)值);
            }
            格式++;
        } else if (*格式 == 's') {
            const char* 串 = __builtin_va_arg(参数, const char*);
            if (串 == 0) 串 = "";
            while (*串 != 0) {
                if ((unsigned long)(出 - 起) < 上限) *出++ = *串;
                串++;
            }
            格式++;
        } else if (*格式 == 'c') {
            if ((unsigned long)(出 - 起) < 上限) *出++ = (char)__builtin_va_arg(参数, int);
            格式++;
        } else if (*格式 == 'g') {
            出 = 交叉写浮点(出, 起, 上限, __builtin_va_arg(参数, double));
            格式++;
        } else if (*格式 == '%') {
            if ((unsigned long)(出 - 起) < 上限) *出++ = '%';
            格式++;
        } else if (*格式 == 0) {
            break;
        } else {
            if ((unsigned long)(出 - 起) < 上限) *出++ = '%';
            if (长 > 0) { if ((unsigned long)(出 - 起) < 上限) *出++ = 'l'; }
            if (长 > 1) { if ((unsigned long)(出 - 起) < 上限) *出++ = 'l'; }
            if ((unsigned long)(出 - 起) < 上限) *出++ = *格式;
            格式++;
        }
    }
    if ((unsigned long)(出 - 起) < 上限) *出 = 0;
    else if (上限 > 0) *(起 + 上限 - 1) = 0;
    return (int)(出 - 起);
}

int sprintf(char* 缓冲, const char* 格式, ...) {
    __builtin_va_list 参数;
    __builtin_va_start(参数, 格式);
    int 长度 = 交叉格式化(缓冲, 0x7FFFFFFFul, 格式, 参数);
    __builtin_va_end(参数);
    return 长度;
}

// 打印入口：格式化到栈缓冲后一次写出（超长截断，不越界）
int printf(const char* 格式, ...) {
    char 缓冲[4096];
    __builtin_va_list 参数;
    __builtin_va_start(参数, 格式);
    int 长度 = 交叉格式化(缓冲, sizeof(缓冲) - 1, 格式, 参数);
    __builtin_va_end(参数);
    if (长度 > 0) 交叉写(1, 缓冲, (unsigned long)长度);
    return 长度;
}

// ── UTF-8 字符操作（与宿主运行时同名同语义，仅保留无缓存的线性扫描路径） ──
static int 字符字节宽(unsigned char 首字节) {
    if (首字节 < 0x80) return 1;
    if ((首字节 & 0xE0) == 0xC0) return 2;
    if ((首字节 & 0xF0) == 0xE0) return 3;
    if ((首字节 & 0xF8) == 0xF0) return 4;
    return 1;
}

int 获取字符数(const char* 文本) {
    int 字符数 = 0, 字节 = 0;
    if (文本 == 0) return 0;
    while (文本[字节] != 0) { 字节 += 字符字节宽((unsigned char)文本[字节]); 字符数++; }
    return 字符数;
}

// 字符下标（按码点）→ 字节下标：跳过 UTF-8 续字节（与宿主运行时同名同语义）
int 字符位置到字节位置(const char* 文本, int 字符位置) {
    int 字节 = 0, 字符序 = 0;
    if (文本 == 0) return 0;
    if (字符位置 < 0) 字符位置 = 0;
    while (字符序 < 字符位置 && 文本[字节] != 0) {
        字节 += 字符字节宽((unsigned char)文本[字节]);
        字符序++;
    }
    return 字节;
}

int 字符码(const char* 文本, int 位置) {
    if (文本 == 0) return -1;
    int 字节 = 字符位置到字节位置(文本, 位置 < 0 ? 0 : 位置);
    unsigned char 甲 = (unsigned char)文本[字节];
    if (甲 == 0) return -1;
    if (甲 < 0x80) return 甲;
    if ((甲 & 0xE0) == 0xC0) return ((甲 & 0x1F) << 6) | (文本[字节 + 1] & 0x3F);
    if ((甲 & 0xF0) == 0xE0) return ((甲 & 0x0F) << 12) | ((文本[字节 + 1] & 0x3F) << 6) | (文本[字节 + 2] & 0x3F);
    if ((甲 & 0xF8) == 0xF0) return ((甲 & 0x07) << 18) | ((文本[字节 + 1] & 0x3F) << 12) | ((文本[字节 + 2] & 0x3F) << 6) | (文本[字节 + 3] & 0x3F);
    return 甲;
}

// 从码点创建 UTF-8 字符串（静态缓冲，与宿主运行时一致：多次调用会互相覆盖）
const char* 字符序列(int 码点) {
    static char 缓冲区[5];
    if (码点 < 0x80) {
        缓冲区[0] = (char)码点; 缓冲区[1] = 0;
    } else if (码点 < 0x800) {
        缓冲区[0] = (char)(0xC0 | (码点 >> 6));
        缓冲区[1] = (char)(0x80 | (码点 & 0x3F));
        缓冲区[2] = 0;
    } else if (码点 < 0x10000) {
        缓冲区[0] = (char)(0xE0 | (码点 >> 12));
        缓冲区[1] = (char)(0x80 | ((码点 >> 6) & 0x3F));
        缓冲区[2] = (char)(0x80 | (码点 & 0x3F));
        缓冲区[3] = 0;
    } else {
        缓冲区[0] = (char)(0xF0 | (码点 >> 18));
        缓冲区[1] = (char)(0x80 | ((码点 >> 12) & 0x3F));
        缓冲区[2] = (char)(0x80 | ((码点 >> 6) & 0x3F));
        缓冲区[3] = (char)(0x80 | (码点 & 0x3F));
        缓冲区[4] = 0;
    }
    return 缓冲区;
}

// ── 字符串胖指针（与宿主运行时同布局：数据 + 字符数） ──
typedef struct {
    const char* 数据;
    int 长度;
} 字符串对象;

字符串对象* 胖字符串包装(const char* 数据) {
    const char* 源 = (数据 != 0) ? 数据 : "";
    unsigned long long 字节数 = strlen(源);
    char* 拷贝 = (char*)malloc(字节数 + 1);
    if (拷贝 == 0) return 0;
    memcpy(拷贝, 源, 字节数 + 1);
    字符串对象* 结果 = (字符串对象*)malloc(sizeof(字符串对象));
    if (结果 == 0) return 0;
    结果->数据 = 拷贝;
    结果->长度 = 获取字符数(拷贝);
    return 结果;
}

const char* 胖字符串数据(字符串对象* 对象) {
    return (对象 != 0 && 对象->数据 != 0) ? 对象->数据 : "";
}

int 胖字符串长度(字符串对象* 对象) { return (对象 != 0) ? 对象->长度 : 0; }

int 胖字符串字符数(字符串对象* 对象) { return 胖字符串长度(对象); }

int 胖字符串比较(字符串对象* 甲, 字符串对象* 乙) {
    return strcmp(胖字符串数据(甲), 胖字符串数据(乙));
}

int 胖字符串字符码(字符串对象* 对象, int 索引) {
    return 字符码(胖字符串数据(对象), 索引);
}

字符串对象* 胖字符串拼接(字符串对象* 甲, 字符串对象* 乙) {
    const char* 左 = 胖字符串数据(甲);
    const char* 右 = 胖字符串数据(乙);
    unsigned long long 左长 = strlen(左), 右长 = strlen(右);
    char* 缓冲 = (char*)malloc(左长 + 右长 + 1);
    if (缓冲 == 0) return 0;
    memcpy(缓冲, 左, 左长);
    memcpy(缓冲 + 左长, 右, 右长);
    缓冲[左长 + 右长] = 0;
    字符串对象* 结果 = (字符串对象*)malloc(sizeof(字符串对象));
    if (结果 == 0) return 0;
    结果->数据 = 缓冲;
    结果->长度 = 获取字符数(缓冲);
    return 结果;
}

字符串对象* 胖字符串子串(字符串对象* 对象, int 起, int 长) {
    const char* 源 = 胖字符串数据(对象);
    int 字节起 = 字符位置到字节位置(源, 起);
    int 字节止 = 字符位置到字节位置(源, 起 + 长);
    int 字节长 = 字节止 - 字节起;
    char* 缓冲;
    字符串对象* 结果;
    if (字节长 < 0) 字节长 = 0;
    缓冲 = (char*)malloc((unsigned long)字节长 + 1);
    if (缓冲 == 0) return 0;
    memcpy(缓冲, 源 + 字节起, (unsigned long)字节长);
    缓冲[字节长] = 0;
    结果 = (字符串对象*)malloc(sizeof(字符串对象));
    if (结果 == 0) return 0;
    结果->数据 = 缓冲;
    结果->长度 = 获取字符数(缓冲);
    return 结果;
}

// 越界按空串处理（交叉子集不引入中止运行时，保证内存安全）
字符串对象* 胖字符串取字符(字符串对象* 对象, int 索引) {
    int 字符数 = 胖字符串长度(对象);
    if (索引 < 0 || 索引 >= 字符数) return 胖字符串子串(对象, 0, 0);
    return 胖字符串子串(对象, 索引, 1);
}

int 胖字符串字符码检查(字符串对象* 对象, int 索引) {
    int 字符数 = 胖字符串长度(对象);
    if (索引 < 0 || 索引 >= 字符数) return -1;
    return 胖字符串字符码(对象, 索引);
}

int 胖字符串转整数(字符串对象* 对象) { return atoi(胖字符串数据(对象)); }

// 入口占位：交叉子集不传参数（WASI/ELF _start 以 0/0 调 main）
void 设置参数(int 参数数, char** 参数表) { (void)参数数; (void)参数表; }

// PE 动态库入口：ld.lld -m arm64pe --dll 要求该符号
int DllMainCRTStartup(void* 模块, int 原因, void* 保留) {
    (void)模块; (void)原因; (void)保留; return 1;
}

int main(int, char**);

// 入口：WASI 命令模块 / aarch64 Linux 静态可执行。
// （PE 目标不提供 _start：动态库无入口，入口为上方 DllMainCRTStartup；且动态库产物
//   可能不含 main，保留 _start 会引入未定义符号 main 而使链接失败。）
#if defined(__wasm__)
__attribute__((noreturn)) void _start(void) {
    int 退出码 = main(0, 0);
    __wasi_proc_exit(退出码);
    __builtin_unreachable();
}
#elif !defined(_WIN32)
__attribute__((noreturn)) void _start(void) {
    int 退出码 = main(0, 0);
    register long x8 __asm__("x8") = 93;
    register long x0 __asm__("x0") = 退出码;
    __asm__ volatile("svc #0" :: "r"(x8), "r"(x0) : "memory");
    __builtin_unreachable();
}
#endif

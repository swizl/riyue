// 日月 ↔ clang 联合编译示例的 C 侧（阶段10 续43）。
// 由 run_tests.sh 用本地化 clang 编成仓库根目录的 `_cinterop.dll`；
// 日月侧在 示例/C互操作测试.心 里用 `外部 函数` 按符号声明调用。
// Linux 宿主同样产出名为 `_cinterop.dll` 的 ELF 共享库，让同一份 .心 双平台可跑。
#ifdef _WIN32
#define 日月导出 __declspec(dllexport)
#else
#define 日月导出 __attribute__((visibility("default")))
#endif

日月导出 int riyue_add(int 甲, int 乙) { return 甲 + 乙; }

日月导出 long long riyue_mul64(long long 甲, long long 乙) { return 甲 * 乙; }

日月导出 double riyue_scale(double 值) { return 值 * 1.5 + 0.25; }

// 阶段10 续51：结构体按指针传递（心侧 `结构体` 变量本身就是指向堆上布局的指针，
// 直接传给 `指针` 形参即可；字段顺序/自然对齐与 C 结构体一致）。
typedef struct {
    int 横;
    int 纵;
} 演示点;

日月导出 int riyue_sum_point(const 演示点 *点) { return 点->横 * 100 + 点->纵; }

// 阶段10 续52：结构体按值传递（C 侧形参是一份副本）——验证聚合 ABI。
// 日月侧改为「链接期直调」C 符号（8 字节结构体按 i64 传一个整数寄存器），
// 不再是运行期 `外部调用N` 的固定标量转发。
日月导出 int riyue_point_weight(演示点 点) { return 点.横 * 10 + 点.纵; }

// 阶段10 续53：结构体按值返回（小聚合体经 RAX 返回）——日月侧同样链接期直调，
// 把整数返回值写回新分配的结构体实例。
日月导出 演示点 riyue_point_make(int 横, int 纵) {
    演示点 点;
    点.横 = 横;
    点.纵 = 纵;
    return 点;
}

// 阶段10 续54：大结构体（>16 字节）按值返回——Win64 与 SysV 都改为「隐藏指针」
// 作为首个整型参数（sret），两平台一致，故日月侧仍可链接期直调。
typedef struct {
    int 横;
    int 纵;
    int 甲;
    int 乙;
    int 丙;
    int 丁;
} 演示大点;

日月导出 演示大点 riyue_big_make(int 横, int 纵, int 基) {
    演示大点 点;
    点.横 = 横;
    点.纵 = 纵;
    点.甲 = 基;
    点.乙 = 基 + 1;
    点.丙 = 基 + 2;
    点.丁 = 基 + 3;
    return 点;
}

// 校验 sret 写入的缓冲区：按指针读回六个字段求和（运行期 dlopen 路径）。
日月导出 int riyue_big_sum(const 演示大点 *点) {
    return 点->横 + 点->纵 + 点->甲 + 点->乙 + 点->丙 + 点->丁;
}

// 阶段10 续56：9–16 字节聚合体按值返回——Win64 只用 RAX 传 1/2/4/8 字节、其余走隐藏指针，
// SysV 则按 RAX:RDX 寄存器对返回；两平台规则不同，日月侧按目标三元组分派（此前为编译期报错）。
typedef struct {
    int 横;
    int 纵;
    int 丙;
} 演示中点;

日月导出 演示中点 riyue_mid_make(int 横, int 纵, int 丙) {
    演示中点 点;
    点.横 = 横;
    点.纵 = 纵;
    点.丙 = 丙;
    return 点;
}

日月导出 int riyue_mid_sum(const 演示中点 *点) {
    return 点->横 + 点->纵 + 点->丙;
}

// 阶段10 续57：9–16 字节聚合体按值**传参**——Win64 按引用（指向副本的指针）、
// SysV 占两个八字节整型寄存器；日月侧同由目标平台分派。
日月导出 int riyue_mid_weight(演示中点 点) {
    return 点.横 * 100 + 点.纵 * 10 + 点.丙;
}

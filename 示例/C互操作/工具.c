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

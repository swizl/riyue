// 阶段10 续50：C 头文件导入自测夹具。
// 由 工具/头文件导入.心 解析后生成 `.心` 声明（枚举常量 + 外部函数），
// 生成结果供 示例/C互操作/头文件用例.心 导入使用；函数实现由 示例/C互操作/工具.c
// 编成的 `_cinterop.dll` 在运行期提供。
#ifndef 演示库_H
#define 演示库_H

#define DEMO_VERSION 3
#define DEMO_MAX 100
#define DEMO_FLAG   0x10

enum DemoState {
    DEMO_READY = 0,
    DEMO_BUSY,
    DEMO_STOPPED = 9
};

/* 三种 ABI 基调：整数 / 整数64 / 浮点 */
int riyue_add(int 甲, int 乙);
long long riyue_mul64(long long 甲, long long 乙);
double riyue_scale(double 值);

// 应被跳过的项：参数超过 4 个
int demo_too_many(int a, int b, int c, int d, int e);

#endif

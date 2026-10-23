// K7 故障注入自测：验证 `读取整文件` 在**真实读错误**下返回 NULL，而不是把半截数据
// 当完整内容返回（正常路径由 示例/文件操作测试.心 等覆盖）。
//
// 为何在 C 层做：Windows 上 fopen 目录直接失败（进不到 fread），而“读到一半 IO 故障”
// 无法用 心 程序构造。故把运行时实现整份包含进本编译单元，直接对内部静态函数
// `从流读取整文件` 注入故障——关闭底层文件描述符后，fread 必然返回 0 且置 ferror，
// 正好命中 K7 的分支。
#include <io.h>
#include <stdio.h>
#include <string.h>

#include "../源/运行时/运行时辅助.c"

static int 断言(int 条件, const char* 名称) {
    printf("%s: %s\n", 条件 ? "通过" : "失败", 名称);
    return 条件 ? 0 : 1;
}

int main(void) {
    int 失败 = 0;
    const char* 路径 = "_k7_自测.txt";
    FILE* 写句柄 = fopen(路径, "wb");
    if (!写句柄) { printf("失败: 无法创建自测文件\n"); return 2; }
    fputs("hello", 写句柄);
    fclose(写句柄);

    // 1) 正常路径：完整读出（对照组，确认自测环境本身可用）
    char* 正常 = 读取整文件(路径);
    失败 += 断言(正常 != NULL && strcmp(正常, "hello") == 0, "正常读取整文件");
    free(正常);

    // 2) 故障注入：底层句柄失效 → fread 返回 0 且 ferror 置位 → 必须返回 NULL
    FILE* 读句柄 = fopen(路径, "rb");
    if (!读句柄) { printf("失败: 无法打开自测文件\n"); return 2; }
    _close(_fileno(读句柄));
    char* 故障 = 从流读取整文件(读句柄);
    失败 += 断言(故障 == NULL, "读错误时返回 NULL（不再返回半截数据）");
    free(故障);
    // 句柄已 _close，不能再 fclose（会重复关闭无关句柄）；进程随即退出，由 OS 回收。

    remove(路径);
    printf("%s\n", 失败 == 0 ? "=== 自测通过 ===" : "=== 自测失败 ===");
    fflush(stdout);
    return 失败;
}

// 阶段10 续45：链接期附加静态库（.a）示例。
// 由 clang 编成目标文件后用 llvm-ar 打包成归档库，编译 日月 程序时经 `--附加库=` 追加；
// 链接器只在归档里查找尚未解析的符号，故能验证「先目标、后归档」的解析顺序。
#include <stdio.h>

void riyue_link_mul(int 甲, int 乙) {
    printf("%d\n", 甲 * 乙);
}

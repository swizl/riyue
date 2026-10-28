// 阶段10 续47：`--库路径=` / `--链接库=` 自测用——打包进 _clink_lib/libclink_sub.a，
// 由链接器经 `-L<目录> -lclink_sub` 找到。
#include <stdio.h>

void riyue_link_sub(int 甲, int 乙) {
    printf("%d\n", 甲 - 乙);
}

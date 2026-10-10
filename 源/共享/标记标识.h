// 由 工具/gen_hash.py 自动生成，勿手动编辑
#ifndef 标记标识_H
#define 标记标识_H
#ifdef __cplusplus
extern "C" {
#endif
// 完美哈希查找：命中返回 token id（关键字 1-99 / 文本标志 101-199），否则返回 0
int 关键字标识(const char* 标识符);
int 文本标志标识(const char* 文本);
#ifdef __cplusplus
}
#endif
#endif // 标记标识_H
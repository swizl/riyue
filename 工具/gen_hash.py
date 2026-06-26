#!/usr/bin/env python3
"""生成关键字查找函数 — 按长度分组 + 逐字节比较，可读且高效"""

关键字映射 = [
    ("函数", "函数"),
    ("空", "空"),
    ("如果", "如果"),
    ("否则", "否则"),
    ("否则如果", "否则如果"),
    ("循环", "循环"),
    ("当", "当"),
    ("做", "做"),
    ("到", "到"),
    ("中断", "中断"),
    ("继续", "继续"),
    ("变量", "变量"),
    ("常量", "常量"),
    ("返回", "返回"),
    ("匹配", "匹配"),
    ("遍历", "遍历"),
    ("导入", "导入"),
    ("整数", "整数类型"),
    ("浮点", "浮点类型"),
    ("布尔", "布尔类型"),
    ("字符串", "字符串类型"),
    ("结构体", "结构体"),
    ("枚举", "枚举"),
    ("真", "真"),
    ("假", "假"),
    ("入", "入"),
    ("方法", "方法"),
]

def gen():
    groups = {}
    for kw, ty in 关键字映射:
        bs = list(kw.encode('utf-8'))
        groups.setdefault(len(bs), []).append((kw, ty, bs))

    lines = []
    lines.append("// 由 工具/gen_hash.py 自动生成，勿手动编辑")
    lines.append("static 标记类型 完美hash查找关键字(const std::string& 标识符) {")
    lines.append("    const char* s = 标识符.data();")
    lines.append("    size_t n = 标识符.size();")
    lines.append("")
    lines.append("    switch (n) {")

    for ln in sorted(groups.keys()):
        entries = groups[ln]
        lines.append(f"        case {ln}:")
        for kw, ty, bs in entries:
            cond = ' && '.join(f's[{i}] == (char)0x{b:02X}' for i, b in enumerate(bs))
            lines.append(f"            if ({cond}) return 标记类型::{ty};  // {kw}")
        lines.append("            return 标记类型::未知;")
        lines.append("")

    lines.append("        default:")
    lines.append("            return 标记类型::未知;")
    lines.append("    }")
    lines.append("}")
    return '\n'.join(lines)

if __name__ == "__main__":
    import sys
    out = sys.argv[1] if len(sys.argv) > 1 else "源/前端/关键字哈希.h"
    with open(out, "w", encoding="utf-8") as f:
        f.write(gen())

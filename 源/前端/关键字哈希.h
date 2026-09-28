// 由 工具/gen_hash.py 自动生成，勿手动编辑
static 标记类型 完美hash查找关键字(const std::string& 标识符) {
    const char* s = 标识符.data();
    size_t n = 标识符.size();

    切换 (n) {
        情况 3:
            如果 (s[0] == (char)0xE7 && s[1] == (char)0xA9 && s[2] == (char)0xBA) return 标记类型::标记_空;  // 空
            如果 (s[0] == (char)0xE5 && s[1] == (char)0xBD && s[2] == (char)0x93) return 标记类型::标记_当;  // 当
            如果 (s[0] == (char)0xE5 && s[1] == (char)0x81 && s[2] == (char)0x9A) return 标记类型::标记_做;  // 做
            如果 (s[0] == (char)0xE5 && s[1] == (char)0x88 && s[2] == (char)0xB0) return 标记类型::到;  // 到
            如果 (s[0] == (char)0xE7 && s[1] == (char)0x9C && s[2] == (char)0x9F) return 标记类型::标记_真;  // 真
            如果 (s[0] == (char)0xE5 && s[1] == (char)0x81 && s[2] == (char)0x87) return 标记类型::标记_假;  // 假
            如果 (s[0] == (char)0xE5 && s[1] == (char)0x85 && s[2] == (char)0xA5) return 标记类型::入;  // 入
            return 标记类型::未知;

        情况 6:
            如果 (s[0] == (char)0xE5 && s[1] == (char)0x87 && s[2] == (char)0xBD && s[3] == (char)0xE6 && s[4] == (char)0x95 && s[5] == (char)0xB0) return 标记类型::函数;  // 函数
            如果 (s[0] == (char)0xE5 && s[1] == (char)0xA6 && s[2] == (char)0x82 && s[3] == (char)0xE6 && s[4] == (char)0x9E && s[5] == (char)0x9C) return 标记类型::标记_如果;  // 如果
            如果 (s[0] == (char)0xE5 && s[1] == (char)0x90 && s[2] == (char)0xA6 && s[3] == (char)0xE5 && s[4] == (char)0x88 && s[5] == (char)0x99) return 标记类型::标记_否则;  // 否则
            如果 (s[0] == (char)0xE5 && s[1] == (char)0xBE && s[2] == (char)0xAA && s[3] == (char)0xE7 && s[4] == (char)0x8E && s[5] == (char)0xAF) return 标记类型::标记_循环;  // 循环
            如果 (s[0] == (char)0xE4 && s[1] == (char)0xB8 && s[2] == (char)0xAD && s[3] == (char)0xE6 && s[4] == (char)0x96 && s[5] == (char)0xAD) return 标记类型::标记_中断;  // 中断
            如果 (s[0] == (char)0xE7 && s[1] == (char)0xBB && s[2] == (char)0xA7 && s[3] == (char)0xE7 && s[4] == (char)0xBB && s[5] == (char)0xAD) return 标记类型::标记_继续;  // 继续
            如果 (s[0] == (char)0xE5 && s[1] == (char)0x8F && s[2] == (char)0x98 && s[3] == (char)0xE9 && s[4] == (char)0x87 && s[5] == (char)0x8F) return 标记类型::变量;  // 变量
            如果 (s[0] == (char)0xE5 && s[1] == (char)0xB8 && s[2] == (char)0xB8 && s[3] == (char)0xE9 && s[4] == (char)0x87 && s[5] == (char)0x8F) return 标记类型::标记_常量;  // 常量
            如果 (s[0] == (char)0xE8 && s[1] == (char)0xBF && s[2] == (char)0x94 && s[3] == (char)0xE5 && s[4] == (char)0x9B && s[5] == (char)0x9E) return 标记类型::标记_返回;  // 返回
            如果 (s[0] == (char)0xE5 && s[1] == (char)0x8C && s[2] == (char)0xB9 && s[3] == (char)0xE9 && s[4] == (char)0x85 && s[5] == (char)0x8D) return 标记类型::匹配;  // 匹配
            如果 (s[0] == (char)0xE9 && s[1] == (char)0x81 && s[2] == (char)0x8D && s[3] == (char)0xE5 && s[4] == (char)0x8E && s[5] == (char)0x86) return 标记类型::遍历;  // 遍历
            如果 (s[0] == (char)0xE5 && s[1] == (char)0xAF && s[2] == (char)0xBC && s[3] == (char)0xE5 && s[4] == (char)0x85 && s[5] == (char)0xA5) return 标记类型::导入;  // 导入
            如果 (s[0] == (char)0xE6 && s[1] == (char)0x95 && s[2] == (char)0xB4 && s[3] == (char)0xE6 && s[4] == (char)0x95 && s[5] == (char)0xB0) return 标记类型::标记_整数类型;  // 整数
            如果 (s[0] == (char)0xE6 && s[1] == (char)0xB5 && s[2] == (char)0xAE && s[3] == (char)0xE7 && s[4] == (char)0x82 && s[5] == (char)0xB9) return 标记类型::标记_浮点类型;  // 浮点
            如果 (s[0] == (char)0xE5 && s[1] == (char)0xB8 && s[2] == (char)0x83 && s[3] == (char)0xE5 && s[4] == (char)0xB0 && s[5] == (char)0x94) return 标记类型::标记_布尔类型;  // 布尔
            如果 (s[0] == (char)0xE6 && s[1] == (char)0x9E && s[2] == (char)0x9A && s[3] == (char)0xE4 && s[4] == (char)0xB8 && s[5] == (char)0xBE) return 标记类型::标记_枚举;  // 枚举
            如果 (s[0] == (char)0xE6 && s[1] == (char)0x96 && s[2] == (char)0xB9 && s[3] == (char)0xE6 && s[4] == (char)0xB3 && s[5] == (char)0x95) return 标记类型::方法;  // 方法
            如果 (s[0] == (char)0xE5 && s[1] == (char)0xB0 && s[2] == (char)0x9D && s[3] == (char)0xE8 && s[4] == (char)0xAF && s[5] == (char)0x95) return 标记类型::标记_尝试;  // 尝试
            如果 (s[0] == (char)0xE6 && s[1] == (char)0x8D && s[2] == (char)0x95 && s[3] == (char)0xE8 && s[4] == (char)0x8E && s[5] == (char)0xB7) return 标记类型::标记_捕获;  // 捕获
            如果 (s[0] == (char)0xE6 && s[1] == (char)0x8A && s[2] == (char)0x9B && s[3] == (char)0xE5 && s[4] == (char)0x87 && s[5] == (char)0xBA) return 标记类型::标记_抛出;  // 抛出
            如果 (s[0] == (char)0xE6 && s[1] == (char)0x9C && s[2] == (char)0x80 && s[3] == (char)0xE7 && s[4] == (char)0xBB && s[5] == (char)0x88) return 标记类型::最终;  // 最终
            如果 (s[0] == (char)0xE5 && s[1] == (char)0x8D && s[2] == (char)0x8F && s[3] == (char)0xE7 && s[4] == (char)0xA8 && s[5] == (char)0x8B) return 标记类型::协程;  // 协程
            如果 (s[0] == (char)0xE8 && s[1] == (char)0xAE && s[2] == (char)0xA9 && s[3] == (char)0xE5 && s[4] == (char)0x87 && s[5] == (char)0xBA) return 标记类型::让出;  // 让出
            return 标记类型::未知;

        情况 9:
            如果 (s[0] == (char)0xE5 && s[1] == (char)0xAD && s[2] == (char)0x97 && s[3] == (char)0xE7 && s[4] == (char)0xAC && s[5] == (char)0xA6 && s[6] == (char)0xE4 && s[7] == (char)0xB8 && s[8] == (char)0xB2) return 标记类型::字符串类型;  // 字符串
            如果 (s[0] == (char)0xE7 && s[1] == (char)0xBB && s[2] == (char)0x93 && s[3] == (char)0xE6 && s[4] == (char)0x9E && s[5] == (char)0x84 && s[6] == (char)0xE4 && s[7] == (char)0xBD && s[8] == (char)0x93) return 标记类型::标记_结构体;  // 结构体
            return 标记类型::未知;

        情况 12:
            如果 (s[0] == (char)0xE5 && s[1] == (char)0x90 && s[2] == (char)0xA6 && s[3] == (char)0xE5 && s[4] == (char)0x88 && s[5] == (char)0x99 && s[6] == (char)0xE5 && s[7] == (char)0xA6 && s[8] == (char)0x82 && s[9] == (char)0xE6 && s[10] == (char)0x9E && s[11] == (char)0x9C) return 标记类型::标记_否则如果;  // 否则如果
            return 标记类型::未知;

        默认:
            return 标记类型::未知;
    }
}
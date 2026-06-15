// 日月运行时辅助函数
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 将UTF-8字符位置转换为字节位置
int 字符位置到字节位置(const char* str, int char_pos) {
    int byte_pos = 0;
    int current_char = 0;

    while (str[byte_pos] != '\0' && current_char < char_pos) {
        unsigned char c = (unsigned char)str[byte_pos];
        if (c < 0x80) {
            byte_pos += 1;
        } else if ((c & 0xE0) == 0xC0) {
            byte_pos += 2;
        } else if ((c & 0xF0) == 0xE0) {
            byte_pos += 3;
        } else if ((c & 0xF8) == 0xF0) {
            byte_pos += 4;
        } else {
            byte_pos += 1;
        }
        current_char++;
    }
    return byte_pos;
}

// 获取UTF-8字符串的字符数
int 获取字符数(const char* str) {
    int count = 0;
    int i = 0;
    while (str[i] != '\0') {
        unsigned char c = (unsigned char)str[i];
        if (c < 0x80) {
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            i += 4;
        } else {
            i += 1;
        }
        count++;
    }
    return count;
}

// 映射辅助函数
// 键数组和值数组是平行的，大小固定为256

// 在键数组中查找键，返回索引，未找到返回-1
int 映射查找(const char** keys, int size, const char* key) {
    for (int i = 0; i < size; i++) {
        if (keys[i] != NULL && strcmp(keys[i], key) == 0) {
            return i;
        }
    }
    return -1;
}

// 在映射中设置键值对，返回新大小
int 映射设置(const char** keys, const char** values, int size, const char* key, const char* value) {
    int idx = 映射查找(keys, size, key);
    if (idx >= 0) {
        values[idx] = value;
        return size;
    }
    if (size < 256) {
        keys[size] = key;
        values[size] = value;
        return size + 1;
    }
    return size;
}

// 从映射中获取值，未找到返回空字符串
const char* 映射获取(const char** keys, const char** values, int size, const char* key) {
    int idx = 映射查找(keys, size, key);
    if (idx >= 0) {
        return values[idx];
    }
    return "";
}

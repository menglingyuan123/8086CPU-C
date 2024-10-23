//
// Created by 孟令圆 on 2024/10/24.
//
#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ctype.h"
#include "instructions.h"



// 辅助函数：检查字符串是否以指定后缀结尾
bool endsWith(const char* str, const char* suffix) {
    size_t strLen = strlen(str);
    size_t suffixLen = strlen(suffix);
    if (suffixLen > strLen) {
        return false;
    }
    return strncmp(str + strLen - suffixLen, suffix, suffixLen) == 0;
}
// 识别指令类型
uint16_t  identifyAM(const char* src) {
    uint16_t n;
    char temp[50]; char base[50]; char offset[50];
    strncpy(temp, src, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0'; // 确保字符串以null结束
    if (src[0] == '[' && src[strlen(src) - 1] == ']') {
        // 去除方括号并检查内容
        strncpy(temp, src + 1, strlen(src) - 2);
        temp[strlen(src) - 2] = '\0';
        if (strchr(temp, '+') != NULL) {
            // 包含加运算符，是基址变址寻址
            // 查找加号的位置

            char* plus_sign = strchr(temp, '+');
            if (plus_sign) {
                // 复制基址部分
                strncpy(base, temp, plus_sign - temp);
                base[plus_sign - temp] = '\0';
                // 复制变址部分
                strcpy(offset, plus_sign + 1);
            }
            uint16_t* dest_reg1 = register_addressing(base);
            uint16_t dest_value1 = *dest_reg1;
            uint16_t* dest_reg2 = register_addressing(offset);
            uint16_t dest_value2 = *dest_reg2;

            return memory[dest_value1 + dest_value2];
        }
        else if (isalpha((unsigned char)temp[0])) {
            // 寄存器间接寻址
            uint16_t* dest_reg = register_addressing(temp);
            uint16_t dest_value = *dest_reg;

            return memory[dest_value];
        }
        else {

            //直接寻址
            return memory[atoi(src)];
        }
    }
    else if (isalpha((unsigned char)src[0])) {
        int t=0;
        for (int i = 0; i < symbol_count; i++) {
            if (strcmp(symbol_table[i].name, src) == 0) {
                n = get_symbol_value( symbol_table[i].name);
                //n = memory[symbol_table[i].value];
                t=1;
                return n;
            }
        }
        if(t==0) {
            // 寄存器直接寻址
            uint16_t* dest_reg = register_addressing(src);
            uint16_t dest_value = *dest_reg;
            return dest_value;
        }
    }
    else {
        // 立即数寻址
        if (endsWith(src, "h") || endsWith(src, "H")) {
            // 去掉后缀"h"或"H"
            size_t len = strlen(src);
            char hexStr[len + 1]; // +1 for null terminator
            strncpy(hexStr, src, len - 1);
            hexStr[len - 1] = '\0'; // null terminate the string

            // 使用strtol将16进制字符串转换为长整数，并检查错误
            char* endptr;
            long value = strtol(hexStr, &endptr, 16);
            return (uint16_t)value;
        }
        else {
            // 假设其余字符串为十进制
            char* endptr;
            long value = strtol(src, &endptr, 10);
            return (uint16_t)value;
        }


    }
}






void getString(const char* array, char* resultBuffer, size_t bufferSize) {
    int len = strlen(array);
    char str[50];
    uint16_t n;

    if (array[0] != '[' || (len <= 2 || array[len - 1] != ']')) {
        strncpy(resultBuffer, array, bufferSize - 1);  // 安全复制
        resultBuffer[bufferSize - 1] = '\0';
        return;
    }
    strncpy(str, array + 1, len - 2);
    str[len - 2] = '\0';

    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (strcmp(symbol_table[i].name, str) == 0) {
            n = memory[symbol_table[i].value];
            snprintf(resultBuffer, bufferSize, "%04X", n);
            return;
        }
    }

    strcpy(resultBuffer, "");  // 如果未找到符号
}

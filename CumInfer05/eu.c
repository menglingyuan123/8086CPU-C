//
// Created by 孟令圆 on 2024/10/22.
//
#include "eu.h"
#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include "instructions.h"
void update_flags(uint16_t result) {
    // 根据结果设置 FLAGS 标志位
    if (result == 0) {
        cpu.FLAGS |= ZF;  // 设置零标志
    } else {
        cpu.FLAGS &= ~ZF;  // 清除零标志
    }
    if (result & 0x8000) {
        cpu.FLAGS |= SF;  // 设置符号标志
    } else {
        cpu.FLAGS &= ~SF;  // 清除符号标志
    }
}

void execute_instruction(const char* instruction) {
    printf("执行指令：%s\n", instruction);

    if (strncmp(instruction, "MOV", 3) == 0) {
        execute_mov("AX", "0x1234");
    } else if (strncmp(instruction, "ADD", 3) == 0) {
        execute_add("AX", "0x0001");
    }

    update_flags(cpu.AX);  // 更新 FLAGS 状态
}

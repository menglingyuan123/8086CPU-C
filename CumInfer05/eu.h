//
// Created by 孟令圆 on 2024/10/22.
//

#ifndef CUMINFER05_EU_H
#define CUMINFER05_EU_H
#include <stdint.h>

// 定义 FLAGS 标志位掩码
#define CF 0x0001  // 进位标志
#define ZF 0x0040  // 零标志
#define SF 0x0080  // 符号标志
#define OF 0x0800  // 溢出标志

void execute_instruction(const char* instruction);  // 执行指令
void update_flags(uint16_t result);  // 根据运算结果更新标志位

#endif //CUMINFER05_EU_H

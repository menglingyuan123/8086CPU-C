//
// Created by 孟令圆 on 2024/10/22.
//

#ifndef CUMINFER05_BIU_H
#define CUMINFER05_BIU_H


#include <stdint.h>

#define QUEUE_SIZE 256  // 增大队列大小到 256 条指令

typedef struct {
    char queue[QUEUE_SIZE][50];  // 指令队列
    int front, rear;  // 队列的头部和尾部指针
} InstructionQueue;

extern InstructionQueue instruction_queue;

void prefetch_instruction();  // 从内存预取指令到队列
char* fetch_from_queue();     // 从队列中取出指令
int is_queue_empty();
int is_queue_full();

#endif //CUMINFER05_BIU_H

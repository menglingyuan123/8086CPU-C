//
// Created by 孟令圆 on 2024/10/22.
//
#include "biu.h"
#include <string.h>
#include <stdio.h>

InstructionQueue instruction_queue = { .front = 0, .rear = 0 };

// 检查队列是否为空
int is_queue_empty() {
    return instruction_queue.front == instruction_queue.rear;
}

// 检查队列是否已满
int is_queue_full() {
    return (instruction_queue.rear + 1) % QUEUE_SIZE == instruction_queue.front;
}

// 从内存预取指令
void prefetch_instruction() {
    if (!is_queue_full()) {
        // 模拟从内存加载指令
        strcpy(instruction_queue.queue[instruction_queue.rear], "MOV AX, 0x1234");
        instruction_queue.rear = (instruction_queue.rear + 1) % QUEUE_SIZE;
        printf("预取指令： MOV AX, 0x1234\n");
    }
}

// 从队列中取出指令
char* fetch_from_queue() {
    if (is_queue_empty()) {
        printf("指令队列为空\n");
        return NULL;
    }
    char* instruction = instruction_queue.queue[instruction_queue.front];
    instruction_queue.front = (instruction_queue.front + 1) % QUEUE_SIZE;
    return instruction;
}

#include <stdio.h>
#include <string.h>
#include "cpu.h"          // 包含 CPU 结构和初始化
#include "biu.h"          // 包含指令队列的定义和操作
#include "instructions.h" // 包含指令解析与执行函数

int main() {
    // 初始化 CPU 和内存
    initialize_cpu();

    char instruction[100];  // 存储用户输入的指令

    printf("=== 8086 CPU 模拟器启动 ===\n");
    printf("输入指令 (例如 MOV AX, 1 或 ADD BX, 0x20)。输入 'exit' 退出。\n");

    // 指令输入循环
    while (1) {
        printf("\n> ");
        fgets(instruction, sizeof(instruction), stdin);  // 获取用户输入
        instruction[strcspn(instruction, "\n")] = '\0';  // 去除换行符

        if (strcmp(instruction, "HIT") == 0) {
            printf("退出模拟器。\n");
            break;
        }

        // 将所有指令保存到指令内存
        if (instruction_count < MAX_INSTRUCTIONS) {
            strcpy(instruction_memory[instruction_count++], instruction);
            printf("已保存指令: %s\n", instruction);
        } else {
            printf("内存已满，无法添加更多指令。\n");
        }
    }

    // 执行所有指令
    printf("\n开始执行所有指令...\n");
    while (cpu.IP < instruction_count) {
        strcpy(cpu.IR, instruction_memory[cpu.IP]);  // 加载到 IR 寄存器

        printf("当前 IP: %04X\n", cpu.IP);
        printf("执行指令 %d: %s\n", cpu.IP + 1, instruction_memory[cpu.IP]);

        // 调用解码与执行函数
        decode_and_execute(instruction_memory[cpu.IP]);

        // 更新 IP
        cpu.IP++;
        printf("执行后状态：\n");
        display_cpu_state();
        printf("\n");
    }

    printf("所有指令执行完毕。\n");
    return 0;
}

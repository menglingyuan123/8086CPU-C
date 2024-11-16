//
// Created by ����Բ on 2024/10/22.
//
#include "eu.h"
#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include "instructions.h"
void update_flags(uint16_t result) {
    // ���ݽ������ FLAGS ��־λ
    if (result == 0) {
        cpu.FLAGS |= ZF;  // �������־
    } else {
        cpu.FLAGS &= ~ZF;  // ������־
    }
    if (result & 0x8000) {
        cpu.FLAGS |= SF;  // ���÷��ű�־
    } else {
        cpu.FLAGS &= ~SF;  // ������ű�־
    }
}

void execute_instruction(const char* instruction) {
    printf("ִ��ָ�%s\n", instruction);

    if (strncmp(instruction, "MOV", 3) == 0) {
        execute_mov("AX", "0x1234");
    } else if (strncmp(instruction, "ADD", 3) == 0) {
        execute_add("AX", "0x0001");
    }

    update_flags(cpu.AX);  // ���� FLAGS ״̬
}

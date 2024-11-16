#include <stdio.h>
#include <string.h>
#include "cpu.h"          // ���� CPU �ṹ�ͳ�ʼ��
#include "biu.h"          // ����ָ����еĶ���Ͳ���
#include "instructions.h" // ����ָ�������ִ�к���

int main() {
    // ��ʼ�� CPU ���ڴ�
    initialize_cpu();

    char instruction[100];  // �洢�û������ָ��

    printf("=== 8086 CPU ģ�������� ===\n");
    printf("����ָ�� (���� MOV AX, 1 �� ADD BX, 0x20)������ 'exit' �˳���\n");

    // ָ������ѭ��
    while (1) {
        printf("\n> ");
        fgets(instruction, sizeof(instruction), stdin);  // ��ȡ�û�����
        instruction[strcspn(instruction, "\n")] = '\0';  // ȥ�����з�

        if (strcmp(instruction, "HIT") == 0) {
            printf("�˳�ģ������\n");
            break;
        }

        // ������ָ��浽ָ���ڴ�
        if (instruction_count < MAX_INSTRUCTIONS) {
            strcpy(instruction_memory[instruction_count++], instruction);
            printf("�ѱ���ָ��: %s\n", instruction);
        } else {
            printf("�ڴ��������޷���Ӹ���ָ�\n");
        }
    }

    // ִ������ָ��
    printf("\n��ʼִ������ָ��...\n");
    while (cpu.IP < instruction_count) {
        strcpy(cpu.IR, instruction_memory[cpu.IP]);  // ���ص� IR �Ĵ���

        printf("��ǰ IP: %04X\n", cpu.IP);
        printf("ִ��ָ�� %d: %s\n", cpu.IP + 1, instruction_memory[cpu.IP]);

        // ���ý�����ִ�к���
        decode_and_execute(instruction_memory[cpu.IP]);

        // ���� IP
        cpu.IP++;
        printf("ִ�к�״̬��\n");
        display_cpu_state();
        printf("\n");
    }

    printf("����ָ��ִ����ϡ�\n");
    return 0;
}

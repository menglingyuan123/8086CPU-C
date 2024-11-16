#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
Registers cpu;
Bus cpu_bus;
uint8_t memory[0x100000];  // 1MB �ڴ�
char instruction_memory[MAX_INSTRUCTIONS][50];
int instruction_count = 0;
Symbol symbol_table[MAX_SYMBOLS];  // ���ű�
int symbol_count = 0;  // ��������
static uint16_t data_offset = 0;  // ���ݶ�ƫ����

void initialize_cpu() {
    memset(&cpu, 0, sizeof(Registers));  // ��ʼ���Ĵ���
    memset(&cpu_bus,0,sizeof(Bus));
    memset(memory, 0, sizeof(memory));   // ��ʼ���ڴ�
    cpu.SP = 0xFFFE;  // ջָ���ʼ��Ϊ�ڴ涥��
    cpu.IP = 0;  // ��ָ���ڴ����ʼ��ַ��ʼִ��
    strcpy(cpu.IR, "");  // ��ʼ�� IR �Ĵ���Ϊ��
    cpu.DS = 0;//���ݶ�Ĭ����0
    cpu.CS = 128;//�����Ĭ����128
    cpu_bus.address_bus = 0;
    cpu_bus.control_bus = 0;
    cpu_bus.data_bus = 0;
}
void add_symbol(const char* name, uint32_t address, uint16_t value) {
    if (symbol_count < MAX_SYMBOLS) {
        strcpy(symbol_table[symbol_count].name, name);
        symbol_table[symbol_count].address = address;
        symbol_table[symbol_count].value = value;
        symbol_count++;
    } else {
        printf("���ű��������޷�����·���: %s\n", name);
    }
}
// �����:ƫ�Ƶ�ַ�������ַ
uint32_t get_physical_address(uint16_t segment, uint16_t offset) {
    return (segment << 4) + offset;
}
// �����ź͵�ַ��ֵ��ӵ����ű�


void define_data(const char* name, const char* directive, const char* value) {
    // ȷ�� DW �������ݴ洢ǰ��ַ�� 2 �ֽڶ����
    if (strcmp(directive, "DW") == 0) {
        if (data_offset % 2 != 0) {
            data_offset++;  // ��������룬���� 1 �ֽ�
        }
    }

    uint32_t address = get_physical_address(cpu.DS, data_offset);  // ��ȡ�����ַ

    // �����ź͵�ַ��ӵ����ű�
    add_symbol(name, address, 0);

    if (strcmp(directive, "DB") == 0) {
        if (value[0] == '\'') {  // �����ַ���
            int i = 1;
            while (value[i] != '\'' && value[i] != '\0') {
                memory[address++] = value[i++];  // ÿ���ַ� 1 �ֽڴ洢
                data_offset++;
            }
            memory[address++] = 0;  // ����ַ���������
            data_offset++;  // ����ƫ����
        } else {  // �����ֽ�ֵ
            uint8_t byte_value = (uint8_t)strtol(value, NULL, 0);
            memory[address] = byte_value;
            data_offset += 1;
        }
        printf("�洢 DB %s ����ַ %04X:%04X\n", value, cpu.DS, data_offset - 1);
    }
    else if (strcmp(directive, "DW") == 0) {
        if (value[0] == '\'') {  // �����ַ�����ʽ�� DW
            int i = 1;
            while (value[i] != '\'' && value[i] != '\0') {
                memory[address++] = value[i] & 0xFF;         // ���ֽ�
                memory[address++] = (value[i] >> 8) & 0xFF;  // ���ֽ�
                data_offset += 2;  // ÿ���ַ�ռ 2 �ֽ�
                i++;
            }
            memory[address++] = 0;  // ��������ֽڵĽ�����
            memory[address++] = 0;
            data_offset += 2;
        } else {  // ������ 16 λֵ
            uint16_t word_value = (uint16_t)strtol(value, NULL, 0);
            memory[address] = word_value & 0xFF;        // �洢���ֽ�
            memory[address + 1] = (word_value >> 8) & 0xFF;  // �洢���ֽ�
            data_offset += 2;
        }
        printf("�洢 DW %s ����ַ %04X:%04X\n", value, cpu.DS, data_offset - 2);
    }
    else {
        printf("δ֪������ָ��: %s\n", directive);
    }
}





void display_cpu_state() {
    printf("AX: %04X  BX: %04X  CX: %04X  DX: %04X\n", cpu.AX, cpu.BX, cpu.CX, cpu.DX);
    printf("AL: %04X  AH: %04X  BL: %04X  BH: %04X\n", cpu.AL, cpu.AH, cpu.BL, cpu.BH);
    printf("CL: %04X  CH: %04X  DL: %04X  DH: %04X\n", cpu.CL, cpu.CH, cpu.DL, cpu.DH);
    printf("SP: %04X  BP: %04X  SI: %04X  DI: %04X\n", cpu.SP, cpu.BP, cpu.SI, cpu.DI);
    printf("IP: %04X  FLAGS: %04X\n", cpu.IP, cpu.FLAGS);
    printf("CS: %04X  DS: %04X  ES: %04X  SS: %04X\n", cpu.CS, cpu.DS, cpu.ES, cpu.SS);
    printf("IR: %s\n", cpu.IR);  // ��ӡ IR ������
}


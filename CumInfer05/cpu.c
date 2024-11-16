#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
Registers cpu;
Bus cpu_bus;
uint8_t memory[0x100000];  // 1MB 内存
char instruction_memory[MAX_INSTRUCTIONS][50];
int instruction_count = 0;
Symbol symbol_table[MAX_SYMBOLS];  // 符号表
int symbol_count = 0;  // 符号数量
static uint16_t data_offset = 0;  // 数据段偏移量

void initialize_cpu() {
    memset(&cpu, 0, sizeof(Registers));  // 初始化寄存器
    memset(&cpu_bus,0,sizeof(Bus));
    memset(memory, 0, sizeof(memory));   // 初始化内存
    cpu.SP = 0xFFFE;  // 栈指针初始化为内存顶部
    cpu.IP = 0;  // 从指令内存的起始地址开始执行
    strcpy(cpu.IR, "");  // 初始化 IR 寄存器为空
    cpu.DS = 0;//数据段默认是0
    cpu.CS = 128;//代码段默认是128
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
        printf("符号表已满，无法添加新符号: %s\n", name);
    }
}
// 计算段:偏移地址的物理地址
uint32_t get_physical_address(uint16_t segment, uint16_t offset) {
    return (segment << 4) + offset;
}
// 将符号和地址、值添加到符号表


void define_data(const char* name, const char* directive, const char* value) {
    // 确保 DW 类型数据存储前地址是 2 字节对齐的
    if (strcmp(directive, "DW") == 0) {
        if (data_offset % 2 != 0) {
            data_offset++;  // 如果不对齐，增加 1 字节
        }
    }

    uint32_t address = get_physical_address(cpu.DS, data_offset);  // 获取物理地址

    // 将符号和地址添加到符号表
    add_symbol(name, address, 0);

    if (strcmp(directive, "DB") == 0) {
        if (value[0] == '\'') {  // 处理字符串
            int i = 1;
            while (value[i] != '\'' && value[i] != '\0') {
                memory[address++] = value[i++];  // 每个字符 1 字节存储
                data_offset++;
            }
            memory[address++] = 0;  // 添加字符串结束符
            data_offset++;  // 增加偏移量
        } else {  // 处理单字节值
            uint8_t byte_value = (uint8_t)strtol(value, NULL, 0);
            memory[address] = byte_value;
            data_offset += 1;
        }
        printf("存储 DB %s 到地址 %04X:%04X\n", value, cpu.DS, data_offset - 1);
    }
    else if (strcmp(directive, "DW") == 0) {
        if (value[0] == '\'') {  // 处理字符串形式的 DW
            int i = 1;
            while (value[i] != '\'' && value[i] != '\0') {
                memory[address++] = value[i] & 0xFF;         // 低字节
                memory[address++] = (value[i] >> 8) & 0xFF;  // 高字节
                data_offset += 2;  // 每个字符占 2 字节
                i++;
            }
            memory[address++] = 0;  // 添加两个字节的结束符
            memory[address++] = 0;
            data_offset += 2;
        } else {  // 处理单个 16 位值
            uint16_t word_value = (uint16_t)strtol(value, NULL, 0);
            memory[address] = word_value & 0xFF;        // 存储低字节
            memory[address + 1] = (word_value >> 8) & 0xFF;  // 存储高字节
            data_offset += 2;
        }
        printf("存储 DW %s 到地址 %04X:%04X\n", value, cpu.DS, data_offset - 2);
    }
    else {
        printf("未知的数据指令: %s\n", directive);
    }
}





void display_cpu_state() {
    printf("AX: %04X  BX: %04X  CX: %04X  DX: %04X\n", cpu.AX, cpu.BX, cpu.CX, cpu.DX);
    printf("AL: %04X  AH: %04X  BL: %04X  BH: %04X\n", cpu.AL, cpu.AH, cpu.BL, cpu.BH);
    printf("CL: %04X  CH: %04X  DL: %04X  DH: %04X\n", cpu.CL, cpu.CH, cpu.DL, cpu.DH);
    printf("SP: %04X  BP: %04X  SI: %04X  DI: %04X\n", cpu.SP, cpu.BP, cpu.SI, cpu.DI);
    printf("IP: %04X  FLAGS: %04X\n", cpu.IP, cpu.FLAGS);
    printf("CS: %04X  DS: %04X  ES: %04X  SS: %04X\n", cpu.CS, cpu.DS, cpu.ES, cpu.SS);
    printf("IR: %s\n", cpu.IR);  // 打印 IR 的内容
}


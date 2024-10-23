#ifndef CUMINFER05_CPU_H
#define CUMINFER05_CPU_H

#include <stdint.h>
#define MEMORY_SIZE 0x100000
#define MAX_INSTRUCTIONS 1024  // 支持最多 1024 条指令
extern char instruction_memory[MAX_INSTRUCTIONS][50];  // 每条指令最大长度50字节
extern int instruction_count;  // 当前存储的指令数量

#define MAX_SYMBOLS 100  // 最多支持100个符号
// 符号表结构体
typedef struct {
    char name[50];  // 符号名
    uint32_t address;  // 符号的内存地址
    uint16_t value;     // 符号的值（1字节或2字节）
} Symbol;

extern Symbol symbol_table[MAX_SYMBOLS];  // 符号表
extern int symbol_count;  // 当前符号数量
void add_symbol(const char* name, uint32_t address, uint16_t value);
// CPU 寄存器结构体
typedef struct {
    uint16_t AX, BX, CX, DX;
    uint16_t AL,AH,BL,BH,CL,CH,DL,DH;
    uint16_t SP, BP, SI, DI;
    uint16_t IP;
    uint16_t CS, DS, ES, SS;
    uint16_t FLAGS;
    char IR[50];  // 指令寄存器，用于存储当前指令
} Registers;


extern Registers cpu;            // 全局 CPU 寄存器
extern uint8_t memory[0x100000];  //  内存
void initialize_cpu();
uint32_t get_physical_address(uint16_t segment, uint16_t offset);
void define_data(const char* name, const char* directive, const char* value);
void display_cpu_state();
uint16_t* get_register_ptr(const char* reg);

#endif

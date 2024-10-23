#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ctype.h"
#include "addressing.h"
// 各类指令的声明
void update_flags_add(uint16_t result, uint16_t dest_value, uint16_t src_value);
void execute_mov(const char* dest, const char* src);
void update_flags_sub(uint16_t result, uint16_t dest_value, uint16_t src_value);
void execute_add(const char* dest, const char* src);
void execute_sub(const char* dest, const char* src);
void execute_lea(const char* dest, const char* src);
void execute_and(const char* dest, const char* src);
void execute_xor(const char* dest, const char* src);
void execute_or(const char* dest, const char* src);
void execute_push(const char* reg);
void execute_pop(const char* reg);
void execute_not(const char* reg);
void execute_inc(const char* reg);
void execute_stc();
void execute_clc();
void execute_jmp();
void execute_int();
void CMPSB();
void MOVSB();

// 各种寻址方式的实现
uint16_t immediate_addressing(const char* operand);
uint16_t* register_addressing(const char* reg);
uint16_t get_symbol_value(const char* name);

// 标志位，用于记录是否在代码段或数据段中
int in_data_segment = 0;  // 是否在数据段内
int in_code_segment = 0;  // 是否在代码段内
void handle_assume(const char* instruction) {
    char segment_name[50] = {0};  // 存储段名
    char* token = NULL;

    // 提取 CS 段名
    token = strstr(instruction, "CS:");
    if (token) {
        sscanf(token, "CS:%[^,]", segment_name);  // 提取 CS 段名
        uint32_t address = get_physical_address(cpu.CS, 0);
        add_symbol(segment_name, address, cpu.CS);  // 添加符号表
        printf("添加符号：%s, 地址：%04X, 值：%04X\n", segment_name, address, cpu.CS);
    }

    // 提取 DS 段名
    token = strstr(instruction, "DS:");
    if (token) {
        sscanf(token, "DS:%[^,]", segment_name);  // 提取 DS 段名
        uint32_t address = get_physical_address(cpu.DS, 0);
        add_symbol(segment_name, address, cpu.DS);  // 添加符号表
        printf("添加符号：%s, 地址：%04X, 值：%04X\n", segment_name, address, cpu.DS);
    }

    // 忽略 SS 段名处理
    if (strstr(instruction, "SS:")) {
        printf("识别到 SS 段，但暂时不处理。\n");
    }
}

// 主解码和执行函数
void decode_and_execute(const char* instruction) {
    // 优先处理特殊指令或段标识
    if (strcmp(instruction, "DATA SEGMENT") == 0) {
        printf("进入数据段。\n");
        in_data_segment = 1;  // 标记进入数据段
        return;
    }
    if (strcmp(instruction, "DATA ENDS") == 0) {
        printf("退出数据段。\n");
        in_data_segment = 0;  // 标记退出数据段
        return;
    }
    if (strcmp(instruction, "CODE SEGMENT") == 0) {
        printf("进入代码段。\n");
        in_code_segment = 1;  // 标记进入代码段
        return;
    }
    if (strcmp(instruction, "CODE ENDS") == 0) {
        printf("退出代码段。\n");
        in_code_segment = 0;  // 标记退出代码段
        return;
    }
    // 处理 ASSUME 指令
    if (strncmp(instruction, "ASSUME", 6) == 0) {
        printf("识别到ASSUME定义\n");
        handle_assume(instruction);
        return;
    }
    // 如果在数据段内，处理数据定义指令
    if (in_data_segment) {
        char name[50], directive[10], value[50];
        sscanf(instruction, "%s %s %s", name, directive, value);
        define_data(name, directive, value);
        return;
    }

    // 解析和拆分指令（仅适用于代码段内的指令）
    char op[10] = {0}, arg1[10] = {0}, arg2[10] = {0};
    char* token = strtok((char*)instruction, " ,");
    if (token) strncpy(op, token, sizeof(op) - 1);

    token = strtok(NULL, " ,");
    if (token) strncpy(arg1, token, sizeof(arg1) - 1);

    token = strtok(NULL, " ,");
    if (token) strncpy(arg2, token, sizeof(arg2) - 1);

    printf("\n解析指令：%s %s, %s\n", op, arg1, arg2);

    // 在代码段内执行指令
    if (in_code_segment) {
        if (strcmp(op, "MOV") == 0) {
            execute_mov(arg1, arg2);
        } else if (strcmp(op, "ADD") == 0) {
            execute_add(arg1, arg2);
        } else if (strcmp(op, "SUB") == 0) {
            execute_sub(arg1, arg2);
        } else if (strcmp(op, "LEA") == 0) {
            execute_lea(arg1, arg2);
        } else if (strcmp(op, "PUSH") == 0) {
            execute_push(arg1);
        } else if (strcmp(op, "POP") == 0) {
            execute_pop(arg1);
        } else if (strcmp(op, "JMP") == 0) {
            execute_jmp(arg1);  // 修正JMP指令，使用目标地址
        } else if (strcmp(op, "MOVSB") == 0) {
            MOVSB();
        } else if (strcmp(op, "CMPSB") == 0) {
            CMPSB();
        } else if (strcmp(op, "OR") == 0) {
            execute_or(arg1, arg2);
        } else if (strcmp(op, "AND") == 0) {
            execute_and(arg1, arg2);
        } else if (strcmp(op, "XOR") == 0) {
            execute_xor(arg1, arg2);
        } else if (strcmp(op, "NOT") == 0) {
            execute_not(arg1);
        } else if (strcmp(op, "INC") == 0) {
            execute_inc(arg1);
        } else if (strcmp(op, "STC") == 0) {
            execute_stc();
        } else if (strcmp(op, "CLC") == 0) {
            execute_clc();
        } else if (strcmp(op, "INT") == 0) {
            execute_int(arg1);
        } else {
            // 处理未知符号，将其视为符号名
            printf("识别到新符号名: %s\n", instruction);

            // 获取当前指令的 IP 作为符号的地址
            uint32_t address = cpu.IP;

            // 将新符号名加入符号表，默认值为 0
            add_symbol(instruction, address, 0);

            printf("添加符号: %s, 地址: %04X, 默认值: 0000\n", instruction, address);
        }
    } else {
        printf("未识别的指令或段定义错误：%s\n", instruction);
    }
}


int is_immediate(const char* operand) {
    // 1. 检查是否是十六进制立即数（以 0x 或 0X 开头）
    if ((operand[0] == '0') && (operand[1] == 'x' || operand[1] == 'X')) {
        return 1;
    }

    // 2. 检查是否是十进制数字（允许负号或正号）
    int i = 0;
    if (operand[0] == '-' || operand[0] == '+') {
        i = 1;  // 跳过符号位
    }
    for (; operand[i] != '\0'; i++) {
        if (!isdigit(operand[i])) {
            return 0;  // 如果有非数字字符，则不是十进制数
        }
    }
    return 1;  // 所有字符都是数字，则为十进制数
}

// 3. 检查是否为字符常量（单引号括起来）
int is_char_constant(const char* operand) {
    // 单字符：如 'A'
    if (operand[0] == '\'' && operand[2] == '\'' && operand[3] == '\0') {
        return 1;  // 单字符常量
    }
    // 双字符：如 'AB'
    if (operand[0] == '\'' && operand[3] == '\'' && operand[4] == '\0') {
        return 2;  // 双字符常量
    }
    return 0;  // 非字符常量
}

//immediate_addressing() 函数，使其根据字符串的格式来选择合适的进制进行转换。
uint16_t immediate_addressing(const char* operand) {
    // 1. 处理单字符常量
    if (is_char_constant(operand) == 1) {
        return (uint16_t)operand[1];  // 返回单字符的 ASCII 码
    }
    // 2. 处理双字符常量：将两个字符的 ASCII 码组合成一个 16 位整数
    if (is_char_constant(operand) == 2) {
        return ((uint16_t)operand[1] << 8) | (uint16_t)operand[2];
    }
    // 3. 十六进制和十进制立即数处理
    if ((operand[0] == '0') && (operand[1] == 'x' || operand[1] == 'X')) {
        return (uint16_t)strtol(operand, NULL, 16);  // 十六进制转换
    }
    return (uint16_t)strtol(operand, NULL, 10);  // 十进制转换
}
// MOV 指令实现
void execute_mov(const char* dest, const char* src) {
    char dest_result[50];
    getString(dest,dest_result,sizeof(dest_result));
    uint16_t* dest_reg = register_addressing(dest_result);  // 获取目标寄存器指针 *dest_reg是我自己定义的
    uint16_t src_value = 0;

    printf("\n执行 MOV %s, %s\n", dest, src);
    // 检查目标寄存器是否有效
    if (dest_reg) {
        //进行寻址之后获取两个数 src_value *dest_reg
        src_value = identifyAM(src);
        *dest_reg = src_value;
    }else {
            printf("无效的目标寄存器: %s\n", dest);
    }
    // 打印执行后的 CPU 状态（可选）
    // printf("执行后状态:\n");
    // display_cpu_state();
}

// 更新标志位函数
void update_flags_add(uint16_t result, uint16_t dest_value, uint16_t src_value) {
    // 进位标志（CF）：最高位进位
    if (result < dest_value) {
        cpu.FLAGS |= 0x1;  // 设置 CF
    } else {
        cpu.FLAGS &= ~0x1;  // 清除 CF
    }

    // 溢出标志（OF）：符号溢出
    uint16_t sign_dest = dest_value & 0x8000;
    uint16_t sign_src = src_value & 0x8000;
    uint16_t sign_res = result & 0x8000;
    if ((sign_dest == sign_src) && (sign_dest != sign_res)) {
        cpu.FLAGS |= (1 << 11);  // 设置 OF
    } else {
        cpu.FLAGS &= ~(1 << 11);  // 清除 OF
    }

    // 零标志（ZF）：结果是否为 0
    if (result == 0) {
        cpu.FLAGS |= (1 << 6);  // 设置 ZF
    } else {
        cpu.FLAGS &= ~(1 << 6);  // 清除 ZF
    }

    // 符号标志（SF）：最高位符号位
    if (result & 0x8000) {
        cpu.FLAGS |= (1 << 7);  // 设置 SF
    } else {
        cpu.FLAGS &= ~(1 << 7);  // 清除 SF
    }

    // 辅助进位标志（AF）：第 4 位进位
    if (((dest_value & 0xF) + (src_value & 0xF)) > 0xF) {
        cpu.FLAGS |= (1 << 4);  // 设置 AF
    } else {
        cpu.FLAGS &= ~(1 << 4);  // 清除 AF
    }
}
// ADD 指令实现
void execute_add(const char* dest, const char* src) {
    char dest_result[50];
    getString(dest,dest_result,sizeof(dest_result));
    uint16_t* dest_reg = register_addressing(dest_result);  // 获取目标寄存器指针 *dest_reg是我自己定义的
    uint16_t src_value = 0;

    printf("\n执行 ADD %s, %s\n", dest, src);

    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // 获取目标寄存器的当前值

        src_value = identifyAM(src);
        printf("解析后的源操作数值: %04X\n", src_value);
        printf("目标寄存器初始值: %04X, 源操作数值: %04X\n", dest_value, src_value);

        uint16_t result = dest_value + src_value;  // 执行加法

        *dest_reg = result;  // 将结果存入目标寄存器

        // 更新标志位
        update_flags_add(result, dest_value, src_value);

        printf("执行后 %s = %04X\n", dest, *dest_reg);
    }else {
        printf("无效的目标寄存器: %s\n", dest);
    }

    //printf("执行后状态:\n");
    //display_cpu_state();
}

// 更新标志位函数
void update_flags_sub(uint16_t result, uint16_t dest_value, uint16_t src_value) {
    // 进位标志（CF）：借位检测（当 dest < src 时设置 CF）
    if (dest_value < src_value) {
        cpu.FLAGS |= 0x1;  // 设置 CF
    } else {
        cpu.FLAGS &= ~0x1;  // 清除 CF
    }

    // 溢出标志（OF）：符号溢出（当符号位不一致时）
    uint16_t sign_dest = dest_value & 0x8000;
    uint16_t sign_src = src_value & 0x8000;
    uint16_t sign_res = result & 0x8000;
    if ((sign_dest != sign_src) && (sign_dest != sign_res)) {
        cpu.FLAGS |= (1 << 11);  // 设置 OF
    } else {
        cpu.FLAGS &= ~(1 << 11);  // 清除 OF
    }

    // 零标志（ZF）：结果是否为 0
    if (result == 0) {
        cpu.FLAGS |= (1 << 6);  // 设置 ZF
    } else {
        cpu.FLAGS &= ~(1 << 6);  // 清除 ZF
    }

    // 符号标志（SF）：结果的符号位（最高位）
    if (result & 0x8000) {
        cpu.FLAGS |= (1 << 7);  // 设置 SF
    } else {
        cpu.FLAGS &= ~(1 << 7);  // 清除 SF
    }

    // 辅助进位标志（AF）：检测第 4 位是否借位
    if (((dest_value & 0xF) - (src_value & 0xF)) < 0) {
        cpu.FLAGS |= (1 << 4);  // 设置 AF
    } else {
        cpu.FLAGS &= ~(1 << 4);  // 清除 AF
    }
}

// SUB 指令实现
void execute_sub(const char* dest, const char* src) {
    char dest_result[50];
    getString(dest,dest_result,sizeof(dest_result));
    uint16_t* dest_reg = register_addressing(dest_result);  // 获取目标寄存器指针 *dest_reg是我自己定义的
    uint16_t src_value = 0;

    printf("\n执行 SUB %s, %s\n", dest, src);

    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // 获取目标寄存器的当前值
        src_value = identifyAM(src);
        uint16_t result = dest_value - src_value;  // 执行加法


        *dest_reg = result;  // 将结果存入目标寄存器

        // 更新标志位
        update_flags_sub(result, dest_value, src_value);

        printf("执行后 %s = %04X\n", dest, *dest_reg);

    }else {
        printf("无效的目标寄存器: %s\n", dest);
    }

}

// PUSH 指令实现
void execute_push(const char* reg) {
    uint16_t* reg_ptr = register_addressing(reg);

    printf("\n执行 PUSH %s\n", reg);

    if (reg_ptr) {
        cpu.SP -= 2;
        uint16_t addr = cpu.SP;
        memory[addr] = *reg_ptr & 0xFF;
        memory[addr + 1] = (*reg_ptr >> 8) & 0xFF;
    }

    printf("执行后状态:\n");
    display_cpu_state();
}

// POP 指令实现
void execute_pop(const char* reg) {
    uint16_t* reg_ptr = register_addressing(reg);

    printf("\n执行 POP %s\n", reg);

    if (reg_ptr) {
        uint16_t addr = cpu.SP;
        *reg_ptr = memory[addr] | (memory[addr + 1] << 8);
        cpu.SP += 2;
    }

    printf("执行后状态:\n");
    display_cpu_state();
}

void execute_or(const char* dest, const char* src) {
    uint16_t* dest_reg = register_addressing(dest);
    uint16_t src_value = 0;
    printf("\n执行or %s, %s\n", dest, src);
    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // 获取目标寄存器的当前值
        uint16_t result = dest_value | src_value;  // 执行加法
        *dest_reg = result;  // 将结果存入目标寄存器
        // 更新标志位
        //  update_flags_or(result, dest_value, src_value);
        printf("执行后 %s = %04X\n", dest, *dest_reg);
    }else {
        printf("无效的目标寄存器: %s\n", dest);
    }
    printf("执行后状态:\n");
    //display_cpu_state();
}
void execute_not(const char* dest) {
    uint16_t* dest_reg = register_addressing(dest);
    // uint16_t src_value = 0;
    printf("\n执行not %s\n", dest);
    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // 获取目标寄存器的当前值
        uint16_t result = ~dest_value;  // 执行加法
        *dest_reg = result;  // 将结果存入目标寄存器
        // 更新标志位
        //  update_flags_or(result, dest_value, src_value);
        printf("执行后 %s = %04X\n", dest, *dest_reg);
    }else {
        printf("无效的目标寄存器: %s\n", dest);
    }

    printf("执行后状态:\n");
    display_cpu_state();
}
void execute_inc(const char* dest) {
    uint16_t* dest_reg = register_addressing(dest);
    // uint16_t src_value = 0;
    printf("\n执行inc %s\n", dest);
    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // 获取目标寄存器的当前值
        uint16_t result = dest_value+1;  // 执行加法
        *dest_reg = result;  // 将结果存入目标寄存器
        // 更新标志位
        //  update_flags_or(result, dest_value, src_value);
        printf("执行后 %s = %04X\n", dest, *dest_reg);
    }else {
        printf("无效的目标寄存器: %s\n", dest);
    }
    printf("执行后状态:\n");
    display_cpu_state();
}

void execute_stc(){
    cpu.FLAGS |= 0x1;
    printf("执行后状态:\n");
    display_cpu_state();
}

void execute_clc(){
    cpu.FLAGS &= ~0x1;
    printf("执行后状态:\n");
    display_cpu_state();
}

void execute_jmp(){
    for(int i=0;i<MAX_SYMBOLS;i++){
        if(strcmp(symbol_table[i].name,"START")==0){
            cpu.IP=symbol_table[i].address;}
    }
    printf("执行后状态:\n");
    display_cpu_state();
}
void execute_int(const char *dest) {
    if (dest==0x21)
    {
        uint16_t* dest_reg = register_addressing("AH");
        uint16_t dest_value = *dest_reg;
        if (dest_value == 0x4c) {
            printf("退出模拟器。\n");
        }
    }

}
void MOVSB() {
    uint16_t* dest_reg1 = register_addressing("SI");
    uint16_t* dest_reg2 = register_addressing("DI");
    uint16_t* dest_reg3 = register_addressing("DS");
    uint16_t* dest_reg4 = register_addressing("ES");
    uint16_t address1,address2;
    uint16_t dest_value1 = *dest_reg1;  // 获取目标寄存器的当前值
    uint16_t dest_value2 = *dest_reg2;  // 获取目标寄存器的当前值
    uint16_t dest_value3 = *dest_reg3;  // 获取目标寄存器的当前值
    uint16_t dest_value4 = *dest_reg4;  // 获取目标寄存器的当前值
    address1 = dest_value3 * 10 + dest_value1;
    address2 = dest_value4 * 10 + dest_value2;
    memory[address2] = memory[address1];
    *dest_reg1 = *dest_reg1 + 1;
    *dest_reg2 = *dest_reg1 + 1;
}
//从 ES:SI 指向的内存地址读取一个字节。
//将读取的字节写入 ES : DI 指向的内存地址。
//SI 和 DI 递增
void update_flags_cmpsb(uint16_t result) {

    // 零标志（ZF）：结果是否为 0
    if (result == 0) {
        cpu.FLAGS |= (1 << 6);  // 设置 ZF
    }
    else {
        cpu.FLAGS &= ~(1 << 6);  // 清除 ZF
    }

}

void CMPSB() {
    uint16_t* dest_reg1 = register_addressing("SI");
    uint16_t* dest_reg2 = register_addressing("DI");
    uint16_t* dest_reg3 = register_addressing("DS");
    uint16_t* dest_reg4 = register_addressing("ES");
    uint16_t address1,address2;
    int result = 1;
    uint16_t dest_value1 = *dest_reg1;  // 获取目标寄存器的当前值
    uint16_t dest_value2 = *dest_reg2;  // 获取目标寄存器的当前值
    uint16_t dest_value3 = *dest_reg3;  // 获取目标寄存器的当前值
    uint16_t dest_value4 = *dest_reg4;  // 获取目标寄存器的当前值
    address1 = dest_value3 * 10 + dest_value1;
    address2 = dest_value4 * 10 + dest_value2;

    if( memory[address2] == memory[address1])  result = 0;
    update_flags_cmpsb(result);
    *dest_reg1 = *dest_reg1 + 1;
    *dest_reg2 = *dest_reg1 + 1;
//如果两个字节相等，则ZF被设置为1；否则，ZF被设置为0。
}
void execute_and(const char* dest, const char* src) {
    uint16_t* dest_reg = register_addressing(dest);
    uint16_t src_value = 0;

    printf("\n执行and %s, %s\n", dest, src);

    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // 获取目标寄存器的当前值
        uint16_t result = dest_value & src_value;  // 执行加法

        *dest_reg = result;  // 将结果存入目标寄存器

        // 更新标志位
        //  update_flags_or(result, dest_value, src_value);

        printf("执行后 %s = %04X\n", dest, *dest_reg);
    }else {
        printf("无效的目标寄存器: %s\n", dest);
    }

    printf("执行后状态:\n");
    //display_cpu_state();
}
void execute_xor(const char* dest, const char* src) {
    uint16_t* dest_reg = register_addressing(dest);
    uint16_t src_value = 0;

    printf("\n执行xor %s, %s\n", dest, src);

    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // 获取目标寄存器的当前值
        uint16_t result = dest_value ^ src_value;

        *dest_reg = result;  // 将结果存入目标寄存器

        // 更新标志位
        //  update_flags_or(result, dest_value, src_value);

        printf("执行后 %s = %04X\n", dest, *dest_reg);
    }else {
        printf("无效的目标寄存器: %s\n", dest);
    }

    printf("执行后状态:\n");
    //display_cpu_state();
}

// 查找符号表中的符号地址
uint32_t get_symbol_address(const char* symbol_name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, symbol_name) == 0) {
            return symbol_table[i].address;  // 返回符号的地址
        }
    }
    printf("符号未找到: %s\n", symbol_name);
    return 0;  // 如果符号未找到，返回地址 0
}

void execute_lea(const char* dest, const char* src) {
    uint16_t* dest_reg = register_addressing(dest);  // 获取目标寄存器指针

    printf("\n执行 LEA %s, %s\n", dest, src);

    if (dest_reg) {
        uint32_t effective_address = 0;

        // 检查源操作数是否是符号名
        if (get_symbol_address(src) != 0) {
            effective_address = get_symbol_address(src);  // 获取符号地址
        } else {
            printf("无效的符号或地址表达式: %s\n", src);
            return;
        }

        *dest_reg = (uint16_t)effective_address;  // 将地址存入目标寄存器

        printf("将地址 %04X 加载到 %s\n", effective_address, dest);
    } else {
        printf("无效的目标寄存器: %s\n", dest);
    }

}

uint16_t get_symbol_value(const char* name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            uint16_t low_byte = memory[symbol_table[i].address];
            uint16_t high_byte = memory[symbol_table[i].address + 1];
            return (high_byte << 8) | low_byte;  // 合成 16 位数据
        }
    }
    printf("符号 %s 未找到。\n", name);
    return 0;  // 未找到符号时返回 0
}
// 寻址方式实现


uint16_t* register_addressing(const char* reg) {
    if (strcmp(reg, "AX") == 0) return &cpu.AX;
    if (strcmp(reg, "BX") == 0) return &cpu.BX;
    if (strcmp(reg, "CX") == 0) return &cpu.CX;
    if (strcmp(reg, "DX") == 0) return &cpu.DX;
    if (strcmp(reg, "AL") == 0) return &cpu.AL;
    if (strcmp(reg, "AH") == 0) return &cpu.AH;
    if (strcmp(reg, "BL") == 0) return &cpu.BL;
    if (strcmp(reg, "BH") == 0) return &cpu.BH;
    if (strcmp(reg, "CL") == 0) return &cpu.CL;
    if (strcmp(reg, "CH") == 0) return &cpu.CH;
    if (strcmp(reg, "DL") == 0) return &cpu.DL;
    if (strcmp(reg, "DH") == 0) return &cpu.DH;
    if (strcmp(reg, "SP") == 0) return &cpu.SP;
    if (strcmp(reg, "BP") == 0) return &cpu.BP;
    if (strcmp(reg, "SI") == 0) return &cpu.SI;
    if (strcmp(reg, "DI") == 0) return &cpu.DI;

    // 段寄存器
    if (strcmp(reg, "ES") == 0) return &cpu.ES;
    if (strcmp(reg, "CS") == 0) return &cpu.CS;
    if (strcmp(reg, "SS") == 0) return &cpu.SS;
    if (strcmp(reg, "DS") == 0) return &cpu.DS;

    // 检查是否是 memory[index] 形式的字符串
    if (strncmp(reg, "memory[", 7) == 0) {
        char* start = strchr(reg, '[');  // 找到 '[' 的位置
        char* end = strchr(reg, ']');    // 找到 ']' 的位置

        if (start && end && end > start) {
            char index_str[10] = {0};
            strncpy(index_str, start + 1, end - start - 1);  // 提取 index 部分

            // 使用 strtol 处理十六进制和十进制
            int index = (int)strtol(index_str, NULL, 0);  // 自动检测进制
            if (index >= 0 && index < MEMORY_SIZE) {
                return (uint16_t*)&memory[index];  // 返回指向 memory[index] 的指针
            } else {
                printf("内存索引 %d 超出范围。\n", index);
                return NULL;
            }
        }
    }
    return NULL;
}

int find_symbol(const char* symbol_name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, symbol_name) == 0) {
            return i;  // 返回符号的索引
        }
    }
    return -1;  // 如果未找到符号，返回 -1
}

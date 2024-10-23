#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ctype.h"
#include "addressing.h"
// ����ָ�������
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

// ����Ѱַ��ʽ��ʵ��
uint16_t immediate_addressing(const char* operand);
uint16_t* register_addressing(const char* reg);
uint16_t get_symbol_value(const char* name);

// ��־λ�����ڼ�¼�Ƿ��ڴ���λ����ݶ���
int in_data_segment = 0;  // �Ƿ������ݶ���
int in_code_segment = 0;  // �Ƿ��ڴ������
void handle_assume(const char* instruction) {
    char segment_name[50] = {0};  // �洢����
    char* token = NULL;

    // ��ȡ CS ����
    token = strstr(instruction, "CS:");
    if (token) {
        sscanf(token, "CS:%[^,]", segment_name);  // ��ȡ CS ����
        uint32_t address = get_physical_address(cpu.CS, 0);
        add_symbol(segment_name, address, cpu.CS);  // ��ӷ��ű�
        printf("��ӷ��ţ�%s, ��ַ��%04X, ֵ��%04X\n", segment_name, address, cpu.CS);
    }

    // ��ȡ DS ����
    token = strstr(instruction, "DS:");
    if (token) {
        sscanf(token, "DS:%[^,]", segment_name);  // ��ȡ DS ����
        uint32_t address = get_physical_address(cpu.DS, 0);
        add_symbol(segment_name, address, cpu.DS);  // ��ӷ��ű�
        printf("��ӷ��ţ�%s, ��ַ��%04X, ֵ��%04X\n", segment_name, address, cpu.DS);
    }

    // ���� SS ��������
    if (strstr(instruction, "SS:")) {
        printf("ʶ�� SS �Σ�����ʱ������\n");
    }
}

// �������ִ�к���
void decode_and_execute(const char* instruction) {
    // ���ȴ�������ָ���α�ʶ
    if (strcmp(instruction, "DATA SEGMENT") == 0) {
        printf("�������ݶΡ�\n");
        in_data_segment = 1;  // ��ǽ������ݶ�
        return;
    }
    if (strcmp(instruction, "DATA ENDS") == 0) {
        printf("�˳����ݶΡ�\n");
        in_data_segment = 0;  // ����˳����ݶ�
        return;
    }
    if (strcmp(instruction, "CODE SEGMENT") == 0) {
        printf("�������Ρ�\n");
        in_code_segment = 1;  // ��ǽ�������
        return;
    }
    if (strcmp(instruction, "CODE ENDS") == 0) {
        printf("�˳�����Ρ�\n");
        in_code_segment = 0;  // ����˳������
        return;
    }
    // ���� ASSUME ָ��
    if (strncmp(instruction, "ASSUME", 6) == 0) {
        printf("ʶ��ASSUME����\n");
        handle_assume(instruction);
        return;
    }
    // ��������ݶ��ڣ��������ݶ���ָ��
    if (in_data_segment) {
        char name[50], directive[10], value[50];
        sscanf(instruction, "%s %s %s", name, directive, value);
        define_data(name, directive, value);
        return;
    }

    // �����Ͳ��ָ��������ڴ�����ڵ�ָ�
    char op[10] = {0}, arg1[10] = {0}, arg2[10] = {0};
    char* token = strtok((char*)instruction, " ,");
    if (token) strncpy(op, token, sizeof(op) - 1);

    token = strtok(NULL, " ,");
    if (token) strncpy(arg1, token, sizeof(arg1) - 1);

    token = strtok(NULL, " ,");
    if (token) strncpy(arg2, token, sizeof(arg2) - 1);

    printf("\n����ָ�%s %s, %s\n", op, arg1, arg2);

    // �ڴ������ִ��ָ��
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
            execute_jmp(arg1);  // ����JMPָ�ʹ��Ŀ���ַ
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
            // ����δ֪���ţ�������Ϊ������
            printf("ʶ���·�����: %s\n", instruction);

            // ��ȡ��ǰָ��� IP ��Ϊ���ŵĵ�ַ
            uint32_t address = cpu.IP;

            // ���·�����������ű�Ĭ��ֵΪ 0
            add_symbol(instruction, address, 0);

            printf("��ӷ���: %s, ��ַ: %04X, Ĭ��ֵ: 0000\n", instruction, address);
        }
    } else {
        printf("δʶ���ָ���ζ������%s\n", instruction);
    }
}


int is_immediate(const char* operand) {
    // 1. ����Ƿ���ʮ���������������� 0x �� 0X ��ͷ��
    if ((operand[0] == '0') && (operand[1] == 'x' || operand[1] == 'X')) {
        return 1;
    }

    // 2. ����Ƿ���ʮ�������֣������Ż����ţ�
    int i = 0;
    if (operand[0] == '-' || operand[0] == '+') {
        i = 1;  // ��������λ
    }
    for (; operand[i] != '\0'; i++) {
        if (!isdigit(operand[i])) {
            return 0;  // ����з������ַ�������ʮ������
        }
    }
    return 1;  // �����ַ��������֣���Ϊʮ������
}

// 3. ����Ƿ�Ϊ�ַ���������������������
int is_char_constant(const char* operand) {
    // ���ַ����� 'A'
    if (operand[0] == '\'' && operand[2] == '\'' && operand[3] == '\0') {
        return 1;  // ���ַ�����
    }
    // ˫�ַ����� 'AB'
    if (operand[0] == '\'' && operand[3] == '\'' && operand[4] == '\0') {
        return 2;  // ˫�ַ�����
    }
    return 0;  // ���ַ�����
}

//immediate_addressing() ������ʹ������ַ����ĸ�ʽ��ѡ����ʵĽ��ƽ���ת����
uint16_t immediate_addressing(const char* operand) {
    // 1. �����ַ�����
    if (is_char_constant(operand) == 1) {
        return (uint16_t)operand[1];  // ���ص��ַ��� ASCII ��
    }
    // 2. ����˫�ַ��������������ַ��� ASCII ����ϳ�һ�� 16 λ����
    if (is_char_constant(operand) == 2) {
        return ((uint16_t)operand[1] << 8) | (uint16_t)operand[2];
    }
    // 3. ʮ�����ƺ�ʮ��������������
    if ((operand[0] == '0') && (operand[1] == 'x' || operand[1] == 'X')) {
        return (uint16_t)strtol(operand, NULL, 16);  // ʮ������ת��
    }
    return (uint16_t)strtol(operand, NULL, 10);  // ʮ����ת��
}
// MOV ָ��ʵ��
void execute_mov(const char* dest, const char* src) {
    char dest_result[50];
    getString(dest,dest_result,sizeof(dest_result));
    uint16_t* dest_reg = register_addressing(dest_result);  // ��ȡĿ��Ĵ���ָ�� *dest_reg�����Լ������
    uint16_t src_value = 0;

    printf("\nִ�� MOV %s, %s\n", dest, src);
    // ���Ŀ��Ĵ����Ƿ���Ч
    if (dest_reg) {
        //����Ѱַ֮���ȡ������ src_value *dest_reg
        src_value = identifyAM(src);
        *dest_reg = src_value;
    }else {
            printf("��Ч��Ŀ��Ĵ���: %s\n", dest);
    }
    // ��ӡִ�к�� CPU ״̬����ѡ��
    // printf("ִ�к�״̬:\n");
    // display_cpu_state();
}

// ���±�־λ����
void update_flags_add(uint16_t result, uint16_t dest_value, uint16_t src_value) {
    // ��λ��־��CF�������λ��λ
    if (result < dest_value) {
        cpu.FLAGS |= 0x1;  // ���� CF
    } else {
        cpu.FLAGS &= ~0x1;  // ��� CF
    }

    // �����־��OF�����������
    uint16_t sign_dest = dest_value & 0x8000;
    uint16_t sign_src = src_value & 0x8000;
    uint16_t sign_res = result & 0x8000;
    if ((sign_dest == sign_src) && (sign_dest != sign_res)) {
        cpu.FLAGS |= (1 << 11);  // ���� OF
    } else {
        cpu.FLAGS &= ~(1 << 11);  // ��� OF
    }

    // ���־��ZF��������Ƿ�Ϊ 0
    if (result == 0) {
        cpu.FLAGS |= (1 << 6);  // ���� ZF
    } else {
        cpu.FLAGS &= ~(1 << 6);  // ��� ZF
    }

    // ���ű�־��SF�������λ����λ
    if (result & 0x8000) {
        cpu.FLAGS |= (1 << 7);  // ���� SF
    } else {
        cpu.FLAGS &= ~(1 << 7);  // ��� SF
    }

    // ������λ��־��AF������ 4 λ��λ
    if (((dest_value & 0xF) + (src_value & 0xF)) > 0xF) {
        cpu.FLAGS |= (1 << 4);  // ���� AF
    } else {
        cpu.FLAGS &= ~(1 << 4);  // ��� AF
    }
}
// ADD ָ��ʵ��
void execute_add(const char* dest, const char* src) {
    char dest_result[50];
    getString(dest,dest_result,sizeof(dest_result));
    uint16_t* dest_reg = register_addressing(dest_result);  // ��ȡĿ��Ĵ���ָ�� *dest_reg�����Լ������
    uint16_t src_value = 0;

    printf("\nִ�� ADD %s, %s\n", dest, src);

    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // ��ȡĿ��Ĵ����ĵ�ǰֵ

        src_value = identifyAM(src);
        printf("�������Դ������ֵ: %04X\n", src_value);
        printf("Ŀ��Ĵ�����ʼֵ: %04X, Դ������ֵ: %04X\n", dest_value, src_value);

        uint16_t result = dest_value + src_value;  // ִ�мӷ�

        *dest_reg = result;  // ���������Ŀ��Ĵ���

        // ���±�־λ
        update_flags_add(result, dest_value, src_value);

        printf("ִ�к� %s = %04X\n", dest, *dest_reg);
    }else {
        printf("��Ч��Ŀ��Ĵ���: %s\n", dest);
    }

    //printf("ִ�к�״̬:\n");
    //display_cpu_state();
}

// ���±�־λ����
void update_flags_sub(uint16_t result, uint16_t dest_value, uint16_t src_value) {
    // ��λ��־��CF������λ��⣨�� dest < src ʱ���� CF��
    if (dest_value < src_value) {
        cpu.FLAGS |= 0x1;  // ���� CF
    } else {
        cpu.FLAGS &= ~0x1;  // ��� CF
    }

    // �����־��OF�������������������λ��һ��ʱ��
    uint16_t sign_dest = dest_value & 0x8000;
    uint16_t sign_src = src_value & 0x8000;
    uint16_t sign_res = result & 0x8000;
    if ((sign_dest != sign_src) && (sign_dest != sign_res)) {
        cpu.FLAGS |= (1 << 11);  // ���� OF
    } else {
        cpu.FLAGS &= ~(1 << 11);  // ��� OF
    }

    // ���־��ZF��������Ƿ�Ϊ 0
    if (result == 0) {
        cpu.FLAGS |= (1 << 6);  // ���� ZF
    } else {
        cpu.FLAGS &= ~(1 << 6);  // ��� ZF
    }

    // ���ű�־��SF��������ķ���λ�����λ��
    if (result & 0x8000) {
        cpu.FLAGS |= (1 << 7);  // ���� SF
    } else {
        cpu.FLAGS &= ~(1 << 7);  // ��� SF
    }

    // ������λ��־��AF�������� 4 λ�Ƿ��λ
    if (((dest_value & 0xF) - (src_value & 0xF)) < 0) {
        cpu.FLAGS |= (1 << 4);  // ���� AF
    } else {
        cpu.FLAGS &= ~(1 << 4);  // ��� AF
    }
}

// SUB ָ��ʵ��
void execute_sub(const char* dest, const char* src) {
    char dest_result[50];
    getString(dest,dest_result,sizeof(dest_result));
    uint16_t* dest_reg = register_addressing(dest_result);  // ��ȡĿ��Ĵ���ָ�� *dest_reg�����Լ������
    uint16_t src_value = 0;

    printf("\nִ�� SUB %s, %s\n", dest, src);

    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
        src_value = identifyAM(src);
        uint16_t result = dest_value - src_value;  // ִ�мӷ�


        *dest_reg = result;  // ���������Ŀ��Ĵ���

        // ���±�־λ
        update_flags_sub(result, dest_value, src_value);

        printf("ִ�к� %s = %04X\n", dest, *dest_reg);

    }else {
        printf("��Ч��Ŀ��Ĵ���: %s\n", dest);
    }

}

// PUSH ָ��ʵ��
void execute_push(const char* reg) {
    uint16_t* reg_ptr = register_addressing(reg);

    printf("\nִ�� PUSH %s\n", reg);

    if (reg_ptr) {
        cpu.SP -= 2;
        uint16_t addr = cpu.SP;
        memory[addr] = *reg_ptr & 0xFF;
        memory[addr + 1] = (*reg_ptr >> 8) & 0xFF;
    }

    printf("ִ�к�״̬:\n");
    display_cpu_state();
}

// POP ָ��ʵ��
void execute_pop(const char* reg) {
    uint16_t* reg_ptr = register_addressing(reg);

    printf("\nִ�� POP %s\n", reg);

    if (reg_ptr) {
        uint16_t addr = cpu.SP;
        *reg_ptr = memory[addr] | (memory[addr + 1] << 8);
        cpu.SP += 2;
    }

    printf("ִ�к�״̬:\n");
    display_cpu_state();
}

void execute_or(const char* dest, const char* src) {
    uint16_t* dest_reg = register_addressing(dest);
    uint16_t src_value = 0;
    printf("\nִ��or %s, %s\n", dest, src);
    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
        uint16_t result = dest_value | src_value;  // ִ�мӷ�
        *dest_reg = result;  // ���������Ŀ��Ĵ���
        // ���±�־λ
        //  update_flags_or(result, dest_value, src_value);
        printf("ִ�к� %s = %04X\n", dest, *dest_reg);
    }else {
        printf("��Ч��Ŀ��Ĵ���: %s\n", dest);
    }
    printf("ִ�к�״̬:\n");
    //display_cpu_state();
}
void execute_not(const char* dest) {
    uint16_t* dest_reg = register_addressing(dest);
    // uint16_t src_value = 0;
    printf("\nִ��not %s\n", dest);
    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
        uint16_t result = ~dest_value;  // ִ�мӷ�
        *dest_reg = result;  // ���������Ŀ��Ĵ���
        // ���±�־λ
        //  update_flags_or(result, dest_value, src_value);
        printf("ִ�к� %s = %04X\n", dest, *dest_reg);
    }else {
        printf("��Ч��Ŀ��Ĵ���: %s\n", dest);
    }

    printf("ִ�к�״̬:\n");
    display_cpu_state();
}
void execute_inc(const char* dest) {
    uint16_t* dest_reg = register_addressing(dest);
    // uint16_t src_value = 0;
    printf("\nִ��inc %s\n", dest);
    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
        uint16_t result = dest_value+1;  // ִ�мӷ�
        *dest_reg = result;  // ���������Ŀ��Ĵ���
        // ���±�־λ
        //  update_flags_or(result, dest_value, src_value);
        printf("ִ�к� %s = %04X\n", dest, *dest_reg);
    }else {
        printf("��Ч��Ŀ��Ĵ���: %s\n", dest);
    }
    printf("ִ�к�״̬:\n");
    display_cpu_state();
}

void execute_stc(){
    cpu.FLAGS |= 0x1;
    printf("ִ�к�״̬:\n");
    display_cpu_state();
}

void execute_clc(){
    cpu.FLAGS &= ~0x1;
    printf("ִ�к�״̬:\n");
    display_cpu_state();
}

void execute_jmp(){
    for(int i=0;i<MAX_SYMBOLS;i++){
        if(strcmp(symbol_table[i].name,"START")==0){
            cpu.IP=symbol_table[i].address;}
    }
    printf("ִ�к�״̬:\n");
    display_cpu_state();
}
void execute_int(const char *dest) {
    if (dest==0x21)
    {
        uint16_t* dest_reg = register_addressing("AH");
        uint16_t dest_value = *dest_reg;
        if (dest_value == 0x4c) {
            printf("�˳�ģ������\n");
        }
    }

}
void MOVSB() {
    uint16_t* dest_reg1 = register_addressing("SI");
    uint16_t* dest_reg2 = register_addressing("DI");
    uint16_t* dest_reg3 = register_addressing("DS");
    uint16_t* dest_reg4 = register_addressing("ES");
    uint16_t address1,address2;
    uint16_t dest_value1 = *dest_reg1;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
    uint16_t dest_value2 = *dest_reg2;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
    uint16_t dest_value3 = *dest_reg3;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
    uint16_t dest_value4 = *dest_reg4;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
    address1 = dest_value3 * 10 + dest_value1;
    address2 = dest_value4 * 10 + dest_value2;
    memory[address2] = memory[address1];
    *dest_reg1 = *dest_reg1 + 1;
    *dest_reg2 = *dest_reg1 + 1;
}
//�� ES:SI ָ����ڴ��ַ��ȡһ���ֽڡ�
//����ȡ���ֽ�д�� ES : DI ָ����ڴ��ַ��
//SI �� DI ����
void update_flags_cmpsb(uint16_t result) {

    // ���־��ZF��������Ƿ�Ϊ 0
    if (result == 0) {
        cpu.FLAGS |= (1 << 6);  // ���� ZF
    }
    else {
        cpu.FLAGS &= ~(1 << 6);  // ��� ZF
    }

}

void CMPSB() {
    uint16_t* dest_reg1 = register_addressing("SI");
    uint16_t* dest_reg2 = register_addressing("DI");
    uint16_t* dest_reg3 = register_addressing("DS");
    uint16_t* dest_reg4 = register_addressing("ES");
    uint16_t address1,address2;
    int result = 1;
    uint16_t dest_value1 = *dest_reg1;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
    uint16_t dest_value2 = *dest_reg2;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
    uint16_t dest_value3 = *dest_reg3;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
    uint16_t dest_value4 = *dest_reg4;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
    address1 = dest_value3 * 10 + dest_value1;
    address2 = dest_value4 * 10 + dest_value2;

    if( memory[address2] == memory[address1])  result = 0;
    update_flags_cmpsb(result);
    *dest_reg1 = *dest_reg1 + 1;
    *dest_reg2 = *dest_reg1 + 1;
//��������ֽ���ȣ���ZF������Ϊ1������ZF������Ϊ0��
}
void execute_and(const char* dest, const char* src) {
    uint16_t* dest_reg = register_addressing(dest);
    uint16_t src_value = 0;

    printf("\nִ��and %s, %s\n", dest, src);

    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
        uint16_t result = dest_value & src_value;  // ִ�мӷ�

        *dest_reg = result;  // ���������Ŀ��Ĵ���

        // ���±�־λ
        //  update_flags_or(result, dest_value, src_value);

        printf("ִ�к� %s = %04X\n", dest, *dest_reg);
    }else {
        printf("��Ч��Ŀ��Ĵ���: %s\n", dest);
    }

    printf("ִ�к�״̬:\n");
    //display_cpu_state();
}
void execute_xor(const char* dest, const char* src) {
    uint16_t* dest_reg = register_addressing(dest);
    uint16_t src_value = 0;

    printf("\nִ��xor %s, %s\n", dest, src);

    if (dest_reg) {
        uint16_t dest_value = *dest_reg;  // ��ȡĿ��Ĵ����ĵ�ǰֵ
        uint16_t result = dest_value ^ src_value;

        *dest_reg = result;  // ���������Ŀ��Ĵ���

        // ���±�־λ
        //  update_flags_or(result, dest_value, src_value);

        printf("ִ�к� %s = %04X\n", dest, *dest_reg);
    }else {
        printf("��Ч��Ŀ��Ĵ���: %s\n", dest);
    }

    printf("ִ�к�״̬:\n");
    //display_cpu_state();
}

// ���ҷ��ű��еķ��ŵ�ַ
uint32_t get_symbol_address(const char* symbol_name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, symbol_name) == 0) {
            return symbol_table[i].address;  // ���ط��ŵĵ�ַ
        }
    }
    printf("����δ�ҵ�: %s\n", symbol_name);
    return 0;  // �������δ�ҵ������ص�ַ 0
}

void execute_lea(const char* dest, const char* src) {
    uint16_t* dest_reg = register_addressing(dest);  // ��ȡĿ��Ĵ���ָ��

    printf("\nִ�� LEA %s, %s\n", dest, src);

    if (dest_reg) {
        uint32_t effective_address = 0;

        // ���Դ�������Ƿ��Ƿ�����
        if (get_symbol_address(src) != 0) {
            effective_address = get_symbol_address(src);  // ��ȡ���ŵ�ַ
        } else {
            printf("��Ч�ķ��Ż��ַ���ʽ: %s\n", src);
            return;
        }

        *dest_reg = (uint16_t)effective_address;  // ����ַ����Ŀ��Ĵ���

        printf("����ַ %04X ���ص� %s\n", effective_address, dest);
    } else {
        printf("��Ч��Ŀ��Ĵ���: %s\n", dest);
    }

}

uint16_t get_symbol_value(const char* name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            uint16_t low_byte = memory[symbol_table[i].address];
            uint16_t high_byte = memory[symbol_table[i].address + 1];
            return (high_byte << 8) | low_byte;  // �ϳ� 16 λ����
        }
    }
    printf("���� %s δ�ҵ���\n", name);
    return 0;  // δ�ҵ�����ʱ���� 0
}
// Ѱַ��ʽʵ��


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

    // �μĴ���
    if (strcmp(reg, "ES") == 0) return &cpu.ES;
    if (strcmp(reg, "CS") == 0) return &cpu.CS;
    if (strcmp(reg, "SS") == 0) return &cpu.SS;
    if (strcmp(reg, "DS") == 0) return &cpu.DS;

    // ����Ƿ��� memory[index] ��ʽ���ַ���
    if (strncmp(reg, "memory[", 7) == 0) {
        char* start = strchr(reg, '[');  // �ҵ� '[' ��λ��
        char* end = strchr(reg, ']');    // �ҵ� ']' ��λ��

        if (start && end && end > start) {
            char index_str[10] = {0};
            strncpy(index_str, start + 1, end - start - 1);  // ��ȡ index ����

            // ʹ�� strtol ����ʮ�����ƺ�ʮ����
            int index = (int)strtol(index_str, NULL, 0);  // �Զ�������
            if (index >= 0 && index < MEMORY_SIZE) {
                return (uint16_t*)&memory[index];  // ����ָ�� memory[index] ��ָ��
            } else {
                printf("�ڴ����� %d ������Χ��\n", index);
                return NULL;
            }
        }
    }
    return NULL;
}

int find_symbol(const char* symbol_name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, symbol_name) == 0) {
            return i;  // ���ط��ŵ�����
        }
    }
    return -1;  // ���δ�ҵ����ţ����� -1
}

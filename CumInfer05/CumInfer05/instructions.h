//
// Created by 孟令圆 on 2024/10/15.
//

#ifndef CUMINFER05_INSTRUCTIONS_H
#define CUMINFER05_INSTRUCTIONS_H
#include <stdio.h>
int decode_and_execute(const char* instruction);// 张 孟
void update_flags_add(uint16_t result, uint16_t dest_value, uint16_t src_value);
void execute_mov(const char* dest, const char* src);
void update_flags_sub(uint16_t result, uint16_t dest_value, uint16_t src_value);
void execute_add(const char* dest, const char* src);
void execute_sub(const char* dest, const char* src);
void execute_push(const char* reg);
void execute_pop(const char* reg);
void execute_lea(const char* dest, const char* src);
void execute_and(const char* dest, const char* src);
void execute_xor(const char* dest, const char* src);//张
void execute_or(const char* dest, const char* src);//张
void execute_not(const char* reg);//张
void execute_inc(const char* reg);//张
void execute_stc();//张
void execute_clc();//张
void execute_jmp();
void execute_int(const char* reg);//吕 孙
void CMPSB();//吕 孙
void MOVSB();//吕 孙
uint32_t get_symbol_address(const char* symbol_name);
uint16_t* register_addressing(const char* reg);
uint16_t get_symbol_value(const char* name);
int is_char_constant(const char* operand);
#endif //CUMINFER05_INSTRUCTIONS_H

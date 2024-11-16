//
// Created by 吕思梦 孙嘉特 on 2024/10/24.
//

#ifndef CUMINFER05_ADDRESSING_H
#define CUMINFER05_ADDRESSING_H
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

bool endsWith(const char *str, const char *suffix);
uint16_t  identifyAM(const char* src);
void getString(const char* array, char* resultBuffer, size_t bufferSize);
#endif //CUMINFER05_ADDRESSING_H

cmake_minimum_required(VERSION 3.24)
project(CumInfer05 C)

set(CMAKE_C_STANDARD 11)

add_executable(CumInfer05 main.c cpu.h cpu.c instructions.h instructions.c eu.h eu.c biu.h biu.c addressing.h addressing.c)

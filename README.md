# C语言设计8086CPU

#### 前言：设计并实现一个简化版的8086 CPU模拟器，该模拟器需能够处理并仿真MOV（数据传送）、ADD（加法）、以及HLT（停机）等基本指令的执行过程。通过模拟CPU的主要组成部分（如寄存器、指令寄存器、程序计数器、控制单元以及数据总线），加深对计算机体系结构和工作原理的理解。

之前草草写了一个8086系统的设计 后来发现有很多不足的地方，现在开始逐一更改更新。

初步分为以下几部分:

#### 寄存器

实现至少8个通用寄存器（如AX, BX, CX, DX, SP, BP, SI, DI）和一个指令寄存器（IR）、一个程序计数器（PC/IP，Instruction Pointer）。寄存器大小应设定为16位。

```c
typedef struct {
    uint16_t AX, BX, CX, DX;
    uint8_t AL,AH,BL,BH,CL,CH,DL,DH;//这些是八位寄存器
    uint16_t SP, BP, SI, DI;
    uint16_t IP;
    uint16_t CS, DS, ES, SS;
    uint16_t FLAGS;
    char IR[50];  // 指令寄存器，用于存储当前指令
} Registers;
```

#### 数据总线

定义一条16位的数据总线，用于CPU内部及CPU与外部存储器之间的数据传输。

```c
typedef struct {
    int address_bus;//在实际中，地址总线是20位的，C语言无法定义20位无符号数，故这里采用int，基本形式不变。
    uint16_t data_bus;
    uint16_t control_bus;
} Bus;
```

#### 控制单元

设计逻辑以解析指令，控制指令的执行流程，包括从内存中读取指令、解码指令、执行指令以及更新PC等。

#### 内存

实现一个简单的内存模型，能够存储指令和数据，至少包含足够空间以加载和执行示例程序。

这里我们将指令内存和数据内存分开存储

##### 数据内存

```c
#define MEMORY_SIZE 0x100000
extern uint8_t memory[MEMORY_SIZE];  //  内存
```

##### 指令内存

```c
#define MAX_INSTRUCTIONS 1024  // 支持最多 1024 条指令
extern char instruction_memory[MAX_INSTRUCTIONS][50];  // 每条指令最大长度50字节
extern int instruction_count;  // 当前存储的指令数量
```

#### 指令集

##### MOV指令

```c
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
```

##### ADD指令

```c
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
```

##### HIT指令

```c
while (1) {
    printf("\n> ");
    fgets(instruction, sizeof(instruction), stdin);  // 获取用户输入
    instruction[strcspn(instruction, "\n")] = '\0';  // 去除换行符

    if (strcmp(instruction, "HIT") == 0) {
        printf("退出模拟器。\n");
        break;
    }
```

在主函数中实现的HIT指令。

## 测试代码段

```
DATA SEGMENT
DATA ENDS
CODE SEGMENT
ASSUME CS:CODE, DS:DATA, SS:STACK
MOV AX,5
MOV BX,AX
ADD AX,BX
CODE ENDS
HIT
```

在这里，数据段没用进行任何定义。只是进行简单的代码运行测试。

## 测试结果
![Homework01](https://github.com/user-attachments/assets/386977ae-e12a-412d-9f14-6e6ae47acbda)


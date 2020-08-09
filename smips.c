// Nguyen Hoang z52157125

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#define MAX_REGISTERS 32
#define MAX_INSTRUCTIONS 1000

void print_instructions(int i, int command);
void print_instruction_subset(int32_t instr_mask, int32_t last_six);
int print_output(int registers[MAX_REGISTERS], int commands[MAX_INSTRUCTIONS], int i);
void print_registers(int registers[MAX_REGISTERS]);
int check_invalid_instruction(int commands[MAX_INSTRUCTIONS], int i);

int main(int argc, char *argv[]) {
    // check for file input
    if (argc < 1) {
        fprintf(stderr, "please enter a file to read.\n");
        exit(EXIT_FAILURE);
    }
    // open file
    FILE *fp = fopen(argv[1], "r");
    // check if file exists
    if (fp == NULL) {
        fprintf(stderr, "File does not exist.\n");
        exit(EXIT_FAILURE);
    }

    int commands[MAX_INSTRUCTIONS];
    int i = 0;

    // scan the instructions to an array
    while ((fscanf(fp, "%x", &commands[i]) != EOF) && (i < MAX_INSTRUCTIONS)) {
        i++;
    }

    // check for invalid instruction code
    int error = check_invalid_instruction(commands, i);
    if (error != 1001) {
        printf("%s:%d: invalid instruction code: %08X\n", argv[1], error, commands[error]);
        exit(EXIT_FAILURE);
    }

    // print out program instructions
    printf("Program\n");
    for (int j = 0; j < i; j++) {
        print_instructions(j, commands[j]);
        printf("\n");
    }

    // Initialise registers to 0
    int registers[MAX_REGISTERS];
    for (int j = 0; j < MAX_REGISTERS; j++) {
        registers[j] = 0;
    }

    // print the output of the code
    printf("Output\n");
    for (int j = 0; j < i; j++) {
        if (print_output(registers, commands, j) == 0)
            break;
    }

    // print registers after execution of
    // the program
    printf("Registers After Execution\n");
    print_registers(registers);
    return 0;
}

void print_instructions(int i, int command) {
    int32_t instr_mask = command >> 26;
    int32_t s = (command >> 21) & 0x1F;
    int32_t t = (command >> 16) & 0x1F;
    int32_t d = (command >> 11) & 0x1F;
    int32_t last_six = command & 0x3F;
    int16_t integer_mask = command & 0xFFFF;
    // check for correct style indentation 
    if (i < 10) {
        printf("  %d: ", i);
    } else {
        printf(" %d: ", i); 
    }
    // if instruction is syscall, print then return
    if (command == 0xC) {
        printf("syscall"); 
        return;
    }
    // if any other instruction print out correspoding instruction
    print_instruction_subset(instr_mask, last_six);

    // print out instruction register values
    if (instr_mask == 0 || instr_mask == 0x1C) {
        printf("$%d, $%d, $%d", d, s, t);
    } else if (instr_mask == 0x4 || instr_mask == 0x5) {
        printf("$%d, $%d, %d", s, t, integer_mask);
    } else if (instr_mask == 0xF) {
        printf("$%d, %d", t, integer_mask);
    } else {
        printf("$%d, $%d, %d", t, s, integer_mask);
    }
    return;
}

void print_instruction_subset(int32_t instr_mask, int32_t last_six) {
    switch (instr_mask) {
        case 0x0:
            switch (last_six) {
                case 0x20:
                    printf("add  ");
                    break;
                case 0x22:
                    printf("sub  ");
                    break;
                case 0x24:
                    printf("and  ");
                    break;
                case 0x25:
                    printf("or  ");
                    break;
                case 0x2A:
                    printf("slt  ");
                    break;
            }
            break;
        case 0x1C:
            printf("mul  ");
            break;
        case 0x4:
            printf("beq  ");
            break;
        case 0x5:
            printf("bne  ");
            break;
        case 0x8:
            printf("addi ");
            break;
        case 0xA:
            printf("slti  ");
            break;
        case 0xC:
            printf("andi ");
            break;
        case 0xD:
            printf("ori  ");
            break;
        case 0xF:
            printf("lui  ");
            break;
    }
    return;
}

int print_output(int registers[MAX_REGISTERS], int commands[MAX_INSTRUCTIONS], int i) {
    registers[0] = 0;
    int command = commands[i];
    int32_t instr_mask = command >> 26;
    int32_t s = (command >> 21) & 0x1F;
    int32_t t = (command >> 16) & 0x1F;
    int32_t d = (command >> 11) & 0x1F;
    int32_t last_six = command & 0x3F;
    int16_t integer_mask = command & 0xFFFF; 
    // syscall
    if (command == 0xC) {
        // print number
        if (registers[2] == 1)
            printf("%d", registers[4]);
        // print char
        else if (registers[2] == 11)
            printf("%c", registers[4]);
        // exit
        else if (registers[2] == 10)
            return 0;
        // unknown syscall
        else {
            printf("Unknown system call: %d\n", registers[2]);
            return 0;
        }
    }

    switch (instr_mask) {
        case 0x0:
            switch (last_six) {
            // add 	d = s + t
            case 0x20:
                registers[d] = registers[s] + registers[t];
                break;
            // sub 	d = s - t
            case 0x22:
                registers[d] = registers[s] - registers[t];
                break;
            // and d = s & t
            case 0x24:
                registers[d] = registers[s] & registers[t];
                break;
            // or d = s | t
            case 0x25:
                registers[d] = registers[s] | registers[t];
                break;
            // slt d = 1 if s < t else 0
            case 0x2A:
                if (registers[s] < registers[t]) {
                    registers[d] = 1;
                } else {
                    registers[d] = 0;
                }
                break;
            }
            break;
        // mul d = s * t
        case 0x1C:
            registers[d] = registers[s] * registers[t];
            break;
        // beq 	if (s == t) PC += I
        case 0x4:
            while (registers[s] == registers[t]) {
                int pc = integer_mask + i;
                while (pc < i) {
                    print_output(registers, commands, pc);
                    pc++;
                }
            }
            break;
        // bne if (s != t) PC += I
        case 0x5:
            while (registers[s] != registers[t]) {
                int pc = integer_mask + i;
                while (pc < i) {
                    print_output(registers, commands, pc);
                    pc++;
                }
            }
            break;
        // addi t = s + I
        case 0x8:
            registers[t] = registers[s] + integer_mask;
            break;
        // slti t = (s < I)
        case 0xA:
            registers[t] = registers[s] < integer_mask;
            break;
        // andi t = s & I
        case 0xC:
            registers[t] = registers[s] & integer_mask;
            break;
        // ori t = s | I
        case 0xD:
            registers[t] = registers[s] | integer_mask;
            break;
        // lui t = I << 16
        case 0xF:
            registers[t] = integer_mask << 16;
            break;
        default:
            return 0;
        }
        return 1;
}

/* 
checks for invalid instructions in hex file, returns 1 if invalid 
instruction found. Else returns 1001 (number outside of max instructions)
*/
int check_invalid_instruction(int commands[MAX_INSTRUCTIONS], int i) {
    for (int j = 0; j < i; j++) {
        int command = commands[j];
        int32_t instr_mask = command >> 26;
        int32_t last_six = command & 0x3F;
        if (commands[j] == 0xC || last_six == 0x20 || last_six == 0x22 || 
            last_six == 0x24 || last_six == 0x25 || last_six == 0x2A || 
            instr_mask == 0x1C || instr_mask == 0x4 || instr_mask == 0x5 || 
            instr_mask == 0x8 || instr_mask == 0xA || instr_mask == 0xC ||
            instr_mask == 0xD || instr_mask == 0xF) {
            continue;
        } else {
            return j;
        }
    }
    return 1001;
}
/*
function to print out registers after program execution, iterates
through registers array and prints out non-zero registers.
*/
void print_registers(int registers[MAX_REGISTERS]) {
    for (int i = 0; i < MAX_REGISTERS; i++) {
        if (registers[i] != 0) {
            // switch between 2 prints to adhere to styling in 
            // assignment specification.
            if (i < 10) {
                printf("$%d  = %d\n", i, registers[i]);
            } else {
                printf("$%d = %d\n", i, registers[i]);
            }
        }
    }   
    return;
}


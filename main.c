#pragma comment(linker, "/SUBSYSTEM:console")
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    uint8_t a, f, b, c, d, e, h, l;
    uint16_t pc;
    uint16_t sp;
    uint64_t total_cycles;
} GameBoy_CPU;

uint8_t memory[0x10000];

uint8_t cpu_read(uint16_t address) {
    return memory[address];
}

void cpu_step(GameBoy_CPU *cpu) {
    uint8_t opcode = cpu_read(cpu->pc);
    cpu->pc++; 

    switch (opcode) {
        case 0x00:
            printf("[0x%04X] Wykonano: NOP\n", cpu->pc - 1);
            cpu->total_cycles += 4;
            break;
        case 0x3C:
            cpu->a++;
            printf("[0x%04X] Wykonano: INC A (Nowa wartość rejestru A = %d)\n", cpu->pc - 1, cpu->a);
            cpu->total_cycles += 4;
            break;
        default:
            printf("\n[BLAD] Nieznana instrukcja: 0x%02X na adresie: 0x%04X\n", opcode, cpu->pc - 1);
            exit(1); 
    }
}

int main() {
    printf("--- Start emulatora Game Boya ---\n\n");
    GameBoy_CPU cpu = {0};
    cpu.pc = 0x0000;

    memory[0x0000] = 0x00;
    memory[0x0001] = 0x3C;
    memory[0x0002] = 0x3C;
    memory[0x0003] = 0x42;

    for (int i = 0; i < 4; i++) {
        cpu_step(&cpu);
    }

    return 0;
}

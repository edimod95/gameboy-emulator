#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include "mmu.h"

// Definicje masek bitowych dla flag
#define FLAG_Z (1 << 7) 
#define FLAG_N (1 << 6) 
#define FLAG_H (1 << 5) 
#define FLAG_C (1 << 4) 

// Struktura procesora
typedef struct {
    union {
        struct { uint8_t f; uint8_t a; }; 
        uint16_t af;
    };
    union {
        struct { uint8_t c; uint8_t b; };
        uint16_t bc;
    };
    union {
        struct { uint8_t e; uint8_t d; };
        uint16_t de;
    };
    union {
        struct { uint8_t l; uint8_t h; };
        uint16_t hl;
    };
    uint16_t pc;
    uint16_t sp;
    uint64_t total_cycles;
} GameBoy_CPU;

// Deklaracje funkcji, aby były widoczne w innych plikach
void cpu_set_flag(GameBoy_CPU *cpu, uint8_t flag, bool value);
bool cpu_get_flag(GameBoy_CPU *cpu, uint8_t flag);
void cpu_step(GameBoy_CPU *cpu);

#endif

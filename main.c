#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// 1. Definicje masek bitowych dla flag
#define FLAG_Z (1 << 7) 
#define FLAG_N (1 << 6) 
#define FLAG_H (1 << 5) 
#define FLAG_C (1 << 4) 

// 2. Struktura procesora
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

// 3. Wirtualna pamięć RAM konsoli
uint8_t memory[0x10000];

// 4. Funkcje pomocnicze do obsługi flag bitowych
void cpu_set_flag(GameBoy_CPU *cpu, uint8_t flag, bool value) {
    if (value) cpu->f |= flag;  
    else cpu->f &= ~flag; 
}

bool cpu_get_flag(GameBoy_CPU *cpu, uint8_t flag) {
    return (cpu->f & flag) != 0;
}

// 5. Funkcja pomocnicza do odczytu pamięci
uint8_t cpu_read(uint16_t address) {
    return memory[address];
}

// 6. Główna logika krokowa procesora
void cpu_step(GameBoy_CPU *cpu) {
    uint16_t current_pc = cpu->pc; // Zapamiętujemy adres instrukcji przed pobraniem
    uint8_t opcode = cpu_read(cpu->pc);
    cpu->pc++; 

    switch (opcode) {
        case 0x00: // NOP
            printf("[0x%04X] Wykonano: NOP\n", current_pc);
            cpu->total_cycles += 4;
            break;

        case 0x01: // LD BC, d16
            {
                uint8_t low = cpu_read(cpu->pc);   cpu->pc++;
                uint8_t high = cpu_read(cpu->pc);  cpu->pc++;
                cpu->bc = (high << 8) | low;
                printf("[0x%04X] Wykonano: LD BC, 0x%04X\n", current_pc, cpu->bc);
                cpu->total_cycles += 12;
            }
            break;

        case 0x20: // JR NZ, e8 (Skok warunkowy)
            {
                int8_t offset = (int8_t)cpu_read(cpu->pc);
                cpu->pc++; 

                if (!cpu_get_flag(cpu, FLAG_Z)) {
                    uint16_t old_pc = cpu->pc;
                    cpu->pc = cpu->pc + offset;
                    printf("[0x%04X] Wykonano: JR NZ, %d (Warunek spelniony: Skok z 0x%04X do 0x%04X)\n", 
                           current_pc, offset, old_pc, cpu->pc);
                    cpu->total_cycles += 12;
                } else {
                    printf("[0x%04X] Wykonano: JR NZ, %d (Warunek NIEspelniony: brak skoku)\n", 
                           current_pc, offset);
                    cpu->total_cycles += 8;
                }
            }
            break;

        case 0x3C: // INC A
            {
                uint8_t original_value = cpu->a;
                cpu->a++;
                cpu_set_flag(cpu, FLAG_Z, (cpu->a == 0)); 
                cpu_set_flag(cpu, FLAG_N, false);        
                cpu_set_flag(cpu, FLAG_H, ((original_value & 0x0F) + 1) > 0x0F);

                printf("[0x%04X] Wykonano: INC A (A = %d | Flagi Z:%d N:%d H:%d C:%d)\n", 
                       current_pc, cpu->a, 
                       cpu_get_flag(cpu, FLAG_Z), cpu_get_flag(cpu, FLAG_N),
                       cpu_get_flag(cpu, FLAG_H), cpu_get_flag(cpu, FLAG_C));
                       
                cpu->total_cycles += 4;
            }
            break;

        case 0x3D: // DEC A
            {
                cpu->a--;
                cpu_set_flag(cpu, FLAG_Z, (cpu->a == 0)); 
                cpu_set_flag(cpu, FLAG_N, true);         

                printf("[0x%04X] Wykonano: DEC A (Nowa wartosc A = %d | Flaga Z = %d)\n", 
                       current_pc, cpu->a, cpu_get_flag(cpu, FLAG_Z));
                cpu->total_cycles += 4;
            }
            break;

        case 0xC3: // JP nn
            {
                uint8_t low = cpu_read(cpu->pc);   cpu->pc++;
                uint8_t high = cpu_read(cpu->pc);  cpu->pc++;
                uint16_t target_address = (high << 8) | low;
                printf("[0x%04X] Wykonano: JP 0x%04X\n", current_pc, target_address);
                cpu->pc = target_address;
                cpu->total_cycles += 16;
            }
            break;

        default:
            printf("\n[BLAD] Nieznana instrukcja: 0x%02X na adresie: 0x%04X\n", opcode, current_pc);
            exit(1); 
    }
}

// 7. Główna funkcja startowa programu
int main() {
    printf("--- Start emulatora Game Boya - Test Warunku JR NZ ---\n\n");
    GameBoy_CPU cpu = {0};
    cpu.pc = 0x0000;
    
    // Zaczynamy z wartoscia 2 w rejestrze A
    cpu.a = 2; 

    // Program odliczajacy w pamieci:
    memory[0x0000] = 0x3D; // 1. DEC A (2 -> 1, Z=0)
    memory[0x0001] = 0x20; // 2. JR NZ (Z=0, wiec skacze o -3)
    memory[0x0002] = -3;   //    Offset skoku
    memory[0x0003] = 0x42; // 3. Nieznana instrukcja (Koniec)

    for (int i = 0; i < 6; i++) {
        cpu_step(&cpu);
    }

    return 0;
}

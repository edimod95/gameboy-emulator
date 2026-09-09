#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "mmu.h"

void cpu_set_flag(GameBoy_CPU *cpu, uint8_t flag, bool value) {
    if (value) cpu->f |= flag;  
    else cpu->f &= ~flag; 
}

bool cpu_get_flag(GameBoy_CPU *cpu, uint8_t flag) {
    return (cpu->f & flag) != 0;
}

void cpu_step(GameBoy_CPU *cpu) {
    uint16_t current_pc = cpu->pc; 
    uint8_t opcode = mmu_read(cpu->pc); // Uzywamy bezpiecznego odczytu przez MMU
    cpu->pc++; 

    switch (opcode) {
        case 0x00: // NOP
            printf("[0x%04X] Wykonano: NOP\n", current_pc);
            cpu->total_cycles += 4;
            break;

        case 0x01: // LD BC, d16
            {
                uint8_t low = mmu_read(cpu->pc);   cpu->pc++;
                uint8_t high = mmu_read(cpu->pc);  cpu->pc++;
                cpu->bc = (high << 8) | low;
                printf("[0x%04X] Wykonano: LD BC, 0x%04X\n", current_pc, cpu->bc);
                cpu->total_cycles += 12;
            }
            break;

        case 0x20: // JR NZ, e8
            {
                int8_t offset = (int8_t)mmu_read(cpu->pc);
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
                uint8_t low = mmu_read(cpu->pc);   cpu->pc++;
                uint8_t high = mmu_read(cpu->pc);  cpu->pc++;
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

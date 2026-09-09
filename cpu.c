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
    uint8_t opcode = mmu_read(cpu->pc); 
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

            case 0x21: // LD HL, d16
            {
                uint8_t low = mmu_read(cpu->pc);   cpu->pc++;
                uint8_t high = mmu_read(cpu->pc);  cpu->pc++;
                cpu->hl = (high << 8) | low;
                printf("[0x%04X] Wykonano: LD HL, 0x%04X\n", current_pc, cpu->hl);
                cpu->total_cycles += 12;
            }
            break;

            case 0x11: // LD DE, d16 (Wczytaj 16-bitową wartość bezpośrednią do rejestru DE)
            {
                uint8_t low = mmu_read(cpu->pc);   cpu->pc++;
                uint8_t high = mmu_read(cpu->pc);  cpu->pc++;
                cpu->de = (high << 8) | low; // Unia automatycznie rozbije to na cpu->d i cpu->e!
                printf("[0x%04X] Wykonano: LD DE, 0x%04X\n", current_pc, cpu->de);
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

            case 0x23: // INC HL
            {
                cpu->hl++;
                printf("[0x%04X] Wykonano: INC HL (Nowa wartość HL: 0x%04X)\n", current_pc, cpu->hl);
                cpu->total_cycles += 8;
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

            case 0x77: // LD (HL), A
            {
                mmu_write(cpu->hl, cpu->a);
                printf("[0x%04X] Wykonano: LD (HL), A (Zapisano 0x%02X pod adres 0x%04X)\n", 
                       current_pc, cpu->a, cpu->hl);
                cpu->total_cycles += 8;
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
            case 0xCD: // CALL nn (Wywołanie podprogramu/funkcji)
            {
                // 1. Czytamy 16-bitowy adres docelowy z kodu (Little Endian)
                uint8_t low = mmu_read(cpu->pc);   cpu->pc++;
                uint8_t high = mmu_read(cpu->pc);  cpu->pc++;
                uint16_t target_address = (high << 8) | low;

                // 2. Odkładamy AKTUALNY adres PC (czyli adres powrotu) na stos
                // Zmniejszamy SP i zapisujemy starszy bajt, potem młodszy
                cpu->sp--;
                mmu_write(cpu->sp, (cpu->pc >> 8) & 0xFF);
                cpu->sp--;
                mmu_write(cpu->sp, cpu->pc & 0xFF);

                printf("[0x%04X] Wykonano: CALL 0x%04X (Zapisano powrót na stosie pod SP: 0x%04X)\n", 
                       current_pc, target_address, cpu->sp);

                // 3. Skaczemy pod adres funkcji
                cpu->pc = target_address;
                cpu->total_cycles += 24; // Wywołanie funkcji zajmuje sporo czasu (24 cykle)
            }
            break;

            case 0xC9: // RET (Powrót z podprogramu/funkcji)
            {
                // 1. Zdejmujemy adres powrotny ze stosu
                uint8_t low = mmu_read(cpu->sp);   cpu->sp++;
                uint8_t high = mmu_read(cpu->sp);  cpu->sp++;
                uint16_t return_address = (high << 8) | low;

                printf("[0x%04X] Wykonano: RET (Powrót do adresu: 0x%04X, nowy SP: 0x%04X)\n", 
                       current_pc, return_address, cpu->sp);

                // 2. Wpisujemy adres do PC
                cpu->pc = return_address;
                cpu->total_cycles += 16;
            }
            break;

            case 0xC5: // PUSH BC (Odłóż parę rejestrów BC na stos)
            {
                cpu->sp--;
                mmu_write(cpu->sp, cpu->b);
                cpu->sp--;
                mmu_write(cpu->sp, cpu->c);

                printf("[0x%04X] Wykonano: PUSH BC (Wartość: 0x%04X, SP: 0x%04X)\n", current_pc, cpu->bc, cpu->sp);
                cpu->total_cycles += 16;
            }
            break;

            case 0xC1: // POP BC (Zdejmij parę rejestrów BC ze stosu)
            {
                cpu->c = mmu_read(cpu->sp); cpu->sp++;
                cpu->b = mmu_read(cpu->sp); cpu->sp++;

                printf("[0x%04X] Wykonano: POP BC (Nowa wartość: 0x%04X, SP: 0x%04X)\n", current_pc, cpu->bc, cpu->sp);
                cpu->total_cycles += 12;
            }
            break;
                case 0x31: // LD SP, d16 (Załaduj 16-bitową wartość bezpośrednią do SP)
            {
                uint8_t low = mmu_read(cpu->pc);   cpu->pc++;
                uint8_t high = mmu_read(cpu->pc);  cpu->pc++;
                cpu->sp = (high << 8) | low;
                printf("[0x%04X] Wykonano: LD SP, 0x%04X\n", current_pc, cpu->sp);
                cpu->total_cycles += 12;
            }
            break;

            case 0xAF: // XOR A (Alternatywa logiczna A z samym sobą -> zeruje rejestr A)
            {
                cpu->a = cpu->a ^ cpu->a; // Wynik to zawsze 0
                // XOR zawsze ustawia flagę Z na 1, a resztę flag na 0
                cpu_set_flag(cpu, FLAG_Z, true);
                cpu_set_flag(cpu, FLAG_N, false);
                cpu_set_flag(cpu, FLAG_H, false);
                cpu_set_flag(cpu, FLAG_C, false);
                
                printf("[0x%04X] Wykonano: XOR A (A = 0x%02X, Flaga Z = 1)\n", current_pc, cpu->a);
                cpu->total_cycles += 4;
            }
            break;

            case 0x32: // LD (HL-), A (Zapisz A pod adres HL, następnie zmniejsz HL o 1)
            {
                mmu_write(cpu->hl, cpu->a);
                cpu->hl--; // Automatyczna dekrementacja pary HL
                printf("[0x%04X] Wykonano: LD (HL-), A (Zapisano pod adres, nowy HL = 0x%04X)\n", current_pc, cpu->hl);
                cpu->total_cycles += 8;
            }
            break;

                    case 0xCB: // Prefiks instrukcji bitowych
            {
                uint8_t cb_opcode = mmu_read(cpu->pc);
                cpu->pc++;
                
                switch (cb_opcode) {
                    case 0x11: // RL C (Obróć rejestr C w lewo przez flagę Carry)
                        {
                            uint8_t old_bit7 = (cpu->c & (1 << 7)) ? 1 : 0;
                            uint8_t old_carry = cpu_get_flag(cpu, FLAG_C) ? 1 : 0;

                            cpu->c = (cpu->c << 1) | old_carry;

                            // RL ustawia flagę Z jeśli wynik to zero, reszta standardowo
                            cpu_set_flag(cpu, FLAG_Z, (cpu->c == 0));
                            cpu_set_flag(cpu, FLAG_N, false);
                            cpu_set_flag(cpu, FLAG_H, false);
                            cpu_set_flag(cpu, FLAG_C, old_bit7);

                            printf("[0x%04X] Wykonano: RL C (C = 0x%02X, Nowy Carry = %d)\n", current_pc, cpu->c, old_bit7);
                            cpu->total_cycles += 8;
                        }
                        break;

                    case 0x7C: // Istniejący już BIT 7, H
                        {
                            bool bit_set = (cpu->h & (1 << 7)) != 0;
                            cpu_set_flag(cpu, FLAG_Z, !bit_set);
                            cpu_set_flag(cpu, FLAG_N, false);
                            cpu_set_flag(cpu, FLAG_H, true);
                            printf("[0x%04X] Wykonano: BIT 7, H (Bit 7 wynosi: %d | Flaga Z = %d)\n", current_pc, bit_set, cpu_get_flag(cpu, FLAG_Z));
                            cpu->total_cycles += 8;
                        }
                        break;
                        
                    default:
                        printf("\n[BLAD] Nieznana instrukcja bitowa CB: 0x%02X na adresie: 0x%04X\n", cb_opcode, cpu->pc - 1);
                        exit(1);
                }
            }
            break;

            case 0x0E: // LD C, d8 (Wczytaj 8-bitową wartość bezpośrednią do rejestru C)
            {
                cpu->c = mmu_read(cpu->pc);
                cpu->pc++;
                printf("[0x%04X] Wykonano: LD C, 0x%02X\n", current_pc, cpu->c);
                cpu->total_cycles += 8;
            }
            break;

            case 0x3E: // LD A, d8 (Wczytaj 8-bitową wartość bezpośrednią do rejestru A)
            {
                cpu->a = mmu_read(cpu->pc);
                cpu->pc++;
                printf("[0x%04X] Wykonano: LD A, 0x%02X\n", current_pc, cpu->a);
                cpu->total_cycles += 8;
            }
            break;

            case 0xE2: // LD ($FF00+C), A (Zapisz wartość z A pod adres I/O: 0xFF00 + C)
            {
                uint16_t io_address = 0xFF00 + cpu->c;
                mmu_write(io_address, cpu->a);
                printf("[0x%04X] Wykonano: LD ($FF00+C), A (Zapisano 0x%02X pod adres 0x%04X)\n", 
                       current_pc, cpu->a, io_address);
                cpu->total_cycles += 8;
            }
            break;

            case 0x0C: // INC C (Zwiększ wartość rejestru C o 1)
            {
                uint8_t original_value = cpu->c;
                cpu->c++;
                
                // INC modyfikuje flagi Z, N, H (podobnie jak INC A)
                cpu_set_flag(cpu, FLAG_Z, (cpu->c == 0));
                cpu_set_flag(cpu, FLAG_N, false);
                cpu_set_flag(cpu, FLAG_H, ((original_value & 0x0F) + 1) > 0x0F);

                printf("[0x%04X] Wykonano: INC C (C = %d)\n", current_pc, cpu->c);
                cpu->total_cycles += 4;
            }
            break;

            case 0xE0: // LD ($FF00+d8), A (Zapisz wartość z A pod adres I/O: 0xFF00 + argument)
            {
                uint8_t offset = mmu_read(cpu->pc);
                cpu->pc++;
                uint16_t io_address = 0xFF00 + offset;
                mmu_write(io_address, cpu->a);
                printf("[0x%04X] Wykonano: LD ($FF00+0x%02X), A (Zapisano 0x%02X pod adres 0x%04X)\n", 
                       current_pc, offset, cpu->a, io_address);
                cpu->total_cycles += 12;
            }
            break;
                    case 0x1A: // LD A, (DE) (Wczytaj bajt z adresu DE do rejestru A)
            {
                cpu->a = mmu_read(cpu->de);
                printf("[0x%04X] Wykonano: LD A, (DE) (Wczytano 0x%02X z adresu DE: 0x%04X)\n", 
                       current_pc, cpu->a, cpu->de);
                cpu->total_cycles += 8;
            }
            break;

            case 0x13: // INC DE (Zwiększ wartość rejestru 16-bitowego DE o 1)
            {
                cpu->de++;
                printf("[0x%04X] Wykonano: INC DE (Nowa wartość DE: 0x%04X)\n", current_pc, cpu->de);
                cpu->total_cycles += 8;
            }
            break;

            case 0x7B: // LD A, E (Skopiuj wartość z rejestru E do A)
            {
                cpu->a = cpu->e;
                printf("[0x%04X] Wykonano: LD A, E (A = 0x%02X)\n", current_pc, cpu->a);
                cpu->total_cycles += 4;
            }
            break;

            case 0xFE: // CP d8 (Porównaj rejestr A z wartością bezpośrednią z kodu)
            {
                uint8_t value = mmu_read(cpu->pc);
                cpu->pc++;
                
                // Operacja CP działa jak odejmowanie (A - value), ale nie zapisuje wyniku.
                // Modyfikuje za to flagi!
                int result = cpu->a - value;
                cpu_set_flag(cpu, FLAG_Z, (result == 0)); // Czy wartości były równe?
                cpu_set_flag(cpu, FLAG_N, true);          // Zawsze true przy CP (bo to odejmowanie)
                
                // Half-Carry przy odejmowaniu: czy dolna połówka wymagała pożyczki?
                cpu_set_flag(cpu, FLAG_H, ((cpu->a & 0x0F) < (value & 0x0F)));
                // Carry przy odejmowaniu: czy cała wartość wymagała pożyczki? (A < value)
                cpu_set_flag(cpu, FLAG_C, (cpu->a < value));

                printf("[0x%04X] Wykonano: CP 0x%02X (A = 0x%02X | Flaga Z = %d)\n", 
                       current_pc, value, cpu->a, cpu_get_flag(cpu, FLAG_Z));
                cpu->total_cycles += 8;
            }
            break;
            case 0x4F: // LD C, A (Skopiuj zawartość rejestru A do C)
            {
                cpu->c = cpu->a;
                printf("[0x%04X] Wykonano: LD C, A (C = 0x%02X)\n", current_pc, cpu->c);
                cpu->total_cycles += 4;
            }
            break;

            case 0x06: // LD B, d8 (Wczytaj 8-bitową wartość bezpośrednią do rejestru B)
            {
                cpu->b = mmu_read(cpu->pc);
                cpu->pc++;
                printf("[0x%04X] Wykonano: LD B, 0x%02X\n", current_pc, cpu->b);
                cpu->total_cycles += 8;
            }
            break;

            case 0x17: // RLA (Obróć akumulator A w lewo przez flagę Carry)
            {
                // Zapamiętujemy stary bit 7 i starą flagę Carry
                uint8_t old_bit7 = (cpu->a & (1 << 7)) ? 1 : 0;
                uint8_t old_carry = cpu_get_flag(cpu, FLAG_C) ? 1 : 0;

                // Przesuwamy A w lewo i na najmłodszy bit wstawiamy stary Carry
                cpu->a = (cpu->a << 1) | old_carry;

                // Aktualizacja flag według specyfikacji RLA (Z, N, H zawsze 0, C bierze bit 7)
                cpu_set_flag(cpu, FLAG_Z, false);
                cpu_set_flag(cpu, FLAG_N, false);
                cpu_set_flag(cpu, FLAG_H, false);
                cpu_set_flag(cpu, FLAG_C, old_bit7);

                printf("[0x%04X] Wykonano: RLA (A = 0x%02X, Nowy Carry = %d)\n", current_pc, cpu->a, old_bit7);
                cpu->total_cycles += 4;
            }
            break;
            
            case 0x05: // DEC B (Zmniejsz rejestr B o 1)
            {
                cpu->b--;
                cpu_set_flag(cpu, FLAG_Z, (cpu->b == 0)); 
                cpu_set_flag(cpu, FLAG_N, true);         

                printf("[0x%04X] Wykonano: DEC B (Nowa wartosc B = %d | Flaga Z = %d)\n", 
                       current_pc, cpu->b, cpu_get_flag(cpu, FLAG_Z));
                cpu->total_cycles += 4;
            }
            break;



        default:
            printf("\n[BLAD] Nieznana instrukcja: 0x%02X na adresie: 0x%04X\n", opcode, current_pc);
            exit(1); 
    }
}

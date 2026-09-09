#include <stdio.h>
#include <stdint.h>
#include "cpu.h"
#include "mmu.h"

int main() {
    printf("--- Start emulatora Game Boya - Test Zapisu do RAM ---\n\n");
    GameBoy_CPU cpu = {0};
    cpu.pc = 0x0000;
    
    // Ustawiamy rejestr A na fajną wartość do zapisu (np. 0xAB)
    cpu.a = 0xAB; 

    // Program testowy:
    uint8_t boot_code[] = {
        0x21, 0x00, 0xC0, // 0x0000: LD HL, 0xC000 (Zapisze 0xC000 do HL)
        0x77,             // 0x0003: LD (HL), A   (Zapisze wartość 0xAB pod adres 0xC000)
        0x42              // 0x0004: Koniec programu
    };

    mmu_load_rom(0x0000, boot_code, sizeof(boot_code));

    // Wykonujemy 3 kroki
    for (int i = 0; i < 3; i++) {
        cpu_step(&cpu);
    }

    // Na koniec sprawdzimy, czy wartość faktycznie znalazła się w pamięci RAM!
    printf("\nWeryfikacja pamięci: pod adresem 0xC000 znajduje się wartość: 0x%02X\n", mmu_read(0xC000));

    return 0;
}


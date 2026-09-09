#include <stdio.h>
#include <stdint.h>
#include "cpu.h"
#include "mmu.h"

int main() {
    printf("--- Start emulatora Game Boya z modulem MMU ---\n\n");
    GameBoy_CPU cpu = {0};
    cpu.pc = 0x0000;
    cpu.a = 2; 

    // Nasz program testowy jako tablica bajtów (ROM)
    uint8_t boot_code[] = {
        0x3D,       // 0x0000: DEC A
        0x20, -3,   // 0x0001: JR NZ, -3
        0x42        // 0x0003: Nieznana instrukcja (Koniec)
    };

    // Ładujemy nasz wirtualny ROM do pamięci od adresu 0x0000
    mmu_load_rom(0x0000, boot_code, sizeof(boot_code));

    // Test: To ostrzeżenie powinno nadal się pojawić, bo mmu_write chroni ROM!
    mmu_write(0x1000, 0xFF); 

    // Wykonujemy pętlę procesora
    for (int i = 0; i < 6; i++) {
        cpu_step(&cpu);
    }

    return 0;
}

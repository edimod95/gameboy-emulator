#include <stdio.h>
#include "mmu.h"

uint8_t bus_memory[0x10000];

uint8_t mmu_read(uint16_t address) {
    // Na tym etapie po prostu zwracamy wartość z tablicy
    return bus_memory[address];
}

void mmu_write(uint16_t address, uint8_t value) {
    // Kontrola zapisu do sekcji ROM (Kartridż)
    if (address >= 0x0000 && address <= 0x7FFF) {
        // Prawdziwy Game Boy ignoruje próby zapisu do ROM-u gier!
        printf("[MMU OSTRZEZENIE] Proba zapisu do sekcji ROM pod adres 0x%04X wartości 0x%02X zablokowana.\n", address, value);
        return;
    }

    // Jeśli to inny adres (np. WRAM), pozwalamy na zapis
    bus_memory[address] = value;
}

void mmu_load_rom(uint16_t address, uint8_t *data, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        if ((address + i) < 0x10000) {
            bus_memory[address + i] = data[i]; // Bezpośredni zapis do tablicy, omijając blokadę mmu_write
        }
    }
}

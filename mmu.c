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

void mmu_inject_nintendo_logo(void) {
    // Oficjalne, binarne logo Nintendo wymagane przez Boot ROM do uruchomienia gry
    uint8_t nintendo_logo[] = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
        0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
        0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
    };
    
    // Wstrzykujemy logo dokładnie tam, gdzie szuka go gra (od adresu 0x0104 w pamięci kartridża)
    for (int i = 0; i < 48; i++) {
        bus_memory[0x0104 + i] = nintendo_logo[i];
    }
}
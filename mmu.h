#ifndef MMU_H
#define MMU_H

#include <stdint.h>

// Deklaracje bezpiecznych funkcji dostępu do pamięci
uint8_t mmu_read(uint16_t address);
void mmu_write(uint16_t address, uint8_t value);
void mmu_load_rom(uint16_t address, uint8_t *data, uint16_t size);

#endif

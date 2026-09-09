#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "cpu.h"
#include "mmu.h"
#include "video.h"

int main(int argc, char* argv[]) {
    printf("--- Emulator Game Boya - Pelne uruchomienie BOOT ROM ---\n\n");

    if (!video_init()) return 1;

    GameBoy_CPU cpu = {0};
    cpu.pc = 0x0000; 

    // Wczytujemy plik Boot ROM
    FILE *boot_file = fopen("dmg_boot.bin", "rb");
    if (boot_file == NULL) {
        printf("[BLAD] Nie znaleziono pliku dmg_boot.bin!\n");
        video_shutdown();
        return 1;
    }

    uint8_t boot_buffer[256];
    size_t bytes_read = fread(boot_buffer, 1, 256, boot_file);
    fclose(boot_file);

    mmu_load_rom(0x0000, boot_buffer, bytes_read);
    mmu_inject_nintendo_logo(); // <-- DODAJ TĘ LINIĘ! Spowoduje to "włożenie" wirtualnej gry z logo do emulatora
    printf("Wczytano %zu bajtow. Rozpoczynanie emulacji sprzętowej...\n\n", bytes_read);

    bool running = true;
    SDL_Event event;
    bool vram_cleared_msg = false;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        // Wykonujemy 1000 instrukcji procesora na klatkę (ok. 16ms),
        // dzięki czemu pętla 8000 powtórzeń minie w ułamku sekundy!
        for (int i = 0; i < 1000; i++) {
            
            // Informacja, kiedy procesor opuści pętlę czyszczenia VRAM (osiągnie adres 0x000C)
            if (cpu.pc == 0x000C && !vram_cleared_msg) {
                printf("\n[SUKCES] VRAM wyczyszczony! Rejestr HL zszedł poniżej 0x8000.\n");
                printf("Procesor przeszedł do konfiguracji układu AUDIO pod adres 0x000C.\n\n");
                vram_cleared_msg = true;
            }

            cpu_step(&cpu);
        }

        video_render_vram();
        video_update();
        SDL_Delay(16); // Standardowe 60 klatek na sekundę
    }

    video_shutdown();
    return 0;
}

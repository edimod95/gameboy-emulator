#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "cpu.h"
#include "mmu.h"
#include "video.h"

int main(int argc, char* argv[]) {
    printf("--- Uruchamianie ekranu graficznego Game Boya ---\n");
    srand(time(NULL));

    if (!video_init()) {
        printf("Nie udalo sie zainicjalizowac wyswietlacza.\n");
        return 1;
    }

    GameBoy_CPU cpu = {0};
    cpu.pc = 0x0000;

    bool running = true;
    SDL_Event event;

    // Główna pętla emulatora działająca w czasie rzeczywistym
    while (running) {
        // 1. Obsługa zamykania okna
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // 2. Generowanie retro szumu na ekranie (każdy piksel losuje 1 z 4 kolorów)
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                uint8_t random_color = rand() % 4;
                video_draw_pixel(x, y, random_color);
            }
        }

        // 3. Wykonaj krok procesora (na razie puste NOPy)
        cpu_step(&cpu);

        // 4. Odśwież obraz w oknie
        video_update();

        // Małe opóźnienie, żeby procesor nie zużywał 100% procesora komputera
        SDL_Delay(16); // ~60 klatek na sekundę
    }

    printf("Zamykanie emulatora...\n");
    video_shutdown();
    return 0;
}

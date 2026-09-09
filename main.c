#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "cpu.h"
#include "mmu.h"
#include "video.h"

int main(int argc, char* argv[]) {
    printf("--- Uruchamianie ekranu sterowanego przez procesor ---\n");

    if (!video_init()) return 1;

    GameBoy_CPU cpu = {0};
    cpu.pc = 0x0000;
    
    // Kolor czarny (wartość 3 w naszej palecie) wpisujemy do rejestru A
    cpu.a = 3; 

    // Program rysujący linię w pamięci VRAM:
    uint8_t boot_code[] = {
        0x21, 0x00, 0x80, // 0x0000: LD HL, 0x8000  (Ustaw cel na start ekranu)
        0x77,             // 0x0003: LD (HL), A    (Zapal piksel na czarno)
        0x23,             // 0x0004: INC HL        (Przejdź do następnego piksela)
        0xC3, 0x03, 0x00  // 0x0005: JP 0x0003     (Skocz z powrotem do rysowania kolejnego piksela!)
    };

    mmu_load_rom(0x0000, boot_code, sizeof(boot_code));

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        // Wykonujemy np. 40 instrukcji procesora na jedną klatkę ekranu,
        // aby rysowanie działo się szybciej i było widoczne dla oka
        for (int i = 0; i < 40; i++) {
            cpu_step(&cpu);
        }

        // Przepisujemy pamięć VRAM na ekran i odświeżamy okno
        video_render_vram();
        video_update();

        SDL_Delay(16); 
    }

    video_shutdown();
    return 0;
}

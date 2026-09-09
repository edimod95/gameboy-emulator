#include <SDL2/SDL.h>
#include <stdio.h>
#include "video.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;

// Bufor ekranu: przechowuje kolor każdego piksela w formacie ARGB
static uint32_t screen_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

const uint32_t GB_PALETTE[4] = {
    0xFFFFFFFF, // 0: Najjaśniejszy (Biały/Jasnozielony)
    0xFFAAAAAA, // 1: Jasnoszary
    0xFF555555, // 2: Ciemnoszary
    0xFF000000  // 3: Najciemniejszy (Czarny)
};

bool video_init(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Błąd inicjalizacji SDL: %s\n", SDL_GetError());
        return false;
    }

    // Tworzymy okno przeskalowane 4x większe (640x576), aby piksele były widoczne
    window = SDL_CreateWindow("Emulator Game Boya - Ekran", 
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                              SCREEN_WIDTH * 4, SCREEN_HEIGHT * 4, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!texture) return false;

    video_clear();
    return true;
}

void video_clear(void) {
    // Wypełniamy ekran najjaśniejszym kolorem retro
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        screen_buffer[i] = GB_PALETTE[0];
    }
}

void video_draw_pixel(int x, int y, uint8_t color_index) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        screen_buffer[y * SCREEN_WIDTH + x] = GB_PALETTE[color_index & 3];
    }
}

void video_update(void) {
    // Przerzucamy bufor pikseli na teksturę i rysujemy w oknie
    SDL_UpdateTexture(texture, NULL, screen_buffer, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void video_shutdown(void) {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

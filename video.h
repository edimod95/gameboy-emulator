#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>
#include <stdbool.h>

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

// Oryginalna paleta 4 odcieni zieleni klasycznego Game Boya
extern const uint32_t GB_PALETTE[4];

bool video_init(void);
void video_update(void);
void video_clear(void);
void video_draw_pixel(int x, int y, uint8_t color_index);
void video_shutdown(void);

#endif

#ifndef COMMON_H
#define COMMON_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdint.h>
#include <stdbool.h>

static inline uint8_t clamp8(int v){
    return (uint8_t)(v < 0 ? 0 : (v > 255 ? 255 : v));
}

#endif // COMMON_H

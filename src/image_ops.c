#include "image_ops.h"
#include <stdio.h>
#include <string.h>

SDL_Surface* load_image_rgba32(const char* path){
    SDL_Surface* loaded = IMG_Load(path);
    if(!loaded){
        SDL_Log("IMG_Load falhou: %s", IMG_GetError());
        return NULL;
    }
    SDL_Surface* conv = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(loaded);
    if(!conv){
        SDL_Log("ConvertSurface falhou: %s", SDL_GetError());
    }
    return conv;
}

bool is_grayscale_surface(SDL_Surface* s){
    if(!s) return false;
    SDL_Surface* tmp = (s->format == SDL_PIXELFORMAT_RGBA32) ? s : SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGBA32);
    if(!tmp) return false;
    SDL_LockSurface(tmp);
    const uint8_t* p = (const uint8_t*)tmp->pixels;
    const int pitch = tmp->pitch;
    bool gray = true;
    const int step = (tmp->w * tmp->h > 400000) ? 8 : 1; // amostragem rápida p/ imagens grandes
    for(int y=0; y<tmp->h && gray; y+=step){
        const uint8_t* row = p + y*pitch;
        for(int x=0; x<tmp->w && gray; x+=step){
            uint8_t r=row[x*4+0], g=row[x*4+1], b=row[x*4+2];
            if(!(r==g && g==b)) gray = false;
        }
    }
    SDL_UnlockSurface(tmp);
    if(tmp != s) SDL_DestroySurface(tmp);
    return gray;
}

SDL_Surface* convert_to_grayscale(SDL_Surface* src){
    if(!src) return NULL;
    SDL_Surface* out = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGBA32);
    if(!out) return NULL;
    SDL_LockSurface(out);
    uint8_t* p = (uint8_t*)out->pixels;
    int pitch = out->pitch;
    for(int y=0;y<out->h;y++){
        uint8_t* row = p + y*pitch;
        for(int x=0;x<out->w;x++){
            uint8_t r = row[x*4+0];
            uint8_t g = row[x*4+1];
            uint8_t b = row[x*4+2];
            // Y = 0.2125 R + 0.7154 G + 0.0721 B
            int yv = (int)(0.2125*r + 0.7154*g + 0.0721*b + 0.5);
            uint8_t v = clamp8(yv);
            row[x*4+0] = v;
            row[x*4+1] = v;
            row[x*4+2] = v;
        }
    }
    SDL_UnlockSurface(out);
    return out;
}

bool save_surface_png_or_bmp(const char* path, SDL_Surface* surf){
    if(!surf) return false;
    #ifdef IMG_SavePNG
    if(IMG_SavePNG(surf, path) == 0) return true;
    #endif
    if(SDL_SaveBMP(surf, path) == 0) return true;
    return false;
}

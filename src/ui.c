#include "ui.h"
#include <stdio.h>
#include <string.h>

bool create_window_renderer(const char* title, int w, int h, SDL_Window **outW, SDL_Renderer **outR){
    *outW = SDL_CreateWindow(title, w, h, SDL_WINDOW_RESIZABLE);
    if(!*outW){ SDL_Log("CreateWindow: %s", SDL_GetError()); return false; }
    *outR = SDL_CreateRenderer(*outW, NULL);
    if(!*outR){ SDL_Log("CreateRenderer: %s", SDL_GetError()); SDL_DestroyWindow(*outW); return false; }
    return true;
}

void draw_image_fit(SDL_Renderer* r, SDL_Surface* surf, int vw, int vh){
    if(!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    if(!tex) { SDL_Log("CreateTextureFromSurface: %s", SDL_GetError()); return; }
    int iw = surf->w, ih = surf->h;
    float sx = (float)vw / iw;
    float sy = (float)vh / ih;
    float s = sx < sy ? sx : sy;
    int dw = (int)(iw * s);
    int dh = (int)(ih * s);
    SDL_Rect dst = { (vw - dw)/2, (vh - dh)/2, dw, dh };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

void draw_histogram_ui(SDL_Renderer* r, const uint32_t hist[256], int x, int y, int w, int h){
    SDL_Rect bg = { x, y, w, h };
    SDL_SetRenderDrawColor(r, 18, 18, 18, 255);
    SDL_RenderFillRect(r, &bg);
    uint32_t maxv = 1;
    for(int i=0;i<256;i++) if(hist[i] > maxv) maxv = hist[i];
    int barw = (w / 256) > 1 ? (w / 256) : 1;
    for(int i=0;i<256;i++){
        float frac = (float)hist[i] / (float)maxv;
        int bh = (int)(frac * h);
        SDL_Rect bar = { x + i*barw, y + (h - bh), barw - 1, bh };
        SDL_SetRenderDrawColor(r, 100, 200, 255, 255);
        SDL_RenderFillRect(r, &bar);
    }
}

void draw_button(SDL_Renderer* r, SDL_Rect rect, const char* label, ButtonState state, TTF_Font* font){
    if(state == BTN_IDLE) SDL_SetRenderDrawColor(r, 60, 120, 200, 255);
    else if(state == BTN_HOVER) SDL_SetRenderDrawColor(r, 80, 160, 240, 255);
    else SDL_SetRenderDrawColor(r, 40, 90, 160, 255);
    SDL_RenderFillRect(r, &rect);
    SDL_SetRenderDrawColor(r, 15, 15, 15, 255);
    SDL_RenderDrawRect(r, &rect);
    if(font){
        SDL_Surface* surf = TTF_RenderUTF8_Blended(font, label, (SDL_Color){255,255,255,255});
        if(surf){
            SDL_Texture* txt = SDL_CreateTextureFromSurface(r, surf);
            SDL_Rect dst = { rect.x + (rect.w - surf->w)/2, rect.y + (rect.h - surf->h)/2, surf->w, surf->h };
            SDL_RenderCopy(r, txt, NULL, &dst);
            SDL_DestroyTexture(txt);
            SDL_DestroySurface(surf);
        }
    }
}

void draw_multiline_text(SDL_Renderer* r, TTF_Font* font, const char* text, int x, int y, int line_gap){
    if(!font || !text) return;
    const char* p = text;
    while(*p){
        const char* nl = strchr(p, '\n');
        int len = nl ? (int)(nl - p) : (int)strlen(p);
        if(len > 0){
            char* line = (char*)malloc(len+1);
            if(!line) return;
            memcpy(line, p, len); line[len] = 0;
            SDL_Surface* surf = TTF_RenderUTF8_Blended(font, line, (SDL_Color){255,255,255,255});
            free(line);
            if(surf){
                SDL_Texture* txt = SDL_CreateTextureFromSurface(r, surf);
                SDL_Rect dst = { x, y, surf->w, surf->h };
                SDL_RenderCopy(r, txt, NULL, &dst);
                SDL_DestroyTexture(txt);
                SDL_DestroySurface(surf);
                y += dst.h + line_gap;
            }
        }
        if(!nl) break;
        p = nl + 1;
    }
}

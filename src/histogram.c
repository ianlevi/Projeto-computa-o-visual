#include "histogram.h"
#include <string.h>
#include <math.h>

void compute_histogram(const SDL_Surface *surf, uint32_t hist[256]){
    memset(hist, 0, 256*sizeof(uint32_t));
    int w = surf->w, h = surf->h;
    SDL_LockSurface((SDL_Surface*)surf);
    const uint8_t* p = (const uint8_t*)surf->pixels;
    int pitch = surf->pitch;
    for(int y=0;y<h;y++){
        const uint8_t* row = p + y*pitch;
        for(int x=0;x<w;x++){
            uint8_t r = row[x*4+0];
            uint8_t g = row[x*4+1];
            uint8_t b = row[x*4+2];
            uint8_t gray = (uint8_t)(0.2125*r + 0.7154*g + 0.0721*b + 0.5);
            hist[gray]++;
        }
    }
    SDL_UnlockSurface((SDL_Surface*)surf);
}

void make_equalization_lut(const uint32_t hist[256], int total_pixels, uint8_t lut[256]){
    uint64_t acc = 0;
    for(int i=0;i<256;i++){
        acc += hist[i];
        double cdf = (double)acc / (double)total_pixels;
        lut[i] = (uint8_t)(cdf * 255.0 + 0.5);
    }
}

void apply_lut_inplace(SDL_Surface *surf, const uint8_t lut[256]){
    SDL_LockSurface(surf);
    uint8_t* p = (uint8_t*)surf->pixels;
    int pitch = surf->pitch;
    for(int y=0;y<surf->h;y++){
        uint8_t* row = p + y*pitch;
        for(int x=0;x<surf->w;x++){
            uint8_t r = row[x*4+0];
            uint8_t g = row[x*4+1];
            uint8_t b = row[x*4+2];
            uint8_t gray = (uint8_t)(0.2125*r + 0.7154*g + 0.0721*b + 0.5);
            uint8_t v = lut[gray];
            row[x*4+0] = v; row[x*4+1] = v; row[x*4+2] = v;
        }
    }
    SDL_UnlockSurface(surf);
}

double compute_mean(const uint32_t hist[256], int total_pixels){
    if(total_pixels <= 0) return 0.0;
    uint64_t sum = 0;
    for(int i=0;i<256;i++) sum += (uint64_t)i * hist[i];
    return (double)sum / (double)total_pixels;
}

double compute_stddev(const uint32_t hist[256], int total_pixels, double mean){
    if(total_pixels <= 0) return 0.0;
    double acc = 0.0;
    for(int i=0;i<256;i++){
        double d = (double)i - mean;
        acc += d*d * (double)hist[i];
    }
    acc /= (double)total_pixels;
    return sqrt(acc);
}

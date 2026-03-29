#ifndef HISTOGRAM_H
#define HISTOGRAM_H
#include "common.h"

void compute_histogram(const SDL_Surface *surf, uint32_t hist[256]);
void make_equalization_lut(const uint32_t hist[256], int total_pixels, uint8_t lut[256]);
void apply_lut_inplace(SDL_Surface *surf, const uint8_t lut[256]);

double compute_mean(const uint32_t hist[256], int total_pixels);
double compute_stddev(const uint32_t hist[256], int total_pixels, double mean);

#endif // HISTOGRAM_H

#ifndef FAST_DCT_H
#define FAST_DCT_H

#include <mcu.h>
#include <stdint.h>

extern float* stage_1(int16_t *input);

extern float *stage_2(int16_t* input, float* cos_bloc, float *sin_bloc);

extern float* stage_3(int16_t *input, float *cos_bloc, float *sin_bloc);

extern void stage_4(int16_t *vector, int16_t* input);

extern void dct_1d(int16_t *vector, float *cos_bloc, float *sin_bloc);

extern void fdct_bloc(int16_t *bloc, float *cos_bloc, float *sin_bloc);

extern void fast_dct(struct array_mcu *mcus);

#endif

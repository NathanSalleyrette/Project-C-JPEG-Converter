#include <math.h>
#include <mcu.h>
#include <stdint.h>
#include <stdlib.h>

float* stage_1(int16_t *input)
{
    float *output = malloc(8*sizeof(float));
    for (uint8_t i = 0; i < 4; i++) {
        output[i] = input[i] + input[7-i];
        output[7-i] = input[i] - input[7-i];
    }
    return output;
}

float *stage_2(float* input, float* cos_bloc, float *sin_bloc)
{
    float*output = malloc(8*sizeof(float));
    for (uint8_t i = 0; i<2; i ++) {
        output[i] = input[i] + input[3-i];
        output[3-i] = input[i] - input[3-i];
    }
    output[4] = (sin_bloc[0] - cos_bloc[0])*input[7] + cos_bloc[0]*(input[4] + input[7]);
    output[7] = -(cos_bloc[0] + sin_bloc[0])*input[4] + cos_bloc[0]*(input[4] + input[7]);
    output[5] = (sin_bloc[1] - cos_bloc[1])*input[6] + cos_bloc[1]*(input[5] + input[6]);
    output[6] = -(cos_bloc[1] + sin_bloc[1])*input[5] + cos_bloc[1]*(input[5] + input[6]);
    free(input);
    return output;
}

float* stage_3(float *input, float *cos_bloc, float *sin_bloc)
{
    float *output = malloc(8*sizeof(float));
    output[0] = input[0] + input[1];
    output[1] = input[0] - input[1];
    output[2] = sqrt(2)*((sin_bloc[2] - cos_bloc[2])*input[3] + cos_bloc[2]*(input[2] + input[3]));
    output[3] = sqrt(2)*(-(cos_bloc[2] + sin_bloc[2])*input[2] + cos_bloc[2]*(input[2] + input[3]));
    output[4] = input[4] + input[6];
    output[6] = input[4] - input[6];
    output[7] = input[7] + input[5];
    output[5] = input[7] - input[5];
    free(input);
    return output;
}

void stage_4(int16_t *vector, float* input)
{
    vector[0] = input[0];
    vector[4] = input[1];
    vector[2] = input[2];
    vector[6] = input[3];
    vector[1] = input[7] + input[4];
    vector[7] = input[7] - input[4];
    vector[3] = sqrt(2)*input[5];
    vector[5] = sqrt(2)*input[6];
    free(input);
}

void dct_1d(int16_t *vector, float *cos_bloc, float *sin_bloc)
{
    float *step1 = stage_1(vector);
    float *step2 = stage_2(step1, cos_bloc, sin_bloc);
    float *step3 = stage_3(step2, cos_bloc, sin_bloc);
    stage_4(vector, step3);
}

void fdct_bloc(int16_t *bloc, float *cos_bloc, float *sin_bloc)
{
    /* for 2D blocs, dct is applied first to each column, then to each of the
    resulting rows */
    for (uint8_t column = 0; column < 8; column ++) {
        int16_t *vector = malloc(8*sizeof(uint16_t));
        for (uint8_t i = 0; i < 8; i ++) {
            vector[i] = bloc[column + (i*8)];
        }
        dct_1d(vector, cos_bloc, sin_bloc);
        for (uint8_t i = 0; i < 8; i++) {
            bloc[column + 8*i] = vector[i];
        }
        free(vector);
    }
    for (uint8_t i = 0; i < 8; i +=8){
        dct_1d(&(bloc[i]), cos_bloc, sin_bloc);
    }
}
void fast_dct(struct array_mcu *mcus)
{
  float cos_bloc[] = {cos(3*3.1415926535897932/16), cos(3.1415926535897932/16), cos(6*3.1415926535897932/16)};
  float sin_bloc[] = {sin(3*3.1415926535897932/16), sin(3.1415926535897932/16), sin(6*3.1415926535897932/16)};
    for (size_t canal = 0; canal < mcus->ct; canal ++) {
        int nb_blocs = mcus->height*mcus->width*mcus->sf[2*canal]*mcus->sf[2*canal+1];
        for (int i_bloc = 0; i_bloc < nb_blocs; i_bloc += 64){
            fdct_bloc(&(mcus->data[canal][i_bloc]), cos_bloc, sin_bloc);
        }
    }
}

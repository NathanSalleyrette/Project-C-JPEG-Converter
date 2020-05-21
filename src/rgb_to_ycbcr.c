#include <rgb_to_ycbcr.h>

/* Conversion RGB to YCbCr */

void rgb_to_ycbcr(struct array_mcu* mcu)
/* Si c'est en noir et blanc on touche rien */

/* Si c'est en couleur on applique les formules données */
{
    if (mcu->ct == COLOR) {
        size_t nb_pixel = mcu->width*mcu->height*mcu->sf[0]*mcu->sf[1]*64;
        for (size_t i = 0; i < nb_pixel; i++) {
            float y = 0.299f*((float)mcu->data[0][i]) + 0.587f*((float)mcu->data[1][i]) + 0.114f*((float)mcu->data[2][i]);
            float cb = -0.1687f*((float)mcu->data[0][i]) - 0.3313f*((float)mcu->data[1][i]) + 0.5f*((float)mcu->data[2][i]) + 128;
            float cr = 0.5f*((float)mcu->data[0][i]) - 0.4187f*((float)mcu->data[1][i]) - 0.0813f*((float)mcu->data[2][i]) + 128;
            mcu->data[0][i] = y;
            mcu->data[1][i] = cb;
            mcu->data[2][i] = cr;
        }
    }
}

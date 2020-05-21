# include "mcu.h"
# include "jpeg_writer.h"
# include <stdint.h>
# include <assert.h>
# include <stdlib.h>

struct array_mcu *mcu = get_mcu_from_jpeg(jpg);

/* Conversion RGB to YCbCr */

void rgb_to_ycbcr(struct jpeg* jpg, struct array_mcu* mcus)
/* Si c'est en noir et blanc on touche rien */

/* Si c'est en couleur on applique les formules données */
{
    if (mcu->ct == COLOR) {
        size_t nb_pixel = mcu->width*mcu->height*mcu->sf[0]*mcu->sf[1]*64;
        for (size_t i = 0; i < nb_pixel; i++) {
            float y = 0.299f*((float)mcu->data[0][i]) + 0.587f*((float)mcu->data[1][i]) + 0.114f*((float)mcu->data[2][i]);
            float Cb = -0.1687f*((float)mcu->data[0][i]) - 0.3313f*((float)mcu->data[1][i]) + 0.5f*((float)mcu->data[2][i]) + 128;
            float Cr = 0.5f*((float)mcu->data[0][i]) - 0.4187f*((float)mcu->data[1][i]) - 0.0813f*((float)mcu->data[2][i]) +128;
            mcu->data[0][i] = y;
            mcu->data[1][i] = Cb;
            mcu->data[2][i] = Cr;
        }
    }
}


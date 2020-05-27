# include <downsampling.h>
# include <mcu.h>
# include <stdint.h>
# include <stdlib.h>
# include <stdio.h>
# include <mcu.h>
# include <jpeg_writer.h>

/* downsampling module must be modified so that block size is 2x2 */

void test(enum direction dim, uint8_t factor)
{
    struct jpeg *jpg = jpeg_create();
    struct array_mcu* mcus = malloc(sizeof(struct array_mcu));
    // établissement du facteur d'échantillonnage
    enum color_component cc = Cb;
    enum direction dir = H;
    jpeg_set_sampling_factor(jpg, cc, dir, 1);
    dir = V;
    jpeg_set_sampling_factor(jpg, cc, dir, 1);
    cc = Cr;
    jpeg_set_sampling_factor(jpg, cc, dir, 1);
    dir = H;
    jpeg_set_sampling_factor(jpg, cc, dir, 1);
    // sf initial
    uint8_t* sf = malloc(6*sizeof(uint8_t));
    if (dim == H) {
        for (size_t i=0; i<3; i++) {
            sf[2*i] = factor;
            sf[2*i + 1] = 1;
        }
    } else {
        for (size_t i=0; i<3; i++) {
            sf[2*i] = 1;
            sf[2*i + 1] = factor;
        }
    }
    mcus->sf = sf;
    // nb mcus en hauteur et largeur
    mcus->height = 2;
    mcus->width = 2;
    // couleur
    mcus->ct = COLOR;
    // description des composantes
    printf("canal initial \n");
    int16_t *cb = malloc(32*sizeof(int16_t));
    int16_t *cr = malloc(32*sizeof(int16_t));
    for (int y_mcu = 0; y_mcu < 2; y_mcu ++) {
      for (int x_mcu = 0; x_mcu < 2; x_mcu ++) {
        for (int i_bloc = 0; i_bloc<2; i_bloc++) {
          for (int i = 0; i<4; i++) {
            cb[4*(2*(2*y_mcu + x_mcu)+i_bloc)+i] = i + 10*i_bloc + 100*x_mcu + 1000*y_mcu;
            cr[4*(2*(2*y_mcu + x_mcu)+i_bloc)+i] = cb[4*(2*(2*y_mcu + x_mcu)+i_bloc)+i];
            printf("%i ", cb[4*(2*(2*y_mcu + x_mcu)+i_bloc)+i]);
          }
        }
      }
    }
    printf("\n");
    int16_t *canaux[] = {0, cb, cr};
    mcus->data = canaux;
    downsample(jpg, mcus);
    int n = mcus->height*mcus->width*4*(mcus->sf[2]*mcus->sf[3]);
    printf("%i \n", n);

    for (int i = 0; i<n; i++) {
        printf("%d ,", mcus->data[1][i]);
    }
    printf("\n");
    n = mcus->height*mcus->width*mcus->sf[4]*mcus->sf[5]*4;
    printf("%i \n", n);
    for (int i = 0; i<n; i++) {
        printf("%d ,", mcus->data[2][i]);
    }
    free(sf);
    free(cb);
    free(cr);
    free(mcus);
    jpeg_destroy(jpg);

}

int main(void){
    printf("test horizontal\n");
    test(H, 2);
    printf("\n");
    printf("test vertical\n");
    test(V, 2);
}

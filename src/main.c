#include <stdlib.h>
#include <stdio.h>
#include <console.h>
#include <jpeg_writer.h>
#include <downsampling.h>
#include <dct.h>
#include <zigzag.h>
#include <huffman.h>
#include <rgb_to_ycbcr.h>
#include <quantification.h>

#include <qtables.h>

/* À enlever */
#include <math.h>
#define _PI (3.1415926535898f)
/* Petit protoype */
void dct_precalcul(struct array_mcu *mcu);


int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    printf("\
--------------------------------------------------------------------------------\n\
Projet C : Mathis MARION, Alan MANIC, Nathan SALLEYRETTE, Thibault BRUYÈRE\n\
Compression JPEG (JFIF) en mode baseline à partir de PPM/PGM (P6 ou P5)\n\
--------------------------------------------------------------------------------\n\
");

    /* Récupération des informations sur le jpeg et le ppm/pgm */
    struct jpeg *jpg = get_jpeg_from_console(argc, argv);
    if (jpg == NULL)
        return EXIT_FAILURE;

    /* Récupération des informations de chaque pixel */
    struct array_mcu *mcu = get_mcu_from_jpeg(jpg);
    if (mcu == NULL)
        return EXIT_FAILURE;

    /* Conversion RGB vers YCbCr */
    /* Conversion en despi en attendant le module */
    rgb_to_ycbcr(mcu);

    /* Downsampling */
    // downsample(jpg, mcu);

    /* DCT */
    /* Le module DCT fonctionne mais c'est un peu long pour des images en 4K
    * à cause du recalcul des cosinus donc je laisse le choix entre quelle
    * fonction utiliser */
    dct(mcu);
    // dct_precalcul(mcu);

    /* Zigzag */
    matrice_to_zigzag(mcu);

    /* Quantification avec une matrice remplie de 1 en attendant le module */
    /* uint8_t *Yquantification = malloc(64*sizeof(uint8_t));
    uint8_t *Cquantification = malloc(64*sizeof(uint8_t));
    for (uint8_t i = 0; i < 64; ++i) {
        Yquantification[i] = 1;
        Cquantification[i] = 1;
    }
    jpeg_set_quantization_table(jpg, Y, Yquantification);
    jpeg_set_quantization_table(jpg, Cb, Cquantification); */
    
    jpeg_set_quantization_table(jpg, Y, quantification_table_Y);
    jpeg_set_quantization_table(jpg, Cb, quantification_table_CbCr);
    jpeg_set_quantization_table(jpg, Cr, quantification_table_CbCr);
    quantization(jpg, mcu);

    /* Huffman */
    jpeg_set_huffman_table_perso(jpg, mcu);

    /* Écriture des données dans l'image */
    jpeg_write_header(jpg);
    jpeg_write_body(jpg, mcu);
    jpeg_write_footer(jpg);

    /* Mémoire vidée */
    jpeg_destroy(jpg);
    delete_mcu(mcu);

    return EXIT_SUCCESS;
}



/*
    Même rôle que la fonction dct mais en un peu plus rapide
*/
void dct_precalcul(struct array_mcu *mcu) {
   float *cos_bloc = malloc(64*sizeof(float));
   for (uint8_t x = 0; x < 8; ++x) {
       cos_bloc[8*x] = 1/(2.f*sqrt(2));
       for (uint8_t i = 1; i < 8; ++i) {
           cos_bloc[i+8*x] = cos(((float)((2*x+1)*i))*_PI/16.f)/2.f;
       }
   }
   float *tampon = malloc(64*sizeof(float));
   for (uint8_t canal = 0; canal < mcu->ct; ++canal) {
       size_t nelem = mcu->width*mcu->height*mcu->sf[2*canal]*mcu->sf[2*canal+1]*64;
       for (size_t i_debut_bloc = 0; i_debut_bloc < nelem; i_debut_bloc += 64) {
           /* On parcourt le tampon avec les indices i et j */
           for (uint8_t i = 0; i < 8; ++i) {
               for (uint8_t j = 0; j < 8; ++j) {

                   /* Calcul du coefficient i, j */
                   tampon[i+8*j] = 0.f;
                   /* On parcourt les données avec les indices x et y */
                   for (uint8_t x = 0; x < 8; ++x) {
                       for (uint8_t y = 0; y < 8; ++y) {
                           /* On a intégré C(i), C(j) et 2/n dans cos_bloc */
                           tampon[i+8*j] += \
   (mcu->data[canal][i_debut_bloc+x+8*y]-128)*cos_bloc[i+8*x]*cos_bloc[j+8*y];
                       }
                   }


               }
           }
           for (uint8_t i = 0; i < 64; ++i) {
               mcu->data[canal][i_debut_bloc+i] = ((int16_t)tampon[i]);
           }
       }
   }
   free(tampon);
   free(cos_bloc);
}

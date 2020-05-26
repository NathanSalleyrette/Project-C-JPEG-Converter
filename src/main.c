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

/* À enlever */
#include <math.h>
#define _PI (3.1415926535898f)
/* Petit protoype */
void dct_precalcul(struct array_mcu *mcu);


int main(int argc, char *argv[])
{
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
    rgb_to_ycbcr(mcu);

    /* Downsampling */
    downsample(jpg, mcu);

    /* DCT */
    dct(mcu);

    /* Zigzag */
    matrice_to_zigzag(mcu);

    /* Quantification */
    bool pertes = false;
    jpeg_set_quantization_table(jpg, Y, get_quantization_table(Y, pertes));
    jpeg_set_quantization_table(jpg, Cb, get_quantization_table(Cb, pertes));
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

#include <stdlib.h>
#include <stdio.h>
#include <console.h>
#include <jpeg_writer.h>
#include <downsampling.h>
#include <dct.h>
#include <zigzag.h>
#include <huffman.h>
#include <bitstream.h>

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    printf("\
--------------------------------------------------------------------------------\n\
Projet C : Mathis MARION, Alan MANIC, Nathan SALLEYRETTE, Thibault BRUYÈRE\n\
Compression JPEG (JFIF) en mode baseline à partir de PPM/PGM (P6 ou P5)\n\
--------------------------------------------------------------------------------\n");

    /* Récupération des informations sur le jpeg et le ppm/pgm */
    struct jpeg *jpg = get_jpeg_from_console(argc, argv);
    if (jpg == NULL)
        return EXIT_FAILURE;
    /* Récupération des informations de chaque pixel */
    struct array_mcu *mcu = get_mcu_from_jpeg(jpg);
    if (mcu == NULL)
        return EXIT_FAILURE;
    /* Conversion R, G, B -> Y, Cb, Cr */
    // TODO
    /* Downsampling */
    downsample(jpg, mcu);
    /* DCT */
    // dct(mcu); /* MET TOUS LES COEFF À 0 */
    /* Zigzag */
    matrice_to_zigzag(mcu);
    /* Quantification */
    // TODO
    uint8_t *Yquantification = malloc(64*sizeof(uint8_t));
    uint8_t *Cquantification = malloc(64*sizeof(uint8_t));
    for (uint8_t i = 0; i < 64; ++i) {
        Yquantification[i] = 1;
        Cquantification[i] = 1;
    }
    jpeg_set_quantization_table(jpg, Y, Yquantification);
    jpeg_set_quantization_table(jpg, Cb, Cquantification);
    /* Huffman */
    jpeg_set_huffman_table(jpg, mcu);

    /* Écriture des données dans l'image */
    jpeg_write_header(jpg);
    jpeg_write_body(jpg, mcu);
    jpeg_write_footer(jpg);

    jpeg_destroy(jpg);

    return EXIT_SUCCESS;
}

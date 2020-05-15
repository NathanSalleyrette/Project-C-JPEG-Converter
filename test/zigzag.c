#include <huffman.h>
#include <mcu.h>
#include <stdio.h>
#include <zigzag.h>

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    printf("test de zigzag\n");

    int16_t *bloc64 = malloc(64*sizeof(int16_t));
    for (uint8_t i = 0; i < 64; ++i) {
        bloc64[i] = i;
    }
    int16_t *bloc128 = malloc(128*sizeof(int16_t));
    for (uint8_t i = 0; i < 128; ++i) {
        bloc128[i] = i;
    }
    uint8_t sf[6] = {2, 1, 1, 1, 0, 1};
    struct array_mcu mcu;
    mcu.data = malloc(3*sizeof(uint16_t *));
    mcu.data[0] = bloc128;
    mcu.data[1] = bloc64;
    mcu.data[2] = bloc64;
    mcu.height = 1;
    mcu.width = 1;
    mcu.ct = COLOR;
    mcu.sf = sf;


    for (uint8_t i = 0; i < 64; ++i) {
        printf("%i\n", bloc64[i]);
    }
    printf("\n\n");
    for (uint8_t i = 0; i < 128; ++i) {
        printf("%i\n", bloc128[i]);
    }

    matrice_to_zigzag(&mcu);

    printf("xxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    for (uint8_t i = 0; i < 64; ++i) {
        printf("%i\n", bloc64[i]);
    }
    printf("\n\n");
    for (uint8_t i = 0; i < 128; ++i) {
        printf("%i\n", bloc128[i]);
    }
    free(bloc64);
    free(bloc128);
    free(mcu.data);
    return EXIT_SUCCESS;
}

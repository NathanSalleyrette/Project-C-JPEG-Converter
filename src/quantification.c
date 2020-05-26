#include <quantification.h>

/* Voir description dans le header */
uint8_t *get_quantization_table(enum color_component cc)
{
    uint8_t *table = malloc(64*sizeof(uint8_t));
    uint8_t *table_copy = (cc == Y) ? quantification_table_Y : quantification_table_CbCr;
    for (uint8_t i = 0; i < 64; ++i) {
        table[i] = table_copy[i];
    }
    return table;
}


/* Voir description dans le header */
void quantization(struct jpeg *jpeg, struct array_mcu *mcu)
{
    for (uint8_t canal = 0; canal < mcu->ct; ++canal) {
        uint8_t *qtable = jpeg_get_quantization_table(jpeg, canal);
        size_t n_elem = mcu->height*mcu->width*mcu->sf[2*canal]*mcu->sf[2*canal + 1]*64;
        for (size_t i_debut_bloc = 0; i_debut_bloc < n_elem; i_debut_bloc += 64) {
            for (uint8_t i = 0; i < 64; ++i) {
                mcu->data[canal][i_debut_bloc + i] /= qtable[i];
            }
        }
    }
}

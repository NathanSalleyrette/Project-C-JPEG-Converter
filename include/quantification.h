#ifndef QUANTIFICATION_H
#define QUANTIFICATION_H

#include <jpeg_writer.h>
#include <mcu.h>
#include <stdbool.h>


/*
    Alloue la mémoire et renvoie la table de quantification correspondante à
    la color_component demandée
    Attention, la table utilisée par jpeg_writer est la même pour Cb et Cr
    La table peut être libérée par un simple free
*/
uint8_t *get_quantization_table(enum color_component cc, bool loss);

/*
    Applique la quantification aux MCUs avec les tables données dans jpeg
*/
void quantization(struct jpeg *jpeg, struct array_mcu *mcu);


#endif

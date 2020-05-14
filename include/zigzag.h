#ifndef ZIGZAG_H
#define ZIGZAG_H

#include <mcu.h>

/*
    Passage en mode zigzag des MCU sur tous les chunk de tous les canaux
*/
void matrice_to_zigzag(struct array_mcu *mcu);

#endif

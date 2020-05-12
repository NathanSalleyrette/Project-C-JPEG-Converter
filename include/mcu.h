#ifndef MCU_H
#define MCU_H

#include <jpeg_writer.h>
#include <stdint.h>
#include <stdlib.h>

/*
    Indique si l'image est de type couleur ou en niveaux de gris
*/
enum color_type {
    color,
    grey
};


/*
    Structure décrivant les MCUs

    array_mcu.data[canal][(i_mcu*h*v + i_bloc)*64 + i]
    si on veut le i ème élément du i_bloc ième bloc du i_mcu ième MCU pour un canal

    Rien n'indique ce que contienne les canaux (R, G, B ou Y, Cb, Cr)
*/
struct array_mcu {
    uint8_t **data;     /* Data : array de 1 ou 3 canaux,
                         * les canaux étant sous forme de chunk de 64 uint8_t */

    size_t height;      /* Nombre de MCUs en hauteurs */
    size_t width;       /* Nombre de MCUs en largeur */

    uint8_t *sf;        /* si ct = grey, sf = {h0, v0}
                         * si grey = color, sf = {h0, v0, h1, v1, h2, v2} */
    enum color_type ct;
};


/*
    Renvoie une struct array_mcu après lecture des données brutes du ppm
    (pas de downsampling, ou de passage en Y, Cb, Cr)
*/
extern struct array_mcu *get_mcu_from_jpeg(struct jpeg *jpg);

#endif
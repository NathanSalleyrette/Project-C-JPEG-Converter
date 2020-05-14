#include <zigzag.h>

/*
    Voir description dans zigzag.h
*/
void matrice_to_zigzag(struct array_mcu *mcu)
{
    /* Tampon de la taille d'un chunk */
    int16_t *tampon = (int16_t *)malloc(64*sizeof(int16_t));
    /* Nombre de canaux du mcu */
    uint8_t ncanaux = (mcu->ct == COLOR) ? 3 : 1;
    /* On parcours tous les canaux */
    for (uint8_t canal = 0; canal < ncanaux; ++canal) {
        /* Nombre d'éléments dans le canal */
        size_t nelem = mcu->height * mcu->width * \
                       mcu->sf[2*canal] * mcu->sf[2*canal+1] * 64;
        /* On se déplace de bloc en bloc : indice i */
        for (size_t i = 0; i < nelem; i += 64) {
            /* On copie le parcour zigzag du bloc dans le tampon */
            uint8_t samplex = 0;
            uint8_t sampley = 0;
            uint8_t direction = 0; /* 0 on monte, 1 on descend */
            for (uint8_t k = 0; k < 64; ++k) {
                tampon[k] = mcu->data[canal][i+samplex+8*sampley];
                if (direction) {
                    if (sampley == 7) {
                        /* On touche la limite sud */
                        direction = 0;
                        ++samplex;
                    } else if (samplex == 0) {
                        /* On touche la limite ouest */
                        direction = 0;
                        --sampley;
                    } else {
                        /* On descend normalement */
                        --samplex;
                        --sampley;
                    }
                } else {
                    if (sampley == 0) {
                        /* On touche la limite nord */
                        direction = 1;
                        ++samplex;
                    } else if (samplex == 7) {
                        /* On touche la limite est */
                        direction = 1;
                        --sampley;
                    } else {
                        /* On remonte normalement */
                        ++samplex;
                        ++sampley;
                    }
                }
            }
            /* On copie le tampon dans le bloc */
            for (uint8_t k = 0; k < 64; ++k) {
                mcu->data[canal][i+k] = tampon[k];
            }
        }
    }
    free(tampon);
}

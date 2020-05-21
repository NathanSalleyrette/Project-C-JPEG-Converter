# include "downsampling.h"
# include "mcu.h"
# include "jpeg_writer.h"
# include <stdint.h>
# include <assert.h>
# include <stdlib.h>

uint8_t *set_sf(struct jpeg* jpg, struct array_mcu *mcus)
{
    /* met à jour mcus->sf et renvoie un pointeur vers un vecteur contenant les
    facteurs de division des blocs */
    enum color_component cc;
    enum direction dir;
    cc = Cb;
    dir = H;
    mcus->sf[2] = jpeg_get_sampling_factor(jpg, cc, dir);
    dir = V;
    mcus->sf[3] = jpeg_get_sampling_factor(jpg, cc, dir);
    cc = Cr;
    mcus->sf[5] = jpeg_get_sampling_factor(jpg, cc, dir);
    dir = H;
    mcus->sf[4] = jpeg_get_sampling_factor(jpg, cc, dir);
    uint8_t *rapport = malloc(4*sizeof(uint8_t));
    for (int i = 0; i<2; i++) {
        rapport[2*i] = mcus->sf[0]/mcus->sf[2*(i+1)];
        rapport[2*i + 1] = mcus->sf[1]/mcus->sf[2*(i+1) +1];
    }
    return rapport;
}

void downsample(struct jpeg* jpg, struct array_mcu* mcus)
/* apply downsampling over every mcu in the array_mcu */
{
    if (mcus->ct == COLOR) {
        //mise à jour de sf et récupération du rapport d'échantillonnage
        uint8_t* rapport = set_sf(jpg, mcus);
        // pour Cb puis Cr
        for (uint8_t canal=1; canal < 3; canal ++){
            uint8_t h = mcus->sf[2*canal];
            uint8_t v = mcus->sf[2*canal+1];
            // pour chaque MCU
            for (size_t i_mcu=0; i_mcu<mcus->height*mcus->width; i_mcu++) {
                // hauteur puis largeur
                for (int dimension = 0; dimension < 2; dimension++) {
                    for (int i_classe = 0; i_classe < mcus->sf[2*canal+dimension]; i_classe++) {
                        for (int i = 0; i < 64; i++) {
                            // moyenne sur chaque coefficient par groupe
                            int16_t moyenne = 0;
                            // l'indice des bloc est incrémenté de 1 dans le
                            // cas horizontal et de *nb_bloc_horizontaux* dans le cas vertical
                            for (int i_bloc=i_classe*rapport[2*(canal-1)+dimension];\
                                i_bloc<(i_classe + 1-dimension + dimension*mcus->sf[0])*rapport[2*(canal-1)+dimension];\
                                 i_bloc = i_bloc + 1-dimension + dimension*mcus->sf[0]) {
                                // Présupposé: la taille du bloc est 8x8
                                moyenne += mcus->data[canal][(i_mcu*h*v + i_bloc)*64 + i];
                            }
                            moyenne = moyenne/rapport[2*(canal-1)+dimension];
                            // on attribue ensuite la moyenne au bloc correspondant à la simplification,
                            // à savoir le i_classe ième


                            /* Erreur ici : mcus->data[canal][(i_mcu*h*v + i_classe)*2 + i]
                             * est en dehors des dimensions de mcus->data[canal] */
                            mcus->data[canal][(i_mcu*h*v + i_classe)*64 + i] = moyenne;




                        }
                    }
                }
            }
        }
         for (int canal = 1; canal < 3; canal++) {
             mcus->data[canal] = realloc(mcus->data[canal], \
             sizeof(int16_t)*mcus->height*mcus->width*64/\
             (rapport[2*(canal-1)]*rapport[2*(canal-1)+1]));
        }
        free(rapport);
    }
}

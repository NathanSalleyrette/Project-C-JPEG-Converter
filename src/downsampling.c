#include <downsampling.h>
#include <jpeg_writer.h>
#include <stdio.h>

void downsample(struct jpeg* jpg, struct array_mcu* mcu)
{
    /* Le downsampling n'est utile que sur les images en couleur */
    if (mcu->ct == GREY) {
        return;
    }

    /* On set les valeurs de h et v dans mcu */
    mcu->sf[2] = jpeg_get_sampling_factor(jpg, Cb, H);
    mcu->sf[3] = jpeg_get_sampling_factor(jpg, Cb, V);
    mcu->sf[4] = jpeg_get_sampling_factor(jpg, Cr, H);
    mcu->sf[5] = jpeg_get_sampling_factor(jpg, Cr, V);

    /* On parcourt les canaux Cb et Cr */
    for (uint8_t canal = 1; canal < 3; ++canal) {
        uint8_t h = mcu->sf[2*canal];
        uint8_t v = mcu->sf[2*canal + 1];
        uint8_t n_pixel_x = mcu->sf[0]/h;  /* Nbre de pixels à sommer sur x */
        uint8_t n_pixel_y = mcu->sf[1]/v;  /* Nbre de pixels à sommer sur y */
        uint8_t n_pixel = n_pixel_x*n_pixel_y;
        size_t n_elem = mcu->width*mcu->height*h*v*64;
        int16_t *tampon = malloc(sizeof(int16_t)*n_elem);

        /* On parcourt tous les pixels de l'image d'arrivé */
        for (uint32_t x_pixel_arrive = 0;
                x_pixel_arrive < mcu->width*h*8; ++x_pixel_arrive) {
            for (uint32_t y_pixel_arrive = 0;
                    y_pixel_arrive < mcu->height*v*8; ++y_pixel_arrive) {
                uint32_t x_somme_debut = x_pixel_arrive*n_pixel_x;
                uint32_t y_somme_debut = y_pixel_arrive*n_pixel_y;
                uint16_t somme = 0;
                /* On somme tous les pixels du groupe */
                for (uint32_t x_somme = x_somme_debut;
                        x_somme < x_somme_debut+n_pixel_x; ++x_somme) {
                    for (uint32_t y_somme = y_somme_debut;
                            y_somme < y_somme_debut+n_pixel_y; ++y_somme) {
                        somme += mcu->data[canal]\
                        [get_indice_from_coordinates(mcu, 0, x_somme, y_somme)];
                    }
                }

                /* On donne la valeur correspondant au pixel d'arrivé */
                tampon[get_indice_from_coordinates(mcu, canal,
                            x_pixel_arrive, y_pixel_arrive)] = somme/n_pixel;

            }
        }
        /* On remplace mcu->data[canal] par le tampon */
        free(mcu->data[canal]);
        mcu->data[canal] = tampon;
    }
}

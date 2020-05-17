#include <stdbool.h>
#include <stdio.h>
#include <jpeg_writer.h>

struct jpeg {
    struct bitstream *bitstream;
    const char *ppm_filename;
    const char *jpeg_filename;
    uint32_t image_height;
    uint32_t image_width;
    uint8_t nb_components;
    uint8_t *sampling_factor; /* Taille 6 (h0, v0, h1, v1, h2, v2) */
    struct huffman **huffman; /* Taille 4 (Y DC, Y AC, Cb-Cr DC, Cb-Cr AC) */
    uint8_t **quantification; /* Taille 2 (Y, Cb-Cr) */
};

/***********************************************/
/* Ouverture, fermeture et fonctions générales */
/***********************************************/

/* Alloue et retourne une nouvelle structure jpeg. */
extern struct jpeg *jpeg_create(void)
{
    struct jpeg *jpg = malloc(sizeof(struct jpeg));
    jpg->bitstream = NULL;
    jpg->ppm_filename = NULL;
    jpg->jpeg_filename = NULL;
    jpg->image_height = 0;
    jpg->image_width = 0;
    jpg->nb_components = 0;
    jpg->sampling_factor = calloc(6, sizeof(uint8_t));
    jpg->huffman = calloc(4, sizeof(struct huffman *));
    jpg->quantification = calloc(2, sizeof(uint8_t *));
    return jpg;
}

/*
    Détruit une structure jpeg.
    Toute la mémoire qui lui est associée est libérée.
*/
extern void jpeg_destroy(struct jpeg *jpg)
{
    if (jpg->bitstream != NULL) {
        bitstream_flush(jpg->bitstream);
        bitstream_destroy(jpg->bitstream);
    }
    free(jpg->sampling_factor);
    for (size_t i = 0; i < 4; ++i)
        delete_huffman(jpg->huffman[i]);
    free(jpg->huffman);
    for (size_t i = 0; i < 2; ++i)
        free(jpg->quantification[i]);
    free(jpg->quantification);
    free(jpg);
}


/*
    Retourne la magnitude de l'element
*/
uint8_t magnitude(int16_t element)
{
    uint16_t absolu = abs(element);
    uint8_t compteur = 0;
    while (absolu) {
        ++compteur;
        absolu >>= 1;
    }
    return compteur;
}

uint16_t indice(int16_t element, uint8_t magnitude)
{
    if (element >= 0) {
        return element;
    } else {
        return (1 << magnitude) - 1 + element;
    }
}


/*
    Ecrit tout l'en-tête JPEG dans le fichier de sortie à partir des
    informations contenues dans la structure jpeg passée en paramètre.
    En sortie, le bitstream est positionné juste après l'écriture de
    l'en-tête SOS, à l'emplacement du premier octet de données brutes à écrire.
*/
extern void jpeg_write_header(struct jpeg *jpg)
{
    /* SOI */
    bitstream_write_bits(jpg->bitstream, 0xffd8, 16, true);


    /* APP0 */
    bitstream_write_bits(jpg->bitstream, 0xffe0, 16, true);
    /* Taille : 16 octets */
    bitstream_write_bits(jpg->bitstream, 16, 16, false);
    /* "JFIF" */
    char *chaine = "JFIF";
    for (size_t i = 0; i < 5; ++i)
        bitstream_write_bits(jpg->bitstream, chaine[i], 8, false);
    /* Version 1.1 */
    bitstream_write_bits(jpg->bitstream, 1, 8, false);
    bitstream_write_bits(jpg->bitstream, 1, 8, false);
    /* 7 octets de 0 */
    for (size_t i = 0; i < 7; ++i)
        bitstream_write_bits(jpg->bitstream, 0, 8, false);


    /* Tables de quantification */
    uint8_t n_qtb;
    if (jpg->nb_components == 1) {
        n_qtb = 1;
    } else {
        n_qtb = 2;
    }
    for (uint8_t i = 0; i < n_qtb; ++i) {
        if (jpg->quantification[i] != NULL) {
            /* DQT */
            bitstream_write_bits(jpg->bitstream, 0xffdb, 16, true);
            /* Taille : 67 octets */
            bitstream_write_bits(jpg->bitstream, 67, 16, false);
            /* Precision : 8 bits et iq : i */
            bitstream_write_bits(jpg->bitstream, 0, 4, false);
            bitstream_write_bits(jpg->bitstream, i, 4, false);
            /* Ecriture de la table */
            for (uint8_t k = 0; k < 64; ++k)
                bitstream_write_bits(jpg->bitstream, jpg->quantification[i][k], 8, false);
        } else {
            fprintf(stderr, "Attention, pas de table de quantification renseignée à l'indice %u\n", i);
        }
    }


    /* SOF0 */
    bitstream_write_bits(jpg->bitstream, 0xffc0, 16, true);
    /* Taille : 8+3*jpg->nb_components octets */
    bitstream_write_bits(jpg->bitstream, 8+3*jpg->nb_components, 16, false);
    /* Précision de chaque composante (8 bits) */
    bitstream_write_bits(jpg->bitstream, 8, 8, false);
    /* Hauteur */
    bitstream_write_bits(jpg->bitstream, jpg->image_height, 16, false);
    /* Largeur */
    bitstream_write_bits(jpg->bitstream, jpg->image_width, 16, false);
    /* Nombre de composantes */
    bitstream_write_bits(jpg->bitstream, jpg->nb_components, 8, false);
    /* Infos sur les composantes */
    for (uint8_t i = 0; i < jpg->nb_components; ++i) {
        bitstream_write_bits(jpg->bitstream, i, 8, false);
        bitstream_write_bits(jpg->bitstream, jpg->sampling_factor[2*i], 4, false);
        bitstream_write_bits(jpg->bitstream, jpg->sampling_factor[2*i+1], 4, false);
        bitstream_write_bits(jpg->bitstream, ((i == 0) ? 0 : 1), 8, false);
    }


    /* Tables de Huffman */
    uint8_t n_htb;
    if (jpg->nb_components == 1) {
        n_htb = 2;
    } else {
        n_htb = 4;
    }
    for (uint8_t i = 0; i < n_htb; ++i) {
        if (jpg->huffman[i] != NULL) {
            /* DHT */
            bitstream_write_bits(jpg->bitstream, 0xffc4, 16, true);
            /* Taille : 19 + huffman->n_symboles */
            bitstream_write_bits(jpg->bitstream, 19 + jpg->huffman[i]->n_symboles, 16, false);
            /* DC ou AC */
            bitstream_write_bits(jpg->bitstream, i%2, 4, false);
            /* Indice de la table */
            bitstream_write_bits(jpg->bitstream, i/2, 4, false);
            /* Nombre de symboles par longueur */
            for (uint8_t k = 0; k < 16; ++k) {
                bitstream_write_bits(jpg->bitstream, jpg->huffman[i]->n_par_etage[k], 8, false);
            }
            /* Écriture des symboles par longueur */
            for (uint8_t k = 0; k < jpg->huffman[i]->n_symboles; ++k) {
                bitstream_write_bits(jpg->bitstream, jpg->huffman[i]->array_symboles[k], 8, false);
            }
        } else {
            fprintf(stderr, "Attention, pas de table de Huffman renseignée à l'indice %u\n", i);
        }
    }


    /* SOS */
    bitstream_write_bits(jpg->bitstream, 0xffda, 16, true);
    /* Taille : 6+2N octets */
    bitstream_write_bits(jpg->bitstream, 6 + 2*jpg->nb_components, 16, false);
    /* Nombre de composantes */
    bitstream_write_bits(jpg->bitstream, jpg->nb_components, 8, false);
    /* Associations de tables par composante */
    for (uint8_t i = 0; i < jpg->nb_components; ++i) {
        bitstream_write_bits(jpg->bitstream, i, 8, false);
        bitstream_write_bits(jpg->bitstream, (i == 0) ? 0 : 1, 4, false);
        bitstream_write_bits(jpg->bitstream, (i == 0) ? 0 : 1, 4, false);
    }
    /* Premier indice de la sélection spectrale */
    bitstream_write_bits(jpg->bitstream, 0, 8, false);
    /* Dernier indice de la sélection spectrale */
    bitstream_write_bits(jpg->bitstream, 63, 8, false);
    /* Approximation successive */
    bitstream_write_bits(jpg->bitstream, 0, 8, false);
}

/* Écrit les données brutes dans l'image */
void jpeg_write_body(struct jpeg *jpg, struct array_mcu *mcu)
{
    size_t n_mcu = mcu->width*mcu->height;
    uint8_t *n_bloc_par_MCU = malloc(mcu->ct*sizeof(uint8_t));
    for (uint8_t i = 0; i < mcu->ct; ++i) {
        n_bloc_par_MCU[i] = mcu->sf[2*i]*mcu->sf[2*i + 1];
    }

    /* Pour faire des différences d'éléments DC */
    int16_t elemDC_precedent[3] = {0, 0, 0};

    /* On parcourt tous les MCU */
    for (size_t i_mcu = 0; i_mcu < n_mcu; i_mcu++) {
        /* Pour chaque MCU on parcourt les canaux */
        for (uint8_t canal = 0; canal < mcu->ct; ++canal) {
            /* On a les arbres de Huffman suivants pour ce canal */
            struct huffman *huffDC = jpg->huffman[(canal == 0) ? 0 : 2];
            struct huffman *huffAC = jpg->huffman[(canal == 0) ? 1 : 3];
            /* Pour chaque MCU et chaque canal on parcourt les blocs correspondants */
            for (uint8_t i_bloc = 0; i_bloc < n_bloc_par_MCU[canal]; ++i_bloc) {
                /* Le bloc en question va de mcu->data[canal][i_debut] à mcu->data[canal][i_debut+63] */
                size_t i_debut = (i_mcu*n_bloc_par_MCU[canal]+i_bloc)*64;

                /* On écrit le coefficient DC dans le bitstream */
                /* On à le coefficient suivant à encoder */
                int16_t coeffDC = mcu->data[canal][i_debut] - elemDC_precedent[canal];
                elemDC_precedent[canal] = mcu->data[canal][i_debut];
                /* On trouve la magnitude et l'indice correspondant */
                uint8_t magnitudeDC = magnitude(coeffDC);
                uint16_t indiceDC = indice(coeffDC, magnitudeDC);
                /* On écrit le symbole encodant la magnitude dans le bitstream */
                bitstream_write_bits(jpg->bitstream,
                                     huffDC->chemins_par_symbole[magnitudeDC],
                                     huffDC->nbits_par_symbole[magnitudeDC],
                                     false);
                /* On écrit l'indice dans le bitstream */
                bitstream_write_bits(jpg->bitstream,
                                     indiceDC,
                                     magnitudeDC,
                                     false);

                /* On écrit les coefficient AC dans le bitstream */
                uint8_t i = 1;
                while (i < 64) {
                    /* On compte le nombre de coefficients nuls */
                    uint8_t n_nuls = 0;
                    while (i < 64 && mcu->data[canal][i_debut + i] == 0) {
                        ++n_nuls;
                        ++i;
                    }
                    if (i == 64) {
                        /* EOB 0x00 */
                        bitstream_write_bits(jpg->bitstream,
                                             huffAC->chemins_par_symbole[0x00],
                                             huffAC->nbits_par_symbole[0x00],
                                             false);
                    } else {
                        while (n_nuls >= 16) {
                            /* ZRL 0xF0 */
                            bitstream_write_bits(jpg->bitstream,
                                                 huffAC->chemins_par_symbole[0xF0],
                                                 huffAC->nbits_par_symbole[0xF0],
                                                 false);
                            n_nuls -= 16;
                        }
                        /* On doit encore coder n_nuls (<16) coeffs nuls et
                         * un coeff non nul */
                        /* Le coefficient AC est le suivant */
                        int16_t coeffAC = mcu->data[canal][i_debut + i];
                        /* On a la magnitude et l'indice suivant à encoder */
                        uint8_t magnitudeAC = magnitude(coeffAC);
                        uint16_t indiceAC = indice(coeffAC, magnitudeAC);
                        /* Le symbole RLE à encoder est le suivant */
                        uint8_t symbole = (n_nuls << 4) | magnitudeAC;
                        /* On encode et on écrit le symbole RLE dans le bitstream */
                        bitstream_write_bits(jpg->bitstream,
                                             huffAC->chemins_par_symbole[symbole],
                                             huffAC->nbits_par_symbole[symbole],
                                             false);
                        /* On écrit l'indice dans le bitstream */
                        bitstream_write_bits(jpg->bitstream,
                                             indiceAC,
                                             magnitudeAC,
                                             false);
                        /* On passe au coefficient suivant */
                        ++i;
                    }
                }
            }
        }
    }
    free(n_bloc_par_MCU);
}

/* Ecrit le footer JPEG (marqueur EOI) dans le fichier de sortie. */
extern void jpeg_write_footer(struct jpeg *jpg)
{
    /* EOI */
    bitstream_flush(jpg->bitstream);
    bitstream_write_bits(jpg->bitstream, 0xffd9, 16, true);
}

/*
    Retourne le bitstream associé au fichier de sortie enregistré
    dans la structure jpeg.
*/
extern struct bitstream *jpeg_get_bitstream(struct jpeg *jpg)
{
    return jpg->bitstream;
}


/****************************************************/
/* Gestion des paramètres de l'encodeur via le jpeg */
/****************************************************/

/* Ecrit le nom de fichier PPM ppm_filename dans la structure jpeg. */
extern void jpeg_set_ppm_filename(struct jpeg *jpg,
                                  const char *ppm_filename)
{
    jpg->ppm_filename = ppm_filename;
}

/* Retourne le nom de fichier PPM lu dans la structure jpeg. */
extern const char *jpeg_get_ppm_filename(struct jpeg *jpg)
{
    return jpg->ppm_filename;
}

/* Ecrit le nom du fichier de sortie jpeg_filename dans la structure jpeg. */
extern void jpeg_set_jpeg_filename(struct jpeg *jpg,
                                   const char *jpeg_filename)
{
    if (jpg->bitstream != NULL)
        bitstream_destroy(jpg->bitstream);
    jpg->jpeg_filename = jpeg_filename;
    jpg->bitstream = bitstream_create(jpeg_filename);
}

/* Retourne le nom du fichier de sortie lu depuis la structure jpeg. */
extern const char *jpeg_get_jpeg_filename(struct jpeg *jpg)
{
    return jpg->jpeg_filename;
}

/*
    Ecrit la hauteur de l'image traitée, en nombre de pixels,
    dans la structure jpeg.
*/
extern void jpeg_set_image_height(struct jpeg *jpg,
                                  uint32_t image_height)
{
    jpg->image_height = image_height;
}

/*
    Retourne la hauteur de l'image traitée, en nombre de pixels,
    lue dans la structure jpeg.
*/
extern uint32_t jpeg_get_image_height(struct jpeg *jpg)
{
    return jpg->image_height;
}

/*
    Ecrit la largeur de l'image traitée, en nombre de pixels,
    dans la structure jpeg.
*/
extern void jpeg_set_image_width(struct jpeg *jpg,
                                 uint32_t image_width)
{
 jpg->image_width = image_width;
}

/*
    Retourne la largeur de l'image traitée, en nombre de pixels,
    lue dans la structure jpeg.
*/
extern uint32_t jpeg_get_image_width(struct jpeg *jpg)
{
    return jpg->image_width;
}


/*
    Ecrit le nombre de composantes de couleur de l'image traitée
    dans la structure jpeg.
*/
extern void jpeg_set_nb_components(struct jpeg *jpg,
                                   uint8_t nb_components)
{
    jpg->nb_components = nb_components;
}

/*
    Retourne le nombre de composantes de couleur de l'image traitée
    lu dans la structure jpeg.
*/
extern uint8_t jpeg_get_nb_components(struct jpeg *jpg)
{
    return jpg->nb_components;
}


/*
    Ecrit dans la structure jpeg le facteur d'échantillonnage sampling_factor
    à utiliser pour la composante de couleur cc et la direction dir.
*/
extern void jpeg_set_sampling_factor(struct jpeg *jpg,
                                     enum color_component cc,
                                     enum direction dir,
                                     uint8_t sampling_factor)
{
    jpg->sampling_factor[dir + 2*cc] = sampling_factor;
}

/*
    Retourne le facteur d'échantillonnage utilisé pour la composante
    de couleur cc et la direction dir, lu dans la structure jpeg.
*/
extern uint8_t jpeg_get_sampling_factor(struct jpeg *jpg,
                                        enum color_component cc,
                                        enum direction dir)
{

    return jpg->sampling_factor[dir + 2*cc];
}

/*
    Ecrit dans la structure jpeg toutes les tables de Huffman nécessaires
    à partir de mcu. Sauf que c'est pas drôle parce que les tables sont
    données...
*/
extern void jpeg_set_huffman_table(struct jpeg* jpg)
{
    jpg->huffman[0] = get_huffman_premade(DC, Y);
    jpg->huffman[1] = get_huffman_premade(AC, Y);

    if (jpg->nb_components == 3) {
        jpg->huffman[2] = get_huffman_premade(DC, Cb);
        jpg->huffman[3] = get_huffman_premade(AC, Cb);
    }
}

/*
    Ecrit dans la structure jpeg toutes les tables de Huffman nécessaires
    à partir de mcu.
*/
extern void jpeg_set_huffman_table_perso(struct jpeg *jpg, struct array_mcu *mcu)
{
    /* Les symboles DC vont de 0 à 11, donc tableau de taille 12
     * Les symboles AC vont de 0x00 à 0xFA (avec des trous au milieu), on fait
     * donc un tableau de taille 251 */
    uint32_t **frequences_DC = malloc(mcu->ct*sizeof(uint32_t *));
    uint32_t **frequences_AC = malloc(mcu->ct*sizeof(uint32_t *));
    for (uint8_t i = 0; i < mcu->ct; ++i) {
        frequences_DC[i] = calloc(12, sizeof(uint32_t));
        frequences_AC[i] = calloc(251, sizeof(uint32_t));
    }

    size_t n_mcu = mcu->width*mcu->height;
    uint8_t *n_bloc_par_MCU = malloc(mcu->ct*sizeof(uint8_t));
    for (uint8_t i = 0; i < mcu->ct; ++i) {
        n_bloc_par_MCU[i] = mcu->sf[2*i]*mcu->sf[2*i + 1];
    }

    /* Pour faire des différences d'éléments DC */
    int16_t elemDC_precedent[3] = {0, 0, 0};

    /* On parcourt tous les MCU */
    for (size_t i_mcu = 0; i_mcu < n_mcu; i_mcu++) {
        /* Pour chaque MCU on parcourt les canaux */
        for (uint8_t canal = 0; canal < mcu->ct; ++canal) {
            /* Pour chaque MCU et chaque canal on parcourt les blocs correspondants */
            for (uint8_t i_bloc = 0; i_bloc < n_bloc_par_MCU[canal]; ++i_bloc) {
                /* Le bloc en question va de mcu->data[canal][i_debut] à mcu->data[canal][i_debut+63] */
                size_t i_debut = (i_mcu*n_bloc_par_MCU[canal]+i_bloc)*64;

                /* On écrit le coefficient DC dans le bitstream */
                /* On à le coefficient suivant à encoder */
                int16_t coeffDC = mcu->data[canal][i_debut] - elemDC_precedent[canal];
                elemDC_precedent[canal] = mcu->data[canal][i_debut];
                /* On trouve la magnitude et l'indice correspondant */
                uint8_t magnitudeDC = magnitude(coeffDC);


                /* Il faut encoder magnitudeDC pour canal et DC */
                ++frequences_DC[canal][magnitudeDC];


                /* On écrit les coefficient AC dans le bitstream */
                uint8_t i = 1;
                while (i < 64) {
                    /* On compte le nombre de coefficients nuls */
                    uint8_t n_nuls = 0;
                    while (i < 64 && mcu->data[canal][i_debut + i] == 0) {
                        ++n_nuls;
                        ++i;
                    }
                    if (i == 64) {
                        /* EOB 0x00 */


                        /* Il faut encoder 0x00 pour canal et AC */
                        ++frequences_AC[canal][0x00];


                    } else {
                        while (n_nuls >= 16) {
                            /* ZRL 0xF0 */


                            /* Il faut encoder 0xF0 pour canal et AC */
                            ++frequences_AC[canal][0xF0];


                            n_nuls -= 16;
                        }
                        /* On doit encore coder n_nuls (<16) coeffs nuls et
                         * un coeff non nul */
                        /* Le coefficient AC est le suivant */
                        int16_t coeffAC = mcu->data[canal][i_debut + i];
                        /* On a la magnitude à encoder */
                        uint8_t magnitudeAC = magnitude(coeffAC);
                        /* Le symbole RLE à encoder est le suivant */
                        uint8_t symbole = (n_nuls << 4) | magnitudeAC;


                        /* Il faut encoder symbole pour canal et AC */
                        ++frequences_AC[canal][symbole];


                        /* On passe au coefficient suivant */
                        ++i;
                    }
                }
            }
        }
    }
    free(n_bloc_par_MCU);

    jpg->huffman[0] = get_huffman_from_freq(frequences_DC[0], 12);
    jpg->huffman[1] = get_huffman_from_freq(frequences_AC[0], 251);

    if (mcu->ct == COLOR) {
        for (uint8_t i = 0; i < 251; ++i) {
            /* Taille maximale 65535 x 65535 risque d'overflow les uint32_t
            * si on additionnes simplement les frequences_AC de Cb et Cr
            * Le +1 sert à ne pas supprimer les symboles de fréquence 1 */
            frequences_AC[1][i] = ((frequences_AC[1][i] + 1) >> 1) + ((frequences_AC[2][i] + 1) >> 1);
        }
        jpg->huffman[2] = get_huffman_from_freq(frequences_DC[1], 12);
        jpg->huffman[3] = get_huffman_from_freq(frequences_AC[1], 251);
    }

    for (uint8_t i = 0; i < mcu->ct; ++i) {
        free(frequences_DC[i]);
        free(frequences_AC[i]);
    }
    free(frequences_DC);
    free(frequences_AC);

}

/*
    Retourne un pointeur vers la table de Huffman utilisée pour encoder
    les données de la composante fréquentielle acdc pour la composante
    de couleur cc, lue dans la structure jpeg.
*/
extern struct huffman *jpeg_get_huffman_table(struct jpeg *jpg,
                                                 enum sample_type acdc,
                                                 enum color_component cc)
{
    return jpg->huffman[acdc + ((cc == Y )? 0 : 2)];
}


/*
    Ecrit dans la structure jpeg la table de quantification à utiliser
    pour compresser les coefficients de la composante de couleur cc.
*/
extern void jpeg_set_quantization_table(struct jpeg *jpg,
                                        enum color_component cc,
                                        uint8_t *qtable)
{
    jpg->quantification[(cc == Y) ? 0 : 1] = qtable;
}

/*
    Retourne un pointeur vers la table de quantification associée à la
    composante de couleur cc, lue dans a structure jpeg.
*/
extern uint8_t *jpeg_get_quantization_table(struct jpeg *jpg,
                                            enum color_component cc)
{
    return jpg->quantification[(cc == Y) ? 0 : 1];
}

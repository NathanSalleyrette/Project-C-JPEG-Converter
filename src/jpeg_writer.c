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
    uint8_t *sampling_factor; // Taille 6 (h0, v0, h1, v1, h2, v2)
    struct huffman **huffman; // Taille 4 (Y DC, Y AC, Cb-Cr DC, Cb-Cr AC)
    uint8_t **quantification; // Taille 2 (Y, Cb-Cr)
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
    bitstream_destroy(jpg->bitstream);
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
    Ecrit tout l'en-tête JPEG dans le fichier de sortie à partir des
    informations contenues dans la structure jpeg passée en paramètre.
    En sortie, le bitstream est positionné juste après l'écriture de
    l'en-tête SOS, à l'emplacement du premier octet de données brutes à écrire.
*/
extern void jpeg_write_header(struct jpeg *jpg)
{
    /* SOI */
    bitstream_write_bits(jpg->bitstream, 0xffd8, 16, true);
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
            bitstream_write_bits(jpg->bitstream, 0xffd8, 16, true);
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
            bitstream_write_bits(jpg->bitstream, i, 4, false);
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
        bitstream_write_bits(jpg->bitstream, (i == 0) ? 0 : 2, 4, false);
        bitstream_write_bits(jpg->bitstream, (i == 0) ? 1 : 3, 4, false);
    }
    /* Premier indice de la sélection spectrale */
    bitstream_write_bits(jpg->bitstream, 0, 8, false);
    /* Dernier indice de la sélection spectrale */
    bitstream_write_bits(jpg->bitstream, 63, 8, false);
    /* Approximation successive */
    bitstream_write_bits(jpg->bitstream, 0, 8, false);
}

/* Ecrit le footer JPEG (marqueur EOI) dans le fichier de sortie. */
extern void jpeg_write_footer(struct jpeg *jpg)
{
    /* EOI */
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
    Ecrit dans la structure jpeg la table de Huffman huff_table à utiliser
    pour encoder les données de la composante fréquentielle acdc, pour la
    composante de couleur cc.
*/
extern void jpeg_set_huffman_table(struct jpeg *jpg,
                                   enum sample_type acdc,
                                   enum color_component cc,
                                   struct huffman *htable)
{
    jpg->huffman[acdc + ((cc == Y )? 0 : 2)] = htable;
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

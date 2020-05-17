#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <mcu.h>

enum sample_type;
enum color_component;


/*
    Structure représentant un arbre de Huffman pour JPEG
    Utile pour coder les magnitudes
*/
struct huffman {
    /* Utile pour retrouver le chemin correspondant à un symbole */
    uint32_t *chemins_par_symbole; /* array de taille magnitude_max + 1
                                    * chemin dans les poids faible */
    uint8_t *nbits_par_symbole; /* array de taille magnitude_max + 1
                                 * correspond aux nombre de bits des chemins */
    uint8_t n_max;

    /* Utile pour l'écriture dans le jpeg */
    uint8_t *n_par_etage; /* array de taille 16 */
    uint8_t *array_symboles; /*array de taille sum(n_par_etage) */
    uint8_t n_symboles; /* valeur de sum(n_par_etage) */
};

/*
    Donne une table de huffman correspondant au AC/DC et au cc
*/
extern struct huffman *get_huffman_premade(enum sample_type st, enum color_component cc);

/*
    Renvoie une structure huffman en fonction des fréquences
    absolues en argument.
    Ne prends pas en commpte les fréquences nulles et ne créé pas de chemin
    uniquement composé de '1'.
*/
extern struct huffman *get_huffman_from_freq(uint32_t *frequences, uint8_t n);

/*
    Destruction et libération de la mémoire liée à huff
*/
extern void delete_huffman(struct huffman *huff);

/*
    Affiche sur la sortie standard les valeur des attributs de huff
    L'argument est n est la valeur passée en argument à la création par
    get_huffman_from_freq (12 pour DC, 11 pour AC)
*/
extern void description_huffman(struct huffman *huff);

#endif /* HUFFMAN_H */

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <mcu.h>


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

    /* Utile pour l'écriture dans le jpeg */
    uint8_t *n_par_etage; /* array de taille 16 */
    uint8_t *array_symboles; /*array de taille sum(n_par_etage) */
    uint8_t n_symboles; /* valeur de sum(n_par_etage) */
};

/*
    Retourne la magnitude de l'element
*/
extern uint8_t magnitude(int16_t element);

/*
    Retourne l'indice de l'élément à partir de sa magnitude
*/
extern uint16_t indice(int16_t element, uint8_t magnitude);

/*
    Renvoie une structure huffman en fonction des fréquences
    absolues en argument.
    Ne prends pas en commpte les fréquences nulles et ne créé pas de chemin
    uniquement composé de '1'.
*/
extern struct huffman *get_huffman_from_freq(uint32_t *frequences, uint8_t n);

/*
    Renvoie une array 2 pointeurs vers struct huffman si l'image est en niveaux
    de gris ou 4 pointeurs vers struct huffman si l'image est en couleurs
    dans l'ordre suivant : (Y DC, Y AC) ou (Y DC, Y AC, Cb-Cr DC, Cb-Cr AC)
    Ces struct huffman servent au codage des magnitudes
*/
extern struct huffman **get_huffman_from_mcu(struct array_mcu *mcu);

/*
    Destruction et libération de la mémoire liée à huff
*/
extern void delete_huffman(struct huffman *huff);

/*
    Affiche sur la sortie standard les valeur des attributs de huff
    L'argument est n est la valeur passée en argument à la création par
    get_huffman_from_freq (12 pour DC, 11 pour AC)
*/
extern void description_huffman(struct huffman *huff, uint8_t n);

#endif /* HUFFMAN_H */

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <jpeg_writer.h>
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
};

/*
    Renvoie une structure huffman en fonction des fréquences
    absolues en argument.
    Ne prends pas en commpte les fréquences nulles et ne créé pas de chemin
    uniquement composé de '1'.
*/
extern struct huffman *get_huffman_from_freq(uint8_t n, uint32_t *frequences);

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
    L'argument est n est la valeur passée en argument à la crétion par
    get_huffman_from_freq
*/
extern void description_huffman(struct huffman *huff, uint8_t n);

#endif /* HUFFMAN_H */

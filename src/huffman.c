#include <huffman.h>
#include <stdio.h>
#include <stdbool.h>


/*
    Structure utile à la création d'arbre de Huffman et à son
    parcours en largeur
*/
struct arbre {
    uint32_t freq_abs;
    bool est_feuille;
    uint8_t symbole; /* si == 0 : est feuille, sinon, indice gauche */
    struct arbre *fils_droit;
    struct arbre *fils_gauche;
    struct arbre *suivant; /* noeud suivant dans un parcours en largeur */
};


/*
    Effectue une percolation descendante du tas-min arbre depuis le noeud
    La valeur prise en compte est freq_abs
*/
void percole_desc(struct arbre **arbre, uint8_t taille, uint8_t noeud)
{
    struct arbre *noeud_init = arbre[noeud];
    uint32_t valeur = noeud_init->freq_abs;
    /* Tant qu'il y a deux fils et qu'un des deux est plus petit, on percole */
    while (2*noeud+2 < taille &&
            (valeur > arbre[2*noeud+1]->freq_abs ||
             valeur > arbre[2*noeud+2]->freq_abs)) {
         if (arbre[2*noeud+1]->freq_abs < arbre[2*noeud+2]->freq_abs) {
             arbre[noeud] = arbre[2*noeud+1];
             noeud = 2*noeud+1;
         } else {
             arbre[noeud] = arbre[2*noeud+2];
             noeud = 2*noeud+2;
         }
     }
     /* Si le noeud n'a qu'un seul fils plus petit, on percole */
     if (2*noeud+1 < taille && valeur > arbre[2*noeud+1]->freq_abs) {
         arbre[noeud] = arbre[2*noeud+1];
         noeud = 2*noeud+1;
     }
     /* On termine la percolation */
     arbre[noeud] = noeud_init;
}


/*
    Voir description dans huffman.h
*/
struct huffman *get_huffman_from_freq(uint32_t *frequences, uint8_t n)
{
    /* On compte le nombre de fréquences non nulles */
    uint8_t n_non_nul = 0;
    for (uint8_t i = 0; i < n; ++i) {
        if (frequences[i] != 0) {
            ++n_non_nul;
        }
    }

    /* On créé l'array d'arbres */
    struct arbre *arbres = (struct arbre *)
                           malloc((2*n_non_nul + 1)*sizeof(struct arbre));
    /* On créé les feuilles */
    uint8_t next_indice = 0;
    for (uint8_t i = 0; i < n; ++i) {
        if (frequences[i] != 0) {
            arbres[next_indice].est_feuille = true;
            arbres[next_indice].symbole = i;
            arbres[next_indice].freq_abs = frequences[i];
            arbres[next_indice].suivant = NULL;
            ++next_indice;
        }
    }
    /* On ajoute un intron (symbole n, fréquence nulle)
     * Il y a donc n_non_nul+1 feuilles */
    arbres[next_indice].est_feuille = true;
    arbres[next_indice].symbole = n;
    arbres[next_indice].freq_abs = 0;
    arbres[next_indice].suivant = NULL;
    ++next_indice;
    /* On créé un tas min (pour l'attribut freq_abs)
     * de pointeurs vers des struct arbre */
    uint8_t taille_tas = n_non_nul+1;
    struct arbre **tas_min = (struct arbre **)
                             malloc(taille_tas*sizeof(struct arbre *));
    /* On le remplis avec les feuilles créés */
    for (uint8_t i = 0; i < taille_tas; ++i) {
        tas_min[i] = &arbres[i];
    }
    /* On organise le tas_min */
    for (uint8_t i = taille_tas; i > 0;) {
        percole_desc(tas_min, taille_tas, --i);
    }
    /* On fusionne les arbres de poids minimum jusqu'a n'en avoir plus qu'un */
    while (taille_tas > 1) {
        /* On prends le plus petit arbre restant */
        struct arbre *gauche = tas_min[0];
        /* On redonne sa propriété au tas_min */
        tas_min[0] = tas_min[--taille_tas];
        percole_desc(tas_min, taille_tas, 0);
        /* On remplace le petit arbre restant
         * par une fusion des deux plus petit */
        struct arbre *droit = tas_min[0];
        struct arbre *nouv_arbre = &arbres[next_indice++];
        nouv_arbre->est_feuille = false;
        nouv_arbre->fils_gauche = gauche;
        nouv_arbre->fils_droit = droit;
        nouv_arbre->freq_abs = gauche->freq_abs + droit->freq_abs;
        nouv_arbre->suivant = NULL;
        tas_min[0] = nouv_arbre;
        /* On redonne sa propriété au tas_min */
        percole_desc(tas_min, taille_tas, 0);
    }
    /* On récupère la racine et on libère le tas_min */
    struct arbre *racine = tas_min[0];
    free(tas_min);

    struct huffman *huff = (struct huffman *)malloc(sizeof(struct huffman));
    huff->chemins_par_symbole = (uint32_t *)calloc(n, sizeof(uint32_t));
    huff->nbits_par_symbole = (uint8_t *)calloc(n, sizeof(uint8_t));
    huff->array_symboles = (uint8_t *)calloc(n_non_nul, sizeof(uint8_t));
    huff->n_par_etage = (uint8_t *)calloc(16, sizeof(uint8_t));

    struct arbre marqueur;
    racine->suivant = &marqueur;
    marqueur.suivant = NULL;
    struct arbre *dpile = racine;
    struct arbre *fpile = &marqueur;
    uint32_t prochain_chemin = 0;
    uint8_t etage = 0;
    uint8_t symbole_courant = 0;
    while (symbole_courant < n_non_nul) {
        if (dpile == &marqueur) {
            /* On change d'étage */
            ++etage;
            prochain_chemin <<= 1;
            /* On remet le marqueur à la fin de la pile */
            dpile = marqueur.suivant;
            fpile->suivant = &marqueur;
            marqueur.suivant = NULL;
            fpile = &marqueur;
        } else if (dpile->est_feuille) {
            /* On test que ce ne soit pas l'intron */
            if (dpile->symbole != n) {
                /* On ajoute un symbole à l'étage */
                huff->chemins_par_symbole[dpile->symbole] = prochain_chemin++;
                huff->nbits_par_symbole[dpile->symbole] = etage;
                huff->array_symboles[symbole_courant++] = dpile->symbole;
                ++huff->n_par_etage[etage];
            }
            dpile = dpile->suivant;
        } else {
            /* On ajoute les deux fils */
            fpile->suivant = dpile->fils_gauche;
            fpile = fpile->suivant;
            fpile->suivant = dpile->fils_droit;
            fpile = fpile->suivant;
            fpile->suivant = NULL;
            dpile = dpile->suivant;
        }
    }
    /* On libère les arbres */
    free(arbres);

    huff->n_symboles = n_non_nul;
    huff->n_max = n;
    return huff;
}

/*
    Voir description dans huffman.h
*/
void delete_huffman(struct huffman *huff)
{
    if (huff == NULL)
        return;
    free(huff->chemins_par_symbole);
    free(huff->nbits_par_symbole);
    free(huff->n_par_etage);
    free(huff->array_symboles);
    free(huff);
}

/*
    Voir description dans huffman.h
*/
void description_huffman(struct huffman *huff)
{
    fprintf(stderr, "%lX", (uint64_t) huff);
    printf("\nNombre de bits par symbole\n");
    for (uint8_t i = 0; i < huff->n_max; ++i) {
        printf("%u\n", huff->nbits_par_symbole[i]);
    }
    printf("\nChemins par symbole : \n");
    for (uint8_t i = 0; i < huff->n_max; ++i) {
        printf("%u\n", huff->chemins_par_symbole[i]);
    }
    printf("\nNombre d'éléments par étage\n");
    uint8_t somme = 0;
    for (uint8_t i = 0; i < 16; ++i) {
        printf("%u\n", huff->n_par_etage[i]);
        somme += huff->n_par_etage[i];
    }
    printf("\nArray de symboles\n");
    for (uint8_t i = 0; i < somme; ++i) {
        printf("%u\n", huff->array_symboles[i]);
    }
    printf("\n");
}

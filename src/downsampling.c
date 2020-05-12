# include "downsampling.h"
# include "mcu.h"
# include "jpeg_writer.h"
# include <stdint.h>
void downsample(struct jpeg fichier, struct array_mcu mcus)
/* apply downsampling over every mcu in the array_mcu* */
{
    if (mcus.ct == COLOR) {
      // TODO: déterminer sf
      // pour Cb puis Cr
      for (int canal=1; canal<3; canal ++){
          // pour chaque MCU
          for (int i_mcu=0; i_mcu<mcus.height*mcus.width; i_mcu++) {
              // hauteur puis largeur
              for (int dimension=0; dimension<2; dimension++) {
                  uint8_t factor = mcus.sf[0+dimension]/mcus.sf[2*canal+dimension]; // Cas d'erreur si pas diviseur !!!
                  for (int i_classe=0; i_classe < factor, i_classe ++) {
                      for (int i=0; i<64; i++) {
                          // moyenne sur chaque coefficient par groupe
                          uint8_t moyenne = 0;
                          for (int i_bloc=i_classe*factor, i_bloc<(i_classe + 1)*factor, i_bloc ++) {
                              // Présupposé: la taille du bloc est 8x8
                              moyenne += mcus.data[canal][(i_mcu*mcus.height*mcus.width + i_bloc)*64 + i];
                          }
                          moyenne = moyenne/factor
                          // on attribue ensuite la moyenne au bloc correspondant à la simplification, à savoir le i_class ième
                          mcus.data[canal][(i_mcu*h*v + i_classe)*64 + i] = moyenne
                      }
                  // TODO: libérer la place
                  }
              }
          }
      }
    }
}

#ifndef _DOWNSAMPLING_H_
#define _DOWNSAMPLING_H_

#include <mcu.h>

/*
    Effectue le downsampling de mcu avec les informations de sampling factors
    de jpg
*/
extern void downsample(struct jpeg* jpg, struct array_mcu* mcu);

#endif /* _DOWNSAMPLING_H_ */

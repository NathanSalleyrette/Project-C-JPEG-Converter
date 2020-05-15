#ifndef _DCT_H_
#define _DCT_H_
#include <mcu.h>

extern float c(int i, int j);

extern int16_t phi(struct array_mcu *mcus, size_t canal, int i_mcu, int i_bloc, int i, int j);

extern void dct_bloc(struct array_mcu *mcus, size_t canal, int i_mcu, int i_bloc);

extern void dct(struct array_mcu *mcus);

#endif /* _DCT_H_ */

#ifndef _DOWNSAMPLING_H_
#define _DOWNSAMPLING_H_

# include "mcu.h"

extern uint8_t* set_sf(struct jpeg* jpg, struct array_mcu* mcus);

extern void downsample(struct jpeg* jpg, struct array_mcu* mcus);

#endif /* _DOWNSAMPLING_H_ */

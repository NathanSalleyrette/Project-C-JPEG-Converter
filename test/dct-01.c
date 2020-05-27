# include <dct.h>
# include <mcu.h>
# include <stdio.h>
# include <stdint.h>
/* the dct module has to be modified so that blocs are size 2x2 */

int main(void)
{
    struct array_mcu* mcus = malloc(sizeof(struct array_mcu));
    // 2x1 mcus and 2 blocs of 2x2 by color_component
    mcus->height = 2;
    mcus->width = 1;
    uint8_t sf[] = {2, 1, 2, 1, 2, 1};
    mcus->sf =  sf;
    mcus->ct = COLOR;
    int16_t y[] = {111, 112, 113, 114, 121, 122, 123, 124, 211, 212, 213, 214, 221, 222, 223, 224};
    int16_t cb[] = {111, 112, 113, 114, 121, 122, 123, 124, 211, 212, 213, 214, 221, 222, 223, 224};
    int16_t cr[] = {111, 112, 113, 114, 121, 122, 123, 124, 211, 212, 213, 214, 221, 222, 223, 224};
    int16_t *data[] = {y, cb, cr};
    mcus->data = data;
    dct(mcus);
    for (int canal = 0; canal <3; canal ++) {
        printf("canal %d \n", canal);
        for (int i = 0; i<16; i++) {
            printf("%d ,", mcus->data[canal][i]);
        }
        printf("\n");
    }
    free(mcus);
}

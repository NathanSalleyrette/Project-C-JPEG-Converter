#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <jpeg_writer.h>
#include <mcu.h>


/* Function creating an array_mcu struct from extracting pixel data from the input file. */
struct array_mcu *get_mcu_from_jpeg(struct jpeg *jpeg)
{
	struct array_mcu *mcus = malloc(sizeof(struct array_mcu));
	mcus->sf = malloc(6 * sizeof(uint8_t));
	mcus->sf[0] = jpeg_get_sampling_factor(jpeg, Y, H);
	mcus->sf[1] = jpeg_get_sampling_factor(jpeg, Y, V);
	/* Before subsampling, sampling factors for Cb and Cr are set to the same value as for Y */
	for (uint8_t i = 2; i < 5; i += 2) {
		mcus->sf[i] = mcus->sf[0];
		mcus->sf[i + 1] = mcus->sf[1];
	}

	mcus->ct = jpeg_get_nb_components(jpeg);
	mcus->width = (jpeg_get_image_width(jpeg) + (mcus->sf[0] << 3) - 1) / (mcus->sf[0] << 3);
	mcus->height = (jpeg_get_image_height(jpeg) + (mcus->sf[1] << 3) - 1) / (mcus->sf[1] << 3);

	/* Creating the data array */
	uint32_t mcu_pixel_width = mcus->sf[0] << 3;
	uint32_t mcu_pixel_height = mcus->sf[1] << 3;
	uint32_t pixel_per_mcu = mcu_pixel_width * mcu_pixel_height;
	mcus->data = malloc(mcus->ct * sizeof(int16_t *));
	for (uint8_t i = 0; i < mcus->ct; i++) {
		mcus->data[i] = malloc(pixel_per_mcu * mcus->width * mcus->height * sizeof(int16_t));
	}

	FILE *image = fopen(jpeg_get_ppm_filename(jpeg), "r");
	/* Reading passed the header */
	for (uint8_t i = 0; i < 4; i++) {
		while (!isspace(fgetc(image)));
	}
	/* Reading all pixels and writing them into mcu->data in the right order */
	for (uint32_t y_mcu = 0; y_mcu < mcus->height; y_mcu++) {
		for (uint8_t y_block = 0; y_block < mcus->sf[1]; y_block++) {
			for (uint32_t y_pixel = 0; y_pixel < 8; y_pixel++) {
				for (uint32_t x_mcu = 0; x_mcu < mcus->width; x_mcu++) {
					for (uint8_t x_block = 0; x_block < mcus->sf[0]; x_block++) {
						for (uint32_t x_pixel = 0; x_pixel < 8; x_pixel++) {
							for (uint8_t component = 0; component < mcus->ct; component++) {
								if (feof(image)) {
									printf("Input file \'%s\' is not correctly encoded. (Less values encountered than expected)\n", jpeg_get_ppm_filename(jpeg));
									return NULL;
								}
								mcus->data[component][pixel_per_mcu * (x_mcu + mcus->width * y_mcu) + 64 * (x_block + mcus->sf[0] * y_block) + x_pixel + 8 * y_pixel] = (uint16_t) fgetc(image);
							}
						}
					}

				}
			}
		}
	}
	fgetc(image);
	if (!feof(image)) {
		printf("Input file \'%s\' is not correctly encoded. (More values encountered than expected)\n", jpeg_get_ppm_filename(jpeg));
		return NULL;
	}
	fclose(image);
	return mcus;
}

/* Function deleting an array_mcu struct and freeing the allocated memory. */
void delete_mcu(struct array_mcu *mcus)
{
	if (mcus == NULL) {
		return;
	}
	for (uint8_t component = 0; component < mcus->ct; component++) {
		free(mcus->data[component]);
	}
	free(mcus->data);
	free(mcus->sf);
	free(mcus);
}

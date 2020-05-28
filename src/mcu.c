#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <jpeg_writer.h>
#include <mcu.h>

size_t get_indice_from_coordinates(struct array_mcu *mcu, uint8_t canal, size_t x, size_t y)
{
    uint8_t h = mcu->sf[2*canal];
    uint8_t v = mcu->sf[2*canal+1];
    size_t i_mcu_x = x/(8*h);
    size_t i_mcu_y = y/(8*v);
    x = x % (8*h);
    y = y % (8*v);
    uint8_t i_bloc_x = x/8;
    uint8_t i_bloc_y = y/8;
    x = x % 8;
    y = y % 8;
    return (i_mcu_x + i_mcu_y*mcu->width)*64*h*v + (i_bloc_x + i_bloc_y*h)*64 + x + y*8;
}

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
		char character = fgetc(image);
		while (character == '#') {
			while (fgetc(image) != '\n');
			character = fgetc(image);
		}
		while (!isspace(character))
			character = fgetc(image);
	}
	/* Reading all pixels and writing them into mcu->data in the right order */
	for (uint32_t y = 0; y < jpeg_get_image_height(jpeg); y++) {
		for (uint32_t x = 0; x < jpeg_get_image_width(jpeg); x++) {
			for (uint8_t component = 0; component < mcus->ct; component++) {
				if (feof(image)) {
					printf("Input file \'%s\' is not correctly encoded. (Less values encountered than expected)\n", jpeg_get_ppm_filename(jpeg));
					return NULL;
				}
				mcus->data[component][get_indice_from_coordinates(mcus, 0, x, y)] = (uint16_t) fgetc(image);
			}
		}
	}
	/* Copying the last pixel column into the empty slots to the right */
	for (uint32_t y = 0; y < jpeg_get_image_height(jpeg); y++) {
		for (uint32_t x = jpeg_get_image_width(jpeg); x < mcu_pixel_width * mcus->width; x++) {
			for (uint8_t component = 0; component < mcus->ct; component++) {
				mcus->data[component][get_indice_from_coordinates(mcus, 0, x, y)] = mcus->data[component][get_indice_from_coordinates(mcus, 0, x - 1, y)];
			}
		}
	}
	/* Copying the last pixel row into the empty slots at the bottom */
	for (uint32_t x = 0; x < mcu_pixel_width * mcus->width; x++) {
		for (uint32_t y = jpeg_get_image_height(jpeg); y < mcu_pixel_height * mcus->height; y++) {
			for (uint8_t component = 0; component < mcus->ct; component++) {
				mcus->data[component][get_indice_from_coordinates(mcus, 0, x, y)] = mcus->data[component][get_indice_from_coordinates(mcus, 0, x, y - 1)];
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

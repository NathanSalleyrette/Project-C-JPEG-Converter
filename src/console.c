#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <console.h>

enum FileType {
	PPM,
	PGM
};

/* Function returning a jpeg struct containing all the data extracted from the command line */
struct jpeg *get_jpeg_from_console(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage : \'%s [--options] <input_file>\'\n", argv[0]);
		printf("Try : \'%s --help\' for more information.\n", argv[0]);
		return NULL;
	}

	/* --help option */
	if (strcmp(argv[1], "--help") == 0 && argc == 2) {
		printf("Usage : \'%s [--options] <input_file>\'\n", argv[0]);
		printf("This program allows you to encode a PPM or PGM file into a JPEG image.\n");
		printf("It only supports binary formats with 256 shades for each component for input files.\n\n");
		printf("Possible options are :\n\n");
		printf("--outfile=output_file.jpg\n");
		printf("Specifies the name for the ouptut file, which is by default set to <input_file>.jpg.\n\n");
		printf("--sample=h1xv1,h2xv2,h3xv3\n");
		printf("Specifies the sampling factors hxv for the 3 color componants (RGB), which are by default set to 1x1.\n");
		return NULL;
	}

	/* flags to check that options are called only once */
	bool outfile_flag = false;
	bool sample_flag = false;

	/* jpeg struct containing all the data extracted */
	struct jpeg *jpeg = jpeg_create();
	uint8_t sampling_factors[] = {1, 1, 1, 1, 1, 1};

	for (int32_t i = 1; i < argc - 1; i++) {

		/* --outfile option */
		if (!strncmp(argv[i], "--outfile=", 10)) {
			if (outfile_flag) {
				printf("\'--outfile\' option should only be called once.\n");
			}
			char *output_filename = &argv[i][10];
			size_t name_length = strlen(output_filename);
			if (name_length < 4 || strcmp(&output_filename[name_length - 4], ".jpg")) {
				printf("Output filename \'%s\' should have \'.jpg\' extension.\n", output_filename);
				return NULL;
			}
			jpeg_set_jpeg_filename(jpeg, output_filename);
			outfile_flag = true;
		}

		/* --sample option */
		else if (!strncmp(argv[i], "--sample=", 9)) {
			if (sample_flag) {
				printf("\'--sample\' option should only be called once.\n");
				return NULL;
			}
			uint8_t sampling_factors[] = {0, 0, 0, 0, 0, 0};
			uint32_t j = 8;
			uint8_t factor_index = 0;
			
			/* Reading option and testing if syntax is valid */
			while (argv[i][j] != '\0') {
				j++;
				if (!isdigit(argv[i][j])) {
					printf("%c\n", argv[i][j]);
					printf("Invalid syntax : expected \'--sample=h1xv1,h2xv2,h3xv3\' where hi and vi are decimal unsigned integer.\n");
					return NULL;
				}
				while (isdigit(argv[i][j])) {
					sampling_factors[factor_index] = 10 * sampling_factors[factor_index] + argv[i][j] - '0';
					j++;
				}
				if (!((factor_index % 2 == 0 && argv[i][j] == 'x') || (factor_index % 2 && factor_index < 5 && argv[i][j] == ',') || factor_index >= 5)) {
					printf("Invalid syntax : expected \'--sample=h1xv1,h2xv2,h3xv3\' where hi and vi are decimal unsigned integer.\n");
					return NULL;
				}
				factor_index++;
			}

			/* Testing if sampling factors are valid */
			uint8_t sum = 0;
			for (uint8_t factor_index = 0; factor_index < 6; factor_index += 2) {
				if (sampling_factors[factor_index] < 1 || sampling_factors[factor_index] > 4 || sampling_factors[factor_index + 1] < 1 || sampling_factors[factor_index + 1] > 4) {
					printf("Invalid argument : h and v should all be between 1 and 4.\n");
					return NULL;
				}
				sum += sampling_factors[factor_index] * sampling_factors[factor_index + 1];
			}
			for (uint8_t factor_index = 2; factor_index < 6; factor_index += 2) {
				if (sampling_factors[0] % sampling_factors[factor_index] != 0 || sampling_factors[1] % sampling_factors[factor_index + 1] != 0) {
					printf("Invalid argument : sampling factors h2 and h3 should divide h1 and sampling factors v2 and v3 should divide v1.\n");
					return NULL;
				}
			}
			if (sum > 10) {
				printf("Invalid argument : h1 * v1 + h2 * v2 + h3 * v3 should be less or equal 10.\n");
				return NULL;
			}
			sample_flag = true;
		}

		/* Default case if option is invalid */
		else {
			printf("\'%s\' is not a valid option.\n", argv[i]);
			printf("Try : \'%s --help\' for more information.\n", argv[0]);
			return NULL;
		}
	}

	/* Writing sampling factors in the jpeg struct */
	uint8_t factor_index = 0;
	for (uint8_t color = 0; color != NB_COLOR_COMPONENTS; color++) {
		for (uint8_t direction = 0; direction != NB_DIRECTIONS; direction++) {
			jpeg_set_sampling_factor(jpeg, (enum color_component)color, (enum direction)direction, sampling_factors[factor_index]);
			factor_index++;
		}
		factor_index++;
	}

	/* Checking if input file is correct */
	char *input_filename = argv[argc - 1];
	size_t name_length = strlen(input_filename);
	enum FileType type;
	if (name_length < 4) {
		printf("Input file \'%s\' does not have \'.ppm\' or \'.pgm\' extension.\n", input_filename);
		return NULL;
	}
	else if (!strcmp(&input_filename[name_length - 4], ".ppm")) {
		type = PPM;
	}
	else if (!strcmp(&input_filename[name_length - 4], ".pgm")) {
		type = PGM;
	}
	else {
		printf("Input file \'%s\' does not have \'.ppm\' or \'.pgm\' extension.\n", input_filename);
		return NULL;
	}
	FILE *image = fopen(input_filename, "r");
	if (image == NULL) {
		printf("Input file \'%s\' does not exist.\n", input_filename);
		return NULL;
	}
	/* Writing input filename to the jpeg struct */
	jpeg_set_ppm_filename(jpeg, input_filename);

	/* Checking file header and extracing image dimensions */
	if (fgetc(image) != 'P') {
		printf("Input file \'%s\' is not correctly encoded. (Wrong magic number)\n", input_filename);
		return NULL;
	}
	if (fgetc(image) != '5' + (type == PPM)) {
		printf("This JPEG encoder only supports binary files (with magic number \'P%i\' for P%cM) as input file.\n", 5 + (type == PPM), type == PPM ? 'P' : 'G');
		return NULL;
	}
	if (!isspace(fgetc(image))) {
		printf("Input file \'%s\' is not correctly encoded. (No spacing character found after magic number)\n", input_filename);
		return NULL;
	}
	uint32_t width = 0;
	char digit = fgetc(image);
	if (!isdigit(digit)) {
		printf("Input file \'%s\' is not correctly encoded. (Wrong width)\n", input_filename);
		return NULL;
	}
	while (isdigit(digit)) {
		width *= 10;
		width += digit - '0';
		digit = fgetc(image);
	}
	if (!isspace(digit)) {
		printf("Input file \'%s\' is not correctly encoded. (No spacing character found after width)\n", input_filename);
		return NULL;
	}
	uint32_t height = 0;
	digit = fgetc(image);
	if (!isdigit(digit)) {
		printf("Input file \'%s\' is not correctly encoded. (Wrong height)\n", input_filename);
		return NULL;
	}
	while (isdigit(digit)) {
		height *= 10;
		height += digit - '0';
		digit = fgetc(image);
	}
	if (!isspace(digit)) {
		printf("Input file \'%s\' is not correctly encoded. (No spacing character found after height)\n", input_filename);
		return NULL;
	}
	uint32_t max = 0;
	digit = fgetc(image);
	if (!isdigit(digit)) {
		printf("Input file \'%s\' is not correctly encoded. (Wrong maximum color value)\n", input_filename);
		return NULL;
	}
	while (isdigit(digit)) {
		max *= 10;
		max += digit - '0';
		digit = fgetc(image);
	}
	if (max != 255) {
		printf("This JPEG encoder ony support input files with 256 shades per color (maximum value of 255).\n");
		return NULL;
	}
	if (!isspace(digit)) {
		printf("Input file \'%s\' is not correctly encoded. (No spacing character found after maximum color value)\n", input_filename);
		return NULL;
	}
	fclose(image);
	/* Writing image dimensions and number of colors in the jpeg struct */
	jpeg_set_nb_components(jpeg, 1 + 2 * (type == PPM));
	jpeg_set_image_width(jpeg, width);
	jpeg_set_image_height(jpeg, height);


	/* Writing default output filename to the jpeg struct if hasn't been specified */
	if (!outfile_flag) {
		char output_filename[name_length + 1];
		strcpy(output_filename, input_filename);
		strcpy(&output_filename[name_length - 3], "jpg");
		jpeg_set_jpeg_filename(jpeg, output_filename);
	}

	return jpeg;
}

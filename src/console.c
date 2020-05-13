#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <console.h>


struct jpeg *get_jpeg_from_console(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage : \'%s [--options] <input_file>\'\n", argv[0]);
		printf("Try : \'%s --help\' for more information.\n", argv[0]);
		return NULL;
	}
	/* --help option */
	if (strcmp(argv[1], "--help") == 0 && argc == 2) {
		printf("Usage : \'%s [--options] <input_file>\'\n\n", argv[0]);
		printf("Possible options are :\n");
		printf("--outfile=output_file.jpg\n");
		printf("Specifies the name for the ouptut file, which is by default set to <input_file>.jpg.\n\n");
		printf("--sample=h1xv1,h2xv2,h3xv3\n");
		printf("Specifies the sampling factors hxv for the 3 color componants (RGB), which are set by default to 1x1.\n");
		return NULL;
	}
	/* flags to check that options are called only once */
	bool outfile_flag = false;
	bool sample_flag = false;
	/* jpeg struct containing all the data extracted */
	struct jpeg *infos = jpeg_create();
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
			jpeg_set_jpeg_filename(infos, output_filename);
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
		else {
			printf("\'%s\' is not a valid option.\n", argv[i]);
			printf("Try : \'%s --help\' for more information.\n", argv[0]);
			return NULL;
		}
	}
	char *input_filename = argv[argc - 1];
	size_t name_length = strlen(input_filename);
	if (name_length < 4 || (strcmp(&input_filename[name_length - 4], ".ppm") && strcmp(&input_filename[name_length - 4], ".pgm"))) {
		printf("Input file \'%s\' does not have \'.ppm\' or \'.pgm\' extension.\n", input_filename);
		return NULL;
	}
	FILE *image = fopen(input_filename, "r");
	if (image == NULL) {
		printf("Input file \'%s\' does not exist.\n", input_filename);
		return NULL;
	}
	fclose(image);
	jpeg_set_ppm_filename(infos, input_filename);
	if (!outfile_flag) {
		char output_filename[name_length + 1];
		strcpy(output_filename, input_filename);
		strcpy(&output_filename[name_length - 3], "jpg");
		jpeg_set_jpeg_filename(infos, output_filename);
	}
	return infos;
}

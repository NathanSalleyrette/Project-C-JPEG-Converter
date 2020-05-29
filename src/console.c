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
		printf("--dct=<arg>\n");
		printf("Specifies what algorithm to use when calculating the discrete cosine transform. <arg> should be one of the following:\n");
		printf("\'standard\' : Basic and rather slow algorithm. (used by default)\n");
		printf("\'loeffler\' : Loeffler's algorithm, rather fast.\n");
		printf("\'fast\'     : The fastest option, with a cost in precision.\n\n");
		printf("--huffman=<arg>\n");
		printf("Specifies what type of huffman table to use. <arg> should be one of the following:\n");
		printf("\'static\'  : Use precalculated tables. (used by default)\n");
		printf("\'dynamic\' : Generate tables from the image data.\n\n");
		printf("--outfile=<output_file>\n");
		printf("Specifies the name for the ouptut file, which is by default set to <input_file>.jpg.\n\n");
		printf("--quantification=<arg>\n");
		printf("Specifies whether quantification should be done with or without loss. <arg> should be either \'loss\' or \'lossless\'. (default is lossless)\n\n");
		printf("--sample=h1xv1,h2xv2,h3xv3\n");
		printf("Specifies the sampling factors hxv for the 3 color components (RGB), which are by default set to 1x1.\n");
		return NULL;
	}

	/* flags to check that options are called only once */
	bool outfile_flag = false;
	bool sample_flag = false;
	bool huffman_flag = false;
	bool quantification_flag = false;
	bool dct_flag = false;

	/* jpeg struct containing all the data extracted */
	struct jpeg *jpeg = jpeg_create();
	uint8_t sampling_factors[] = {1, 1, 1, 1, 1, 1};
	/* Setting default flags */
	jpeg_set_huffman_type(jpeg, false);
	jpeg_set_loss(jpeg, false);
	jpeg_set_dct_type(jpeg, STANDARD);

	for (int32_t i = 1; i < argc - 1; i++) {

		/* --outfile option */
		if (!strncmp(argv[i], "--outfile=", 10)) {
			if (outfile_flag) {
				printf("\'--outfile\' option should only be called once.\n");
				return NULL;
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
			for (uint8_t factor_index = 0; factor_index < 6; factor_index++)
				sampling_factors[factor_index] = 0;
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

		/* --huffman option */
		else if (!strncmp(argv[i], "--huffman=", 10)) {
			if (huffman_flag) {
				printf("\'--huffman\' option should only be called once.\n");
				return NULL;
			}
			if (!strcmp(&argv[i][10], "static")) 
				jpeg_set_huffman_type(jpeg, false);
			else if (!strcmp(&argv[i][10], "dynamic"))
				jpeg_set_huffman_type(jpeg, true);
			else {
				printf("Invalid argument : \'%s\' is not a valid parameter for --huffman option.\n", &argv[i][10]);
				return NULL;
			}
			huffman_flag = true;
		}

		/* --quantification option */
		else if (!strncmp(argv[i], "--quantification=", 17)) {
			if (quantification_flag) {
				printf("\'--quantification\' option should only be called once.\n");
				return NULL;
			}
			if (!strcmp(&argv[i][17], "lossless")) 
				jpeg_set_loss(jpeg, false);
			else if (!strcmp(&argv[i][17], "loss"))
				jpeg_set_loss(jpeg, true);
			else {
				printf("Invalid argument : \'%s\' is not a valid parameter for --quantification option.\n", &argv[i][17]);
				return NULL;
			}
			quantification_flag = true;
		
		}
		/* --dct option */
		else if (!strncmp(argv[i], "--dct=", 6)) {
			if (dct_flag) {
				printf("\'--dct\' option should only be called once.\n");
				return NULL;
			}
			if (!strcmp(&argv[i][6], "standard")) 
				jpeg_set_dct_type(jpeg, STANDARD);
			else if (!strcmp(&argv[i][6], "loeffler"))
				jpeg_set_dct_type(jpeg, LOEFFLER);
			else if (!strcmp(&argv[i][6], "fast"))
				jpeg_set_dct_type(jpeg, INTEGER);
			else {
				printf("Invalid argument : \'%s\' is not a valid parameter for --dct option.\n", &argv[i][6]);
				return NULL;
			}
			dct_flag = true;
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
	}

	/* Checking if input file is correct */
	char *input_filename = argv[argc - 1];
	size_t name_length = strlen(input_filename);
	enum FileType type;
	if (name_length < 4) {
		printf("Input file \'%s\' does not have \'.ppm\' or \'.pgm\' extension.\n", input_filename);
		return NULL;
	}
	else if (!strcmp(&input_filename[name_length - 4], ".ppm")) 
		type = PPM;
	else if (!strcmp(&input_filename[name_length - 4], ".pgm")) { 
		if (sample_flag)
			printf("Sampling factors will be ignored for Cb and Cr.\n");
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
	/* Reading passed comments */
	char character = fgetc(image);
	while (character == '#') {
		while (fgetc(image) != '\n' || !feof(image));
		character = fgetc(image);
	}
	/* Reading magic number */
	if (character != 'P') {
		printf("Input file \'%s\' is not correctly encoded. (Wrong magic number)\n", input_filename);
		return NULL;
	}
	if (fgetc(image) != '5' + (type == PPM)) {
		printf("This JPEG encoder only supports binary files (with magic number \'P%i\' for P%cM) as input file.\n", 5 + (type == PPM), type == PPM ? 'P' : 'G');
		return NULL;
	}
	char spacing = fgetc(image);
	if (!isspace(spacing)) {
		printf("Input file \'%s\' is not correctly encoded. (No spacing character found after magic number)\n", input_filename);
		return NULL;
	}
	uint32_t width = 0;
	char digit = fgetc(image);
	/* Reading passed comments */
	if (spacing == '\n') {
		while (digit == '#') {
			while (fgetc(image) != '\n' || !feof(image));
			digit = fgetc(image);
		}
	}
	/* Reading image width */
	if (!isdigit(digit)) {
		printf("Input file \'%s\' is not correctly encoded. (Wrong width)\n", input_filename);
		return NULL;
	}
	while (isdigit(digit)) {
		width *= 10;
		width += digit - '0';
		digit = fgetc(image);
	}
	spacing = digit;
	if (!isspace(spacing)) {
		printf("Input file \'%s\' is not correctly encoded. (No spacing character found after width)\n", input_filename);
		return NULL;
	}
	uint32_t height = 0;
	digit = fgetc(image);
	/* Reading passed comments */
	if (spacing == '\n') {
		while (digit == '#') {
			while (fgetc(image) != '\n' || !feof(image));
			digit = fgetc(image);
		}
	}
	/* Reading image height */
	if (!isdigit(digit)) {
		printf("Input file \'%s\' is not correctly encoded. (Wrong height)\n", input_filename);
		return NULL;
	}
	while (isdigit(digit)) {
		height *= 10;
		height += digit - '0';
		digit = fgetc(image);
	}
	spacing = digit;
	if (!isspace(spacing)) {
		printf("Input file \'%s\' is not correctly encoded. (No spacing character found after height)\n", input_filename);
		return NULL;
	}
	uint32_t max = 0;
	digit = fgetc(image);
	/* Reading passed comments */
	if (spacing == '\n') {
		while (digit == '#') {
			while (fgetc(image) != '\n' || !feof(image));
			digit = fgetc(image);
		}
	}
	/* Reading maximum color value */
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

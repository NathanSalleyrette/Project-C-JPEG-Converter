#include <stdlib.h>
#include <stdio.h>
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
	for (int i = 1; i < argc - 1; i++) {
		/* --outfile option */
		if (!strncmp(argv[i], "--outfile=", 10)) {
			if (outfile_flag) {
				printf("\'--outfile\' option should only be called once.\n");
			}
			char *output_filename = &argv[i][10];
			size_t name_length = strlen(output_filename);
			if (name_length < 4 || strcmp(&output_filename[name_length - 5], ".jpeg")) {
				printf("Output filename \'%s\' should have \'.jpeg\' extension.\n", output_filename);
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
		char output_filename[name_length + 2];
		strcpy(output_filename, input_filename);
		strcpy(&output_filename[name_length - 4], ".jpeg");
		jpeg_set_jpeg_filename(infos, output_filename);
	}
	return infos;
}

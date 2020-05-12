#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <console.h>

void print_wrong(char **argv)
{
	printf("Usage : \'%s [--options] <input_file>\'\n", argv[0]);
	printf("Try : \'%s --help\' for more information.\n", argv[0]);
}

void print_help(char **argv)
{
	printf("Usage : \'%s [--options] <input_file>\'\n\n", argv[0]);
	printf("Possible options are :\n");
	printf("--outfile=output_file.jpg\n");
	printf("Specifies the name for the ouptut file, which is by default set to <input_file>.jpg.\n\n");
	printf("--sample=h1xv1,h2xv2,h3xv3\n");
	printf("Specifies the sampling factors hxv for the 3 color componants (RGB), which are set by default to 1x1.\n");
}

struct jpeg *get_jpeg_from_console(int argc, char **argv)
{
	if (argc < 2) {
		print_wrong(argv);
		return NULL;
	}
	if (strcmp(argv[1], "--help") == 0 && argc == 2) {
		print_help(argv);
		return NULL;
	}
	bool outfile_flag = false;
	bool sample_flag = false;
	struct jpeg *infos = jpeg_create();
	for (int i = 1; i < argc - 1; i++) {
		if (strncmp(argv[i], "--outfile=", 10) == 0) {
			if (outfile_flag) {
				printf("--outfile option should only be called once.\n");
			}
			if (argv[i][10] == '\0') {
				printf("--outfile option cannot be empty.\n");
				return NULL;
			}
			jpeg_set_jpeg_filename(infos, &argv[i][10]);
			outfile_flag = true;
		}
		else if (strncmp(argv[i], "--sample=", 9) == 0) {
			if (sample_flag) {
				printf("--sample option should only be called once.\n");
				return NULL;
			}
			sample_flag = true;
		}
		else {
			printf("%s is not a valid option.\n", argv[i]);
			print_wrong(argv);
			return NULL;
		}
	}
	jpeg_set_ppm_filename(infos, argv[argc - 1]);
	return infos;
}

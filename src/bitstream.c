#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <bitstream.h>

struct bitstream {
	FILE *image;
	uint8_t buffer;
	uint8_t index;
};

/* Retourne un nouveau bitstream prêt à écrire dans le fichier filename. */
struct bitstream *bitstream_create(const char *filename)
{
	struct bitstream *stream = malloc(sizeof(struct bitstream));
	stream->image = fopen(filename, "wb");
	stream->buffer = 0;
	stream->index = 0;
	return stream;
}

/*
Ecrit nb_bits bits dans le bitstream. La valeur portée par cet ensemble de
bits est value. Le paramètre is_marker permet d'indiquer qu'on est en train
d'écrire un marqueur de section dans l'entête JPEG ou non.
*/
void bitstream_write_bits(struct bitstream *stream, uint32_t value, uint8_t nb_bits, bool is_marker)
{
	for (uint8_t i = 0; i < nb_bits; i++) {
		stream->buffer += ((value >> (nb_bits - i - 1)) % 2) << (7 - stream->index);
		stream->index++;
		if (stream->index == 8) {
			stream->index = 0;
			bool ff_flag = (stream->buffer == 255);
			fwrite(&stream->buffer, 1, 1, stream->image);
			stream->buffer = 0;
			if (ff_flag && !is_marker)
				fwrite(&stream->buffer, 1, 1, stream->image);
		}
	}
}

/* Force l'exécution des écritures en attente sur le bitstream, s'il en existe. */
void bitstream_flush(struct bitstream *stream)
{
	if (stream->index != 0) {
		fwrite(&stream->buffer, 1, 1, stream->image);
		stream->buffer = 0;
		stream->index = 0;
	}
}

/* Détruit le bitstream passé en paramètre, en libérant la mémoire qui lui est associée. */
void bitstream_destroy(struct bitstream *stream)
{
	fclose(stream->image);
	free(stream);
}

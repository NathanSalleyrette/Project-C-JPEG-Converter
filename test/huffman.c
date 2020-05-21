#include <huffman.h>
#include <mcu.h>
#include <stdio.h>
#include <zigzag.h>

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    // Test de get_huffman_from_freq
    uint8_t n = 6;
    uint32_t frequences[6] = {10, 100, 50, 37, 40, 50};
    struct huffman *huff = get_huffman_from_freq(frequences, n);
    description_huffman(huff);
    delete_huffman(huff);
    return EXIT_SUCCESS;
}

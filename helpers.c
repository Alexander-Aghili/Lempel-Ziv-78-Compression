#include "helpers.h"

/*
    Argument parser:
        - Default values are passed in.
        - Sets values from command line as appropriate
        - Opens files and error checks if necessary
*/
int argparser(int argc, char **argv, int *input_file, int *output_file, bool *verbose, bool *help) {
    int opt = 0;
    int fd;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i':
            fd = open(optarg, O_RDONLY);
            *input_file = fd;
            if (fd < 0) {
                perror(NULL);
                return 1;
            }
            break;
        case 'o':
            fd = open(optarg, O_CREAT + O_WRONLY + O_TRUNC, S_IRUSR + S_IWUSR);
            *output_file = fd;
            if (fd < 0) {
                perror(NULL);
                return 2;
            }
            break;
        case 'v': *verbose = true; break;
        case 'h': *help = true; return 4;
        default: *help = true; return 5;
        }
    }
    return 0;
}

/*
    Checks if fd is open (>2... -1 = NULL, 0 = stdin, 1 = stdout, 2 = stderr), and if so, closes that fd.
*/
void check_null_and_close(int fd) {
    if (fd > 2) {
        int response = close(fd);
        if (response < 0) {
            perror(NULL);
        }
    }
}

/*
    Gets bit length of code
*/
uint8_t get_bitlength(uint16_t code) {
    uint8_t last_one_bit = 0;
    for (uint32_t i = 0; i < sizeof(uint16_t) * BYTE; i++) {
        if (((1 << i) & code) >> i) {
            last_one_bit = i;
        }
    }
    return last_one_bit + 1;
}

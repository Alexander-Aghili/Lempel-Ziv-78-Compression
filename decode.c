#include "code.h"
#include "helpers.h"
#include "endian.h"
#include "io.h"
#include "word.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

bool read_decode_header(int infile, int outfile);
void decode(int infile, int outfile);
void print_verbose(void);
void print_help(void);

int main(int argc, char **argv) {
    bool help = false;
    bool verbose = false;
    int input_file = 0;
    int output_file = 1;

    int response = argparser(argc, argv, &input_file, &output_file, &verbose, &help);

    if (response == 4) {
        print_help();
        return 0;
    }

    if (response != 0) {
        check_null_and_close(input_file);
        check_null_and_close(output_file);
        if (help) {
            print_help();
        }
        return -1;
    }

    bool valid_header = read_decode_header(input_file, output_file);
    if (!valid_header) {
        fprintf(stderr, "Bad Magic Number\n");
        return 1;
    }
    decode(input_file, output_file);

    if (verbose) {
        print_verbose();
    }

    check_null_and_close(input_file);
    check_null_and_close(output_file);

    return 0;
}

/*
    Decodes header from infile, verifies Magic number, sets permissions for outfile.
*/
bool read_decode_header(int infile, int outfile) {
    FileHeader fileheader;
    memset((void *) &fileheader, 0, sizeof(FileHeader)); //Clears padding to avoid valgrind errors
    read_header(infile, &fileheader);
    if (fileheader.magic != MAGIC) {
        return false;
    }
    fchmod(outfile, (mode_t) fileheader.protection);
    return true;
}

/*
    Decodes information from infile to outfile.
*/
void decode(int infile, int outfile) {
    WordTable *table = wt_create();
    uint8_t current_sym = 0;
    uint16_t current_code = 0;
    uint16_t next_code = START_CODE;
    while (read_pair(infile, &current_code, &current_sym, get_bitlength(next_code))) {
        table[next_code] = word_append_sym(table[current_code], current_sym);
        write_word(outfile, table[next_code]);
        next_code++;
        if (next_code == MAX_CODE) {
            wt_reset(table);
            next_code = START_CODE;
        }
    }
    flush_words(outfile);
    wt_delete(table);
    table = NULL;
}

void print_verbose(void) {
    uint64_t total_bytes = (total_bits / BYTE);
    fprintf(stderr, "Compresssed file size: %lu bytes\n", total_syms);
    fprintf(stderr, "Uncompressed file size: %lu bytes\n", total_bytes);
    fprintf(stderr, "Compresssion ratio: %02.02f%%\n",
        100 * (1 - ((float) total_syms / (float) total_bytes)));
}

void print_help(void) {
    printf("SYNOPSIS\n"
           "   Decompresses files with the LZ78 decompression algorithm.\n"
           "   Used with files compressed with the corresponding encoder.\n\n"

           "USAGE\n"
           "   ./decode [-vh] [-i input] [-o output]\n\n"

           "OPTIONS\n"
           "   -v          Display decompression statistics\n"
           "   -i input    Specify input to decompress (stdin by default)\n"
           "   -o output   Specify output of decompressed input (stdout by default)\n"
           "   -h          Display program usage\n");
}

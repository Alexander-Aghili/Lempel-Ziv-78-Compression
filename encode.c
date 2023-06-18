#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "io.h"
#include "code.h"
#include "trie.h"
#include "word.h"
#include "helpers.h"

void encode(int infile, int outfile);
void write_encode_header(int infile, int outfile);
void print_verbose(void);
void print_help(void);

/*
    Main function that gets arguments and runs encoding algorithms.
*/
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

    write_encode_header(input_file, output_file);
    encode(input_file, output_file);

    if (verbose) {
        print_verbose();
    }

    check_null_and_close(input_file);
    check_null_and_close(output_file);

    return 0;
}

/*
    Writes encoded header into file
*/
void write_encode_header(int infile, int outfile) {
    FileHeader fileheader;
    memset((void *) &fileheader, 0, sizeof(FileHeader)); //Clears padding to avoid valgrind errors
    struct stat stat_struct;
    fstat(infile, &stat_struct);
    fileheader.magic = MAGIC;
    fileheader.protection = stat_struct.st_mode;
    write_header(outfile, &fileheader);
}

/*
    Compressses infile into outfile
*/
void encode(int infile, int outfile) {
    TrieNode *root = trie_create();
    TrieNode *current_node = root;
    TrieNode *previous_node = NULL;
    uint8_t current_sym = 0;
    uint8_t previous_sym = 0;
    uint16_t next_code = START_CODE;

    while (read_sym(infile, &current_sym)) {
        TrieNode *next_node = trie_step(current_node, current_sym);
        if (next_node != NULL) {
            previous_node = current_node;
            current_node = next_node;
        } else {
            write_pair(outfile, current_node->code, current_sym, get_bitlength(next_code));
            current_node->children[current_sym] = trie_node_create(next_code);
            current_node = root;
            next_code++;
        }

        if (next_code == MAX_CODE) {
            trie_reset(root);
            current_node = root;
            next_code = START_CODE;
        }
        previous_sym = current_sym;
    }
    if (current_node != root) {
        write_pair(outfile, previous_node->code, previous_sym, get_bitlength(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }
    write_pair(outfile, STOP_CODE, 0, get_bitlength(next_code));
    flush_pairs(outfile);
    trie_delete(root);
    root = NULL;
}

/*
    Prints verbose output to stderr
*/
void print_verbose(void) {
    uint64_t total_bytes = (total_bits / BYTE);
    fprintf(stderr, "Compresssed file size: %lu bytes\n", total_bytes);
    fprintf(stderr, "Uncompressed file size: %lu bytes\n", total_syms);
    fprintf(stderr, "Compresssion ratio: %02.02f%%\n",
        100 * (1 - ((float) total_bytes / (float) total_syms)));
}

void print_help(void) {
    printf("SYNOPSIS\n"
           "   Compresses files using the LZ78 compression algorithm.\n"
           "   Compressed files are decompressed with the corresponding decoder.\n\n"

           "USAGE\n"
           "   ./encode [-vh] [-i input] [-o output]\n\n"

           "OPTIONS\n"
           "   -v          Display compression statistics\n"
           "   -i input    Specify input to compress (stdin by default)\n"
           "   -o output   Specify output of compressed input (stdout by default)\n"
           "   -h          Display program help and usage\n");
}

#include "io.h"
#include "endian.h"
#include "code.h"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define FILE_ERROR -1

#define BYTE 8

//Buffer struct that stores
// ptr (buffer)
// index
// length
//of the buffer.
typedef struct Buffer {
    uint8_t ptr[BLOCK];
    uint32_t index;
    uint32_t length;
} Buffer;

typedef uint16_t Bit;

//Two buffers, one for symbols and one for the pairs
static Buffer syms_buffer;
static Buffer pairs_buffer;

uint64_t total_syms = 0; // To count the symbols processed.
uint64_t total_bits = 0; // To count the bits processed.

void check_swap_endian_header(FileHeader *header);
void check_print_file_error(int response);
void flush_and_reset_buffer_to_file(int outfile, Buffer *buffer, uint64_t to_write);
void clear_and_reset_buffer_from_file(int infile, Buffer *buffer, uint64_t to_read);
void write_bits(int outfile, uint16_t bits, int bitlen);
void write_single_bit(Bit bit, int block, int position);
Bit get_single_bit(uint16_t bits, uint8_t bit_offset);

/*
    This function reads *to_read* number of bytes from file *infile* and places them in the buffer *buf*
    It does so by looping continuously until:
        A) An error occurs, in which case a FILE_ERROR (-1) is returned
        B) to_read bytes are read, and then the number of bytes read is returned.
        C) A call to read returned zero, meaning the end of file has been reached, in which case the function returns zero.
*/
int read_bytes(int infile, uint8_t *buf, int to_read) {
    int total_bytes_read = 0;
    int bytes_read = 0;
    uint8_t *curr_buf = buf;
    do {
        bytes_read = (int) read(infile, curr_buf, to_read);
        total_bytes_read += bytes_read;
        to_read -= bytes_read;
        curr_buf += (uint8_t) bytes_read;
        if (bytes_read < 0) {
            return FILE_ERROR;
        }

    } while (to_read > 0 && bytes_read != 0);
    return total_bytes_read;
}

/*
    Behaves similar to read_bytes. Writes bytes until all bytes are written or error occurs.
*/
int write_bytes(int outfile, uint8_t *buf, int to_write) {
    int total_byte_written = 0;
    int bytes_written = 0;
    uint8_t *curr_buf = buf;
    do {
        bytes_written = (int) write(outfile, curr_buf, to_write);
        total_byte_written += bytes_written;
        to_write -= bytes_written;
        curr_buf += (uint8_t) bytes_written;
        if (bytes_written < 0) {
            return FILE_ERROR;
        }
    } while (to_write > 0);
    return total_byte_written;
}

/*
    Swaps endianess of header if required.
*/
void check_swap_endian_header(FileHeader *header) {
    if (big_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    return;
}

/*
    Asserts that a file error has not occured.
*/
void check_print_file_error(int response) {
    if (response == FILE_ERROR) {
        perror(NULL);
        assert(response != FILE_ERROR);
    }
}

/*
    Reads header bytes from file and puts it into header.
*/
void read_header(int infile, FileHeader *header) {
    int response = read_bytes(infile, (uint8_t *) header, sizeof(FileHeader));
    check_print_file_error(response);
    check_swap_endian_header(header);
    total_syms += (sizeof(FileHeader));
    return;
}

/*
    Writes header details into file.
*/
void write_header(int outfile, FileHeader *header) {
    check_swap_endian_header(header);
    int response = write_bytes(outfile, (uint8_t *) header, sizeof(FileHeader));
    check_print_file_error(response);
    total_bits += (sizeof(FileHeader) * BYTE);
}

/*
    Flushes *to_write* bytes from *buffer* into *outfile* and sets buffer->index to zero.
*/
void flush_and_reset_buffer_to_file(int outfile, Buffer *buffer, uint64_t to_write) {
    total_bits += BYTE * to_write;
    int response = write_bytes(outfile, buffer->ptr, (int) to_write);
    memset(buffer->ptr, 0, BLOCK);
    buffer->index = 0;
    check_print_file_error(response);
}

/*
    Clears buffer by reading new bytes into buffer.
    Reads *to_read* (or less as returned by read_bytes) bytes and places them into *buffer*.
    Resets buffer->index to zero and buffer->length to # of bytes read to ensure no going over bytes read.
*/
void clear_and_reset_buffer_from_file(int infile, Buffer *buffer, uint64_t to_read) {
    memset(buffer->ptr, 0, BLOCK);
    int response = read_bytes(infile, buffer->ptr, to_read);
    check_print_file_error(response);

    buffer->index = 0;
    buffer->length = response;
    total_syms += response;
}

/*
    Reads symbol in amoritzed time by calling 
*/
bool read_sym(int infile, uint8_t *sym) {
    int response = 1;
    if (syms_buffer.index == 0 || syms_buffer.index == syms_buffer.length) {
        response = read_bytes(infile, syms_buffer.ptr, BLOCK);
        check_print_file_error(response);

        syms_buffer.index = 0;
        syms_buffer.length = response;
        total_syms += response;
    }

    *sym = syms_buffer.ptr[syms_buffer.index];
    syms_buffer.index = syms_buffer.index + 1;

    if (response == 0) {
        return false;
    }
    return true;
}

/*
    Places *bit* into *position* of *bits*
*/
uint16_t read_single_bit_with_position(uint16_t bits, Bit bit, int position) {
    return (uint16_t) (bit << position) | bits;
}

/*
    This function is highly complex. Given more time, I would definitely refactor this.
    An overview: Reads *bitlen* bits from pairs_buffer (or infile) and place those bits into *bits*.
    It does so bit by bit. 
    Of course, the retreival of data from disk is done in sizes of BLOCK. 
    This is checked if pairs_buffer.index == 0 (No data retreived yet) or pairs_buffer.index == (BLOCK*BYTE) (Buffer full).
*/
void read_bits(int infile, uint16_t *bits, int bitlen) {
    int block = pairs_buffer.index / BYTE;
    int position = pairs_buffer.index % BYTE;
    int original_position = position;

    for (int i = 0; i < bitlen; i++) {
        if (pairs_buffer.index == 0 || pairs_buffer.index == (BLOCK * BYTE)) {
            clear_and_reset_buffer_from_file(infile, &pairs_buffer, BLOCK);
            block = 0;
        }
        *bits = read_single_bit_with_position(*bits,
            get_single_bit(pairs_buffer.ptr[block], position % BYTE), position - original_position);
        pairs_buffer.index = pairs_buffer.index + 1;

        if (position == 15 || position == 7) {
            block++;
        }
        position++;
    }
}

/*
    Reads bits from infile and places into of code and sym appropriately. Bitlen is bitlength of code.
    If code is STOP_CODE, returns false indicating the end of the file.
*/
bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    *code = 0;
    read_bits(infile, code, bitlen);

    if (*code == STOP_CODE) {
        return false;
    }
    uint16_t temp_sym = 0;
    read_bits(infile, &temp_sym, sizeof(uint8_t) * BYTE);

    *sym = (uint8_t) temp_sym;

    return true;
}

/*
    Gets single bit from *bits* in position *bit_offset* and returns it (As a Bit).
*/
Bit get_single_bit(uint16_t bits, uint8_t bit_offset) {
    return (Bit) (bits >> bit_offset) & 1;
}

/*
    Writes *bit* into pairs_buffer at byte *block* and bit *position* in that block.
*/
void write_single_bit(Bit bit, int block, int position) {
    pairs_buffer.ptr[block] &= ~(1 << position);
    pairs_buffer.ptr[block] |= (bit << position);
}

/*
    Writes bits bit by bit from *bits* up to *bitlen* to *outfile*. 
*/
void write_bits(int outfile, uint16_t bits, int bitlen) {
    for (int i = 0; i < bitlen; i++) {
        if (pairs_buffer.index == (BLOCK * BYTE)) {
            flush_pairs(outfile);
        }

        int block = pairs_buffer.index / BYTE;
        int position = pairs_buffer.index % BYTE;
        Bit bit = get_single_bit(bits, i);
        write_single_bit(bit, block, position);
        pairs_buffer.index = pairs_buffer.index + 1;
    }
}

/*
    Writes *code* and *sym* into outfile. Bitlen is bit length of code.  
*/
void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    write_bits(outfile, code, bitlen);
    write_bits(outfile, (uint16_t) sym, sizeof(uint8_t) * BYTE);
}

/*
    Flushes pairs_buffer to *outfile*
*/
void flush_pairs(int outfile) {
    flush_and_reset_buffer_to_file(outfile, &pairs_buffer, (pairs_buffer.index / BYTE));
}

/*
    Writes Word *w* to *outfile* (or buffer).
*/
void write_word(int outfile, Word *w) {
    if (w->len + syms_buffer.index >= BLOCK) {
        flush_words(outfile);
    }

    for (uint32_t i = 0; i < w->len; i++) {
        syms_buffer.ptr[syms_buffer.index] = w->syms[i];
        syms_buffer.index = syms_buffer.index + 1;
    }
}

/*
    Flushes words in syms_buffer to outfile.  
*/
void flush_words(int outfile) {
    flush_and_reset_buffer_to_file(outfile, &syms_buffer, syms_buffer.index);
}

#pragma once

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>

#define OPTIONS "i:o:vh"
#define BYTE    8

int argparser(int argc, char **argv, int *input_file, int *output_file, bool *verbose, bool *help);

void check_null_and_close(int fd);

uint8_t get_bitlength(uint16_t code);

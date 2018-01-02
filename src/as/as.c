#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <btn/vector.h>
#include <btn/cstr.h>

#include "as.h"
#include "print.h"
#include "option.h"

static const as_params default_patt = {
    .oformat = AS_OF_OBJ,
    .iformat = AS_IF_ASM,
    .syntax  = AS_SYNTAX_PATT,
    .output_file = "out.obj",
    .out_hex = false,
    .out_bin = false,
    .out_sym = false,
    .out_lst = false,
};

static const as_params default_lccc  = {
    .oformat = AS_OF_LLF,
    .iformat = AS_IF_ASM,
    .syntax = AS_SYNTAX_LCCC,
    .output_file = "out.obj",
    .out_hex = false,
    .out_bin = false,
    .out_sym = false,
    .out_lst = false,
};

#if defined DEF_PATT
#define DEFAULT_PARAMS default_patt
#define COMPILE_STRING "Patt version: defaults to the Patt's assembler syntax and McGraw-Hill toolchain .obj format\n"
#elif defined DEF_LCCC
#define DEFAULT_PARAMS default_lccc
#define COMPILE_STRING "LCCC version: defaults to the LCCC assembler syntax and LLF binary output format\n"
#endif

extern const option as_options[];
extern size_t as_num_options;

void print_version(void)
{
    printf("LCCC LC-3 assembler v0.01a\n"
           "Copyright (C) 2017 Brandon Nguyen\n"
           "\n"
           COMPILE_STRING);
}

void print_help(void)
{
    printf("Usage: lccc-as [options...] <assembly source files...>\n");
    printf("Options:\n");
    print_options(as_options, as_num_options);
}

int main(int argc, char ** argv)
{
    // construct the params
    as_params driver_params = DEFAULT_PARAMS;
    vector(const char *) files;
    vector_ctor(&files, sizeof(const char *), NULL, NULL);

    parse_options(M_AS, as_options, as_num_options,
                  &driver_params, &files, argc, argv);

    if (vector_size(&files) == 0) {
        msg(M_AS, M_FATAL, "No input files");
        exit(AS_RET_NO_INPUT);
    }
}

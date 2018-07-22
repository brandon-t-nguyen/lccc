#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <btn/vector.h>
#include <btn/cstr.h>

#include "as.h"
#include "asm.h"
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
    .output_file = "out.o",
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
    // setup the context
    asm_context context;
    asm_context_ctor(&context);

    // construct the params
    context.params = DEFAULT_PARAMS;
    parse_options(M_AS, as_options, as_num_options,
                  &context.params, &context.file_paths, argc, argv);

    size_t num_files = vector_size(&context.file_paths);
    if (num_files == 0) {
        msg(M_AS, M_FATAL, "No input files");
        exit(AS_RET_NO_INPUT);
    }

    bool ok = true;

    // check validity of options
    if ((context.params.iformat == AS_IF_HEX || context.params.iformat == AS_IF_BIN) &&
        context.params.oformat != AS_OF_OBJ) {
        msg(M_AS, M_ERROR, "Output format must be 'obj' in order to use the 'bin' or 'hex' input formats");
        ok = false;
    }

    // open the files
    for (size_t i = 0; i < num_files; ++i) {
        const char * path = NULL;
        vector_get(&context.file_paths, i, &path);
        FILE * file = fopen(path, "r");
        if (file == NULL) {
            switch (errno) {
            case ENOENT:
                msg(M_AS, M_ERROR,
                    ANSI_F_BWHT "%s" ANSI_RESET ": No such file or directory",
                    path);
                break;
            default:
                msg(M_AS, M_ERROR,
                    ANSI_F_BWHT "%s" ANSI_RESET ": Unable to open file or directory",
                    path);
                break;
            }
            ok = false;
        } else {
            vector_push_back(&context.files, &file);
        }
    }

    if (!ok) {
        msg(M_AS, M_FATAL, "Errors caught");
        exit(AS_RET_BAD_INPUT);
    }

    asm_front(&context);
    asm_back(&context);

    as_ret ret = AS_RET_OK;
    if (context.error_count > 0) {
        msg(M_AS, M_FATAL, "%d errors found", context.error_count);
        ret = AS_RET_BAD_INPUT;
    }

    // TODO: emit the code

done:
    asm_context_dtor(&context);
    return ret;
}

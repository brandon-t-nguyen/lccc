#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <btn/vector.h>
#include <btn/cstr.h>
#include <btn/bst.h>

#include "as.h"
#include "asm.h"
#include "print.h"

static
ssize_t get_last_dot_index(const char * str)
{
    ssize_t len = strlen(str);
    ssize_t idx = len - 1;
    while (idx > -1) {
        if (str[idx] == '.')
            return idx;
        --idx;
    }
    return -1;
}
static
char * replace_extension(const char * str, const char * rep)
{
    char * buffer = NULL;
    ssize_t dot_idx = get_last_dot_index(str);
    if (dot_idx == -1) {
        // no dot: just append
        buffer = (char *) malloc(sizeof(char) * (strlen(str) + strlen(rep) + 1 + 1)); // + null and dot
        sprintf(buffer, "%s.%s", str, rep);
    } else {
        buffer = (char *) malloc(sizeof(char) * ((dot_idx + 1) + strlen(rep) + 1));
        strcpy(buffer, str);
        strcpy(buffer + (dot_idx + 1), rep);
    }
    return buffer;
}

#define ASSERT_OPEN(_file, _path, _mode)\
    _file = fopen(_path, _mode);\
    if (_file == NULL) {\
        switch (errno) {\
        case ENOENT:\
            msg(M_AS, M_ERROR,\
                ANSI_F_BWHT "%s" ANSI_RESET ": No such file or directory",\
                path);\
            break;\
        default:\
            msg(M_AS, M_ERROR,\
                ANSI_F_BWHT "%s" ANSI_RESET ": Unable to open file or directory",\
                path);\
            break;\
        }\
        ++error_count;\
        goto done;\
    }\

// true binary
static
void emit_binary(FILE * f, uint16_t word)
{
    uint8_t lo = (word & 0xF);
    uint8_t hi = ((word >> 8) & 0xF);
    fwrite(&hi, 1, 1, f);
    fwrite(&lo, 1, 1, f);
}

// plaintext hex
static
void emit_hex(FILE * f, uint16_t word)
{
    fprintf(f, "%04X\n", word);
}

// plaintext binary
static
void emit_bin(FILE * f, uint16_t word)
{
    for (int i = 0; i < 16; ++i) {
        if (word & 0x8000)
            fputc('1', f);
        else
            fputc('0', f);
        word <<= 1;
    }
    printf(f, "\n");
}

static
void emit(asm_context * context, uint16_t word)
{
    emit_binary(context->o_file, word);
    if (context->params.out_hex) {
        emit_hex(context->h_file, word);
    }

    if (context->params.out_bin) {
        emit_hex(context->b_file, word);
    }
}

int asm_emit_obj(asm_context * context)
{
    int error_count = 0;
    char * path  = NULL;
    ASSERT_OPEN(context->o_file, context->params.output_file, "w");

    if (context->params.out_hex) {
        path = replace_extension(context->params.output_file, "hex");
        ASSERT_OPEN(context->h_file, path, "w");
        free(path);
    }

    if (context->params.out_bin) {
        path = replace_extension(context->params.output_file, "bin");
        ASSERT_OPEN(context->b_file, path, "w");
        free(path);
    }

    if (context->params.out_sym) {
        path = replace_extension(context->params.output_file, "sym");
        ASSERT_OPEN(context->s_file, path, "w");
        free(path);
    }

    if (context->params.out_lst) {
        path = replace_extension(context->params.output_file, "lst");
        ASSERT_OPEN(context->l_file, path, "w");
        free(path);
    }

    // for .obj, section 0 of program 0 is the one we emit
    asm_program * prog = (asm_program *) vector_getp(&context->progs, 0);
    asm_section * sec  = (asm_section *) vector_getp(&prog->sections, 0);
    emit(context, prog->entry);
    size_t num_code = vector_size(&sec->code);
    for (size_t i = 0; i < num_code; ++i) {
        uint16_t word;
        vector_get(&sec->code, i, &word);
        emit(context, word);
    }

done:
    if (path != NULL) free(path);
    #define CLOSE_FILE(_file) if (_file != NULL) {fclose(_file); _file = NULL;}
    CLOSE_FILE(context->o_file);
    CLOSE_FILE(context->h_file);
    CLOSE_FILE(context->b_file);
    CLOSE_FILE(context->s_file);
    CLOSE_FILE(context->l_file);
    return error_count;
}

int asm_emit(asm_context * context)
{
    int error_count = 0;

    // open the needed files
    if (context->params.oformat == AS_OF_OBJ) {
        error_count += asm_emit_obj(context);
    } else {
    }

    context->error_count += error_count;
    return error_count;
}

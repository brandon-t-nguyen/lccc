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
#include <btn/iterator.h>

#include "as.h"
#include "asm.h"
#include "print.h"

static
size_t get_last_dot_index(const char * str)
{
    size_t len = strlen(str);
    size_t idx = len - 1;
    while (idx != SIZE_MAX) {
        if (str[idx] == '.')
            return idx;
        --idx;
    }
    return SIZE_MAX;
}
static
char * replace_extension(const char * str, const char * rep)
{
    char * buffer = NULL;
    size_t dot_idx = get_last_dot_index(str);
    if (dot_idx == SIZE_MAX) {
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
void emit_obj(FILE * f, uint16_t word)
{
    // .obj is big endian
    uint16_t rev = ((word & 0xFF) << 8) | ((word >> 8) & 0xFF);
    fwrite(&rev, 2, 1, f);
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
    fputc('\n', f);
}

static
void emit(asm_context * context, uint16_t word)
{
    emit_obj(context->o_file, word);
    if (context->params.out_hex) {
        emit_hex(context->h_file, word);
    }

    if (context->params.out_bin) {
        emit_bin(context->b_file, word);
    }
}

// writes out "traditional" symbol table
static
void write_symbol_table(asm_context * context, asm_program * prog)
{
    FILE * f = context->s_file;
    fprintf(f, "// Symbol Name          Page Address\n");
    fprintf(f, "// -------------------- ------------\n");

    bst_it it;
    it_begin(&prog->local.table, &it);
    while (!it_is_end(&it)) {
        bst_node * node;
        it_read(&it, &node);
        const char * sym;
        asm_symbol_val sym_val;
        bst_node_get_key(&prog->local.table, node, &sym);
        bst_node_get_val(&prog->local.table, node, &sym_val);
        fprintf(f, "// %-20s         %04X\n", sym, sym_val.addr);

        it_next(&it, 1);
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

    if (context->params.out_sym) {
        write_symbol_table(context, prog);
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

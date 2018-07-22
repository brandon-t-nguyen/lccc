#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <btn/vector.h>
#include <btn/container.h>

#include "asm.h"

static
void token_dtor(asm_token * tok)
{
    free(tok->str);
}

void asm_line_ctor(asm_line * line)
{
    line->number = 0;
    vector_ctor(&line->tokens, sizeof(asm_token), NULL, token_dtor);
    line->raw = NULL;
}

void asm_line_dtor(asm_line * line)
{
    vector_dtor(&line->tokens);
    if (line->raw) {
        free(line->raw);
    }
}

void asm_op_ctor(asm_op * op)
{
    vector_ctor(&op->operands, sizeof(asm_op), NULL, NULL);
    op->asop = OP_INVALID;
}

void asm_op_dtor(asm_op * op)
{
    vector_dtor(&op->operands);
}

void asm_section_ctor(asm_section * section)
{
    section->addr = 0;
    vector_ctor(&section->ops, sizeof(asm_op), NULL, NULL); // the enclosing program manages op memory
}

void asm_section_dtor(asm_section * section)
{
    vector_dtor(&section->ops);
}

void asm_source_ctor(asm_source * code)
{
    code->name = NULL;
    vector_ctor(&code->lines, sizeof(asm_line), NULL, asm_line_dtor);
}

void asm_source_dtor(asm_source * code)
{
    vector_dtor(&code->lines);
}

void asm_program_ctor(asm_program * prog)
{
    asm_source_ctor(&prog->src);
    prog->entry = 0x0000;

    vector_ctor(&prog->ops, sizeof(asm_op), NULL, asm_op_dtor);
    vector_ctor(&prog->sections, sizeof(asm_section), NULL, asm_section_dtor);

    asm_symbol_table_ctor(&prog->local);
    asm_symbol_table_ctor(&prog->global);
}

void asm_program_dtor(asm_program * prog)
{
    asm_source_dtor(&prog->src);
    vector_dtor(&prog->ops);
    vector_dtor(&prog->sections);

    asm_symbol_table_dtor(&prog->local);
    asm_symbol_table_dtor(&prog->global);
}

static
void fclose_shim(void * arg)
{
    FILE * f = *(FILE **) arg;
    fclose(f);
}

void asm_context_ctor(asm_context * context)
{
    vector_ctor(&context->file_paths, sizeof(const char *), NULL, NULL);
    vector_ctor(&context->files, sizeof(FILE *), NULL, fclose_shim);
    vector_ctor(&context->progs, sizeof(asm_program), NULL, asm_program_dtor);

    context->error_count = 0;
}

void asm_context_dtor(asm_context * context)
{
    vector_dtor(&context->file_paths);
    vector_dtor(&context->files);
    vector_dtor(&context->progs);
}

void asm_symbol_table_ctor(asm_symbol_table * table)
{
    bst_ctor(&table->table, sizeof(const char *), sizeof(asm_symbol_val),
             NULL, NULL, strcmp);
}

void asm_symbol_table_dtor(asm_symbol_table * table)
{
    bst_dtor(&table->table);
}

bool asm_symbol_table_put(asm_symbol_table * table, const char * sym, asm_symbol_val * sym_val)
{
    return bst_insert(&table->table, sym, sym_val);
}

bool asm_symbol_table_get(asm_symbol_table * table, const char * sym, asm_symbol_val * sym_val)
{
    return bst_find(&table->table, sym, sym_val);
}

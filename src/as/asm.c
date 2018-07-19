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

void asm_operand_dtor(asm_operand * operand)
{
    if (operand->type == OP_STR) {
        free(operand->data.str);
    }
}

void asm_op_ctor(asm_op * op)
{
    vector_ctor(&op->operands, sizeof(asm_op), NULL, asm_operand_dtor);
}

void asm_op_dtor(asm_op * op)
{
    vector_dtor(&op->operands);
}

void asm_section_ctor(asm_section * section)
{
    section->addr = 0;
    vector_ctor(&section->ops, sizeof(asm_op), NULL, asm_op_dtor);
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
    vector_ctor(&context->progs, sizeof(asm_program), NULL, NULL);
}

void asm_context_dtor(asm_context * context)
{
    vector_dtor(&context->file_paths);
    vector_dtor(&context->files);
    vector_dtor(&context->progs);
}

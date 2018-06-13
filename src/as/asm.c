#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <btn/vector.h>

#include "asm.h"

void asm_line_ctor(asm_line * line)
{
    line->number = 0;
    vector_ctor(&line->tokens, sizeof(char *), NULL, free);
}

void asm_line_dtor(asm_line * line)
{
    vector_dtor(&line->tokens);
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
#ifndef __ASM_H__
#define __ASM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

#include <btn/vector.h>
#include <btn/cstr.h>


// these structs help get the source files into
// a workable state
typedef struct _asm_line
{
    unsigned int number;
    vector(char *) tokens;
    char * raw;
} asm_line;

void asm_line_ctor(asm_line * line);
void asm_line_dtor(asm_line * line);

typedef struct _asm_source
{
    const char * name;
    vector(line) lines;
} asm_source;

void asm_source_ctor(asm_source * code);
void asm_source_dtor(asm_source * code);


#ifdef __cplusplus
}
#endif

#endif//__ASM_H__

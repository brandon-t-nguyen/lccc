#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <brandon/tok.h>
#include <brandon/vvector.h>
#include <brandon/cstr.h>

#include "as.h"


#define ARRAY_LEN(x) (sizeof(x)/sizeof(x[0]))

const AsmOpProp pattOps[] =
{
    {"add",AsmOp_add,AsmOpClass_Concrete},
    {"and",AsmOp_and,AsmOpClass_Concrete},
    {"not",AsmOp_not,AsmOpClass_Concrete},
    {"br",AsmOp_br,AsmOpClass_Concrete},{"brn",AsmOp_br,AsmOpClass_Concrete},
    {"brnz",AsmOp_br,AsmOpClass_Concrete},{"brnp",AsmOp_br,AsmOpClass_Concrete},
    {"brnzp",AsmOp_br,AsmOpClass_Concrete},{"brz",AsmOp_br,AsmOpClass_Concrete},
    {"brzp",AsmOp_br,AsmOpClass_Concrete},{"brp",AsmOp_br,AsmOpClass_Concrete},
    {"nop",AsmOp_nop,AsmOpClass_Concrete},
    {"jmp",AsmOp_jmp,AsmOpClass_Concrete},
    {"jsr",AsmOp_jsr,AsmOpClass_Concrete},
    {"jsrr",AsmOp_jsrr,AsmOpClass_Concrete},
    {"ret",AsmOp_ret,AsmOpClass_Concrete},
    {"ld",AsmOp_ld,AsmOpClass_Concrete},
    {"ldr",AsmOp_ldr,AsmOpClass_Concrete},
    {"ldi",AsmOp_ldi,AsmOpClass_Concrete},
    {"st",AsmOp_st,AsmOpClass_Concrete},
    {"str",AsmOp_str,AsmOpClass_Concrete},
    {"sti",AsmOp_sti,AsmOpClass_Concrete},

    {".orig",AsmOp_orig,AsmOpClass_Meta},
    {".blkw",AsmOp_blkw,AsmOpClass_Concrete},
    {".fill",AsmOp_fill,AsmOpClass_Concrete},
    {".stringz",AsmOp_stringz,AsmOpClass_Concrete}
};

// backward compatible
const AsmOpProp lcccOps[] =
{
    {"mov",AsmOp_mov,AsmOpClass_Concrete},
    {"or",AsmOp_or,AsmOpClass_Concrete},
    {"neg",AsmOp_neg,AsmOpClass_Concrete},
    {"push",AsmOp_push,AsmOpClass_Concrete},
    {"pop",AsmOp_pop,AsmOpClass_Concrete},

    {".string",AsmOp_stringz,AsmOpClass_Concrete},
    {".stringp",AsmOp_stringp,AsmOpClass_Concrete},
    {".define",AsmOp_define,AsmOpClass_Meta},
    {".section",AsmOp_section,AsmOpClass_Meta},
    {".export",AsmOp_export,AsmOpClass_Meta},
    {".import",AsmOp_import,AsmOpClass_Meta}
};

AsmOp parseOpcode( const char * str, AsmSettings * settings )
{
    for (int i = 0; i < ARRAY_LEN(pattOps); ++i)
    {
        if (!strcmp_caseless( pattOps[i].match, str ))
        {
            return pattOps[i].op;
        }
    }

    if (settings->syntax == AsmSyntax_patt)
    {
        return AsmOp_invalid;
    }

    for (int i = 0; i < ARRAY_LEN(lcccOps); ++i)
    {
        if (!strcmp_caseless( lcccOps[i].match, str ))
        {
            return lcccOps[i].op;
        }
    }

    return AsmOp_invalid;
}

void processSource( Source * src, AsmSettings * set )
{
    while (Tokenizer_hasTokens( src->tok ))
    {
        const char * token = Tokenizer_next( src->tok );
        if (!strcmp(token,"\n"))
        {
            token = "\\n";
        }
        printf("Token: %s\n",token);
    }
}

char testProg[] = "; Brandon Nguyen\n"
                  "; lorem ipsum\n"
                  "    .section .text\n"
                  "\n"
                  "hello .stringz \"Hello world!\\n\";\n"
                  "main\n"
                  "    add r0, r0, r1;bullshit lmao\n"
                  "    push {r1,r2,r3}\n"
                  ;

int main( int argc, char * argv[] )
{
    // Driver: get a string containing the entire source
    Source src;
    Source_ctor( &src, testProg, "testProg" );

    AsmSettings set = {.syntax = AsmSyntax_patt};

    processSource( &src, &set );

    return 0;
}

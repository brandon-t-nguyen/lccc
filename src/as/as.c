#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <brandon/tok.h>
#include <brandon/vvector.h>
#include <brandon/vbst.h>
#include <brandon/cstr.h>

#include "as.h"
#include "symbol.h"
#include "program.h"


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

const AsmOpProp InvalidOpProp = {"INVALID",AsmOp_invalid,AsmOpClass_Meta};

const AsmOpProp * parseOpcode( const char * str, AsmSettings * settings )
{
    for (int i = 0; i < ARRAY_LEN(pattOps); ++i)
    {
        if (!strcmp_caseless( pattOps[i].match, str ))
        {
            return &pattOps[i];
        }
    }

    if (settings->syntax == AsmSyntax_patt)
    {
        return &InvalidOpProp;
    }

    for (int i = 0; i < ARRAY_LEN(lcccOps); ++i)
    {
        if (!strcmp_caseless( lcccOps[i].match, str ))
        {
            return &lcccOps[i];
        }
    }

    return &InvalidOpProp;
}

typedef enum ProcessState_enum
{
    ProcessState_Init,
    ProcessState_FindOperands,
    ProcessState_EndOperands,
    ProcessState_FoundSymbol
} ProcessState;

Code * Code_new( void )
{
    Code * thiz = (Code *)malloc( sizeof(Code) );
    thiz->offset = -1;
    thiz->length = -1;
    thiz->opProp = NULL;
    return thiz;
}

Program * processSource( Source * src, AsmSettings * set )
{
    Program * prog = Program_new();

    const char * token = NULL;
    ProcessState state = ProcessState_Init;
    const char * symbolHold = NULL; // symbol to hold until concrete op found
    AsmOpProp * prop = NULL;
    int lineNum = 0;    // index 0
    do
    {
        // perform state logic
        switch (state)
        {
        case ProcessState_Init:
            break;
        case ProcessState_FindOperands:
            break;
        case ProcessState_EndOperands:
            break;
        case ProcessState_FoundSymbol:
            break;
        }

        // perform transitions
        token = Tokenizer_next( src->tok );
        if ( *token == '\n' )
        {
            ++lineNum;
        }
    } while (Tokenizer_hasTokens( src->tok ));

    return prog;
}
        /*
        if (!strcmp(token,"\n"))
        {
            token = "\\n";
        }
        printf("Token: %s\n",token);
        */


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
    ///*
    Source src;
    Source_ctor( &src, testProg, "testProg" );

    AsmSettings set = {.syntax = AsmSyntax_lccc};

    Program * prog = processSource( &src, &set );

    Source_dtor( &src );
    //*/

    /*
    IVmap * bst = Vbst_IVmap_new_reg( strcmp, free, free );

    IVmap_insert( bst, strdup("bat"), strdup("man") );
    IVmap_insert( bst, strdup("ant"), strdup("farm"));
    IVmap_insert( bst, strdup("cat"), strdup("in the hat"));
    IVmap_insert( bst, strdup("battle"), strdup("of helm's deep") );
    IVmap_insert( bst, strdup("1337"), strdup("hax0rs") );

    const char * str;
    IVmap_find( bst, "bat", &str);
    printf("%s: %s\n","bat",str);
    IVmap_find( bst, "ant", &str);
    printf("%s: %s\n","ant",str);
    IVmap_find( bst, "cat", &str);
    printf("%s: %s\n","cat",str);
    IVmap_find( bst, "battle", &str);
    printf("%s: %s\n","battle",str);
    IVmap_find( bst, "1337", &str);
    printf("%s: %s\n","1337",str);

    IVmap_delete( bst );
    */
    /*
    Vbst * bst = Vbst_new_reg( strcmp, free, free );

    Vbst_insert( bst, strdup("bat"), strdup("man") );
    Vbst_insert( bst, strdup("ant"), strdup("farm"));
    Vbst_insert( bst, strdup("cat"), strdup("in the hat"));
    Vbst_insert( bst, strdup("battle"), strdup("of helm's deep") );
    Vbst_insert( bst, strdup("1337"), strdup("hax0rs") );

    const char * str;
    Vbst_find( bst, "bat", &str);
    printf("%s: %s\n","bat",str);
    Vbst_find( bst, "ant", &str);
    printf("%s: %s\n","ant",str);
    Vbst_find( bst, "cat", &str);
    printf("%s: %s\n","cat",str);
    Vbst_find( bst, "battle", &str);
    printf("%s: %s\n","battle",str);
    Vbst_find( bst, "1337", &str);
    printf("%s: %s\n","1337",str);

    Vbst_delete( bst );
    */

    return 0;
}

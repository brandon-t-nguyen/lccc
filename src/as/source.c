#include <stdlib.h>
#include <string.h>

#include <brandon/log.h>
#include <brandon/tok.h>
#include <brandon/vvector.h>
#include <brandon/cstr.h>

#include "as.h"
// Preprocessor for the assembler

typedef enum fullTokenizeState_enum
{
    fts_find,
    fts_token,
    fts_quote,
    fts_comment
} fts;

// strips whitespace, tokenizes but keeps newlines, commas
static
void fullTokenize( const char * str, int * pNum, char *** pArray, void * param )
{
    VVector * vec = VVector_new(8);
    int len = strlen(str);

    fts state = fts_find;
    int pos = -1;
    int end = -1;
    for (int i = 0; i < len; ++i)
    {
        char c = str[i];
        switch (state)
        {
        case fts_find:
            if (c == ';')
            {
                state = fts_comment;
            }
            // included delimiters
            else if (c == '\n' || c == ',')
            {
                VVector_push( vec, strchar(c) );
            }
            // quote
            else if (c == '"')
            {
                state = fts_quote;
                pos = i;
            }
            // found token
            else if (c != ' ' && c != '\t' && c != '\0')
            {
                state = fts_token;
                pos = i;
            }
            break;
        case fts_token:
            if (c == ' ' || c == '\t' || c == '\0')
            {
                state = fts_find;
                end = i-1;
                VVector_push( vec, stricpy( str, pos, end ) );
            }
            else if (c == '\n' || c == ',')
            {
                state = fts_find;
                end = i-1;
                VVector_push( vec, stricpy( str, pos, end ) );
                VVector_push( vec, strchar(c) );
            }
            else if (c == ';')
            {
                state = fts_comment;
                end = i-1;
                VVector_push( vec, stricpy( str, pos, end ) );
            }
            break;
        case fts_quote:
            if (c == '"')
            {
                state = fts_find;
                end = i;
                VVector_push( vec, stricpy( str, pos, end ) );
            }
            break;
        case fts_comment:
            if (c == '\n')
            {
                state = fts_find;
                VVector_push( vec, strchar(c) );
            }
            break;
        }
    }

    *pNum = VVector_length(vec);
    *pArray = VVector_toArray_cpy(vec);

    VVector_delete(vec);
}

void Source_ctor( Source * thiz, const char * src, const char * name )
{
    Tokenizer * tok = Tokenizer_new( src, "\n" );

    thiz->name = strdup(name);
    thiz->raw = strdup(src);
    thiz->numLines = Tokenizer_countTokens(tok);
    thiz->lines = Tokenizer_tokens_cpy(tok);

    thiz->tok = Tokenizer_new_custom( src, fullTokenize, NULL );
}

void Source_dtor( Source * thiz )
{
    free(thiz->name);
    free(thiz->raw);
    for (int i = thiz->numLines-1; i >= 0; --i)
    {
        free(thiz->lines[i]);
    }
    free(thiz->lines);
}

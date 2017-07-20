#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <brandon/tok.h>
#include <brandon/vvector.h>

char testProg[] = "; Brandon Nguyen\n"
                  "; Go fuck yourself\n"
                  "    .section .text\n"
                  "\n"
                  "main\n"
                  "    add r0, r0, r1;bullshit lmao\n"
                  ;

static VVector * processSource( const char * src );
int main( int argc, char * argv[] )
{
    VVector * lines = processSource( testProg );

    int len = VVector_length( lines );
    for (int i = 0; i < len; ++i)
    {
        printf( "%s\n", (const char *)VVector_get( lines, i ) );
    }

    VVector_delete( lines );
    return 0;
}

// turn raw source code into a vector of lines with comments filtered out
VVector * processSource( const char * src )
{
    // split into lines
    Tokenizer * tok = Tokenizer_new( src, "\n" );
    int numTokens = Tokenizer_countTokens( tok );
    const char * const * rawLines = Tokenizer_tokens( tok );

    // heuristic: 1/4th is empty meaningless lines
    VVector * output = VVector_new( numTokens*3/4 );
    VVector_registerDelete( output, free ); // use the standard free function

    for (int i = 0; i < numTokens; ++i)
    {
        const char * line = rawLines[i];
        int len = strlen( line );

        // scan for non-whitespace; end at comment
        int pos = 0;
        char c;
        int hasContent = 0;
        for (pos = 0; pos < len; ++pos)
        {
            char c = line[pos];
            if (c == ';')
            {
                break;
            }

            // if a non-space or non-tab is found, content is found
            if ( !hasContent && c != ' ' && c != '\t')
            {
                hasContent = 1;
            }
        }

        // if there is meaningful content, copy it
        if (hasContent)
        {
            int contentLen = pos;
            char * content =  (char *) malloc( contentLen+1 );
            strncpy( content, line, contentLen );
            content[contentLen] = '\0'; // add the null term

            // add to our lines
            VVector_push( output, content );
        }
    }

    Tokenizer_delete( tok );
    return output;
}

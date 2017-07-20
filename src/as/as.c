#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <brandon/tok.h>
#include <brandon/vvector.h>

#include "as.h"

char testProg[] = "; Brandon Nguyen\n"
                  "; Go fuck yourself\n"
                  "    .section .text\n"
                  "\n"
                  "main\n"
                  "    add r0, r0, r1;bullshit lmao\n"
                  ;

extern VVector * processSource( const char * src );
int main( int argc, char * argv[] )
{
    VVector * lines = processSource( testProg );

    int len = VVector_length( lines );
    for (int i = 0; i < len; ++i)
    {
        Line * line = (Line *)VVector_get( lines, i );
        printf( "%4d %s\n", line->number, line->code );
    }

    VVector_delete( lines );
    return 0;
}

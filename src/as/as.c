#include <stdio.h>

#include <brandon/tok.h>

int main( int argc, char * argv[] )
{
    // test tokenizer
    const char * str = "hello world this is a tokenized string";

    Tokenizer * tok = Tokenizer_new( str, " " );
    while( Tokenizer_hasTokens( tok ) )
    {
        printf("%s\n", Tokenizer_next( tok ) );
    }

    return 0;
}

#include <string.h>
#include <stdlib.h>

#include <brandon/cstr.h>

// end is the last char
char * stricpy( const char * str, size_t start, size_t end )
{
    int len = end - start + 1;
    char * out = (char *)malloc( sizeof(char) * (len + 1) );
    strncpy( out, str+start, len );
    out[len] = '\0';
    //dprintf( "%s: [%d:%d] %s\n", __func__, start, end, out );
    return out;
}

char * strchar( char c )
{
    char * out = (char *)malloc( sizeof(char) * 2 );
    out[0] = c;
    out[1] = '\0';
    //dprintf( "%s: 0x%02X\n", __func__, c );
    return out;
}

#define IS_UPPER(c) ('A' <= c && c <= 'Z')
#define TO_LOWER(c) (c + ('a' - 'A'))
int strcmp_caseless( const char * s1, const char * s2 )
{
    char c1 = *s1; char c2 = *s2;
    while (c1)
    {

        // case matching
        if (IS_UPPER(c1))
            c1 = TO_LOWER(c1);
        if (IS_UPPER(c2))
            c2 = TO_LOWER(c2);

        int comp = (c1-c2);
        if (comp)
            return comp;

        ++s1; ++s2;
        c1 = *s1; c2 = *s2;
    }
    return c1-c2;
}

#ifndef __BRANDON_CSTR_H__
#define __BRANDON_CSTR_H__
// C string processing code
#ifdef __cplusplus
extern "C" {
#endif
#include <string.h>

/**
 * Creates a string given the position of the first and last character
 * in a source string
 * @param[in]   str     The source string
 * @param[in]   start   The index of the starting character
 * @param[in]   end     The index of the ending character
 * @return      The requested string. Must be free()'ed later
 */
char * stricpy( const char * str, size_t start, size_t end );

/**
 * Creates a string given a single character
 * @param[in]   c   The character
 * @return      The requested string. Must be free()'ed later
 */
char * strchar( char c );

/**
 * Performs a caseless strcmp
 * @return <0 if s1 < s2, 0 if s1 == s2, >0 if s1 > s2 in lexographic ordering
 */
int strcmp_caseless( const char * s1, const char * s2 );

#ifdef __cplusplus
}
#endif

#endif//__BRANDON_CSTR_H__

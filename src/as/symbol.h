#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "as.h"

struct Symbol_str;
typedef struct Symbol_str Symbol;

/**
 * Creates a new symbol
 * @param[in]   name    The name of the symbol
 * @param[in]   code    The associated code. Use NULL if it's an external
 */
Symbol * Symbol_new( const char * name, Code * code );

/**
 * Symbols use a ref counting system
 * You can ref and deref them. Macro provided
 * to "delete" them in a sense
 */
Symbol * Symbol_ref( Symbol * thiz );
void Symbol_deref( Symbol * thiz );
#define Symbol_delete(thiz) Symbol_deref(thiz)

/**
 * Returns the name
 */
const char * Symbol_name( Symbol * thiz );

/**
 * Returns the associated code
 */
Code * Symbol_code( Symbol * thiz );

/**
 * Returns if the symbol is an imported symbol
 */
bool Symbol_imported( Symbol * thiz );

struct SymbolTable_str;
typedef struct SymbolTable_str SymbolTable;

/**
 * Creates a new symbol table
 */
SymbolTable * SymbolTable_new( void );
void SymbolTable_delete( SymbolTable * thiz );

/**
 * Adds a symbol to the table. Will increase ref count
 * @return 0 if failure (already exists or NULL symbol), 1 if success
 */
int SymbolTable_add( SymbolTable * thiz, Symbol * sym );

/**
 * Retrieves a symbol given its name. Will not increase ref count
 * @return  Symbol pointer if found, NULL if not found
 */
Symbol * SymbolTable_retrieve( SymbolTable * thiz, const char * name );

#ifdef __cplusplus
}
#endif

#endif//__SYMBOL_H__

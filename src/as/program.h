#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "as.h"
#include "symbol.h"

typedef struct Program_str
{
    SymbolTable * localSymbols; // symbols declared in this unit
    SymbolTable * importSymbols;// symbols that are external
    SymbolTable * exportSymbols;// symbols to expose
    SymbolTable * allSymbols;   // all symbols: check for collisions
    IVmap(char*,char*) * defines;

    VVector(Code*) * codes;
} Program;
Program * Program_new( void );
void Program_delete( Program * thiz );

/**
 * @return 1 if success, 0 if failure (local symbol already present)
 */
int Program_newLocalSymbol( Program * thiz, const char * name, Code * code );

/**
 * @return 1 if success, 0 if failure (symbol already imported)
 */
int Program_newImportSymbol( Program * thiz, const char * name );

/**
 * @return 1 if success, 0 if failure (no such local symbol)
 */
int Program_exportSymbol( Program * thiz, const char * name );

/**
 * @return The symbol mapped to this name.
 */
Symbol * Program_getSymbol( Program * thiz, const char * name );

#ifdef __cplusplus
}
#endif

#endif//__PROGRAM_H__

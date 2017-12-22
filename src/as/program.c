#include <stdlib.h>
#include <string.h>

#include <btn/ds/ivmap.h>
#include <btn/ds/vbst.h>

#include "as.h"
#include "symbol.h"
#include "program.h"

void Program_ctor( Program * thiz )
{
    thiz->localSymbols  = SymbolTable_new();
    thiz->importSymbols = SymbolTable_new();
    thiz->exportSymbols = SymbolTable_new();
    thiz->allSymbols    = SymbolTable_new();
    thiz->defines       = Vbst_IVmap_new_reg( strcmp, free, free );
}
void Program_dtor( Program * thiz )
{
    SymbolTable_delete( thiz->localSymbols );
    SymbolTable_delete( thiz->importSymbols );
    SymbolTable_delete( thiz->exportSymbols );
    SymbolTable_delete( thiz->allSymbols );
    IVmap_delete( thiz->defines );
}

void Program_delete( Program * thiz )
{
    Program_dtor( thiz );
    free( thiz );
}

Program * Program_new( void )
{
    Program * thiz = (Program *)malloc( sizeof(Program) );
    Program_ctor( thiz );
    return thiz;
}


int Program_newLocalSymbol( Program * thiz, const char * name, Code * code )
{
    Symbol * sym = SymbolTable_retrieve( thiz->localSymbols, name );
    if (sym == NULL)
    {
        return 1;
    }

    sym = Symbol_new( name, code );
    SymbolTable_add( thiz->localSymbols, sym );
    SymbolTable_add( thiz->allSymbols, sym );

    return 0;
}

int Program_newImportSymbol( Program * thiz, const char * name )
{
    Symbol * sym = SymbolTable_retrieve( thiz->importSymbols, name );
    if (sym == NULL)
    {
        return 1;
    }

    sym = Symbol_new( name, NULL );
    SymbolTable_add( thiz->importSymbols, sym );
    SymbolTable_add( thiz->allSymbols, sym );

    return 0;
}

int Program_exportSymbol( Program * thiz, const char * name )
{
    Symbol * toExport = SymbolTable_retrieve( thiz->localSymbols, name );
    if (toExport == NULL)
    {
        return 1;
    }

    SymbolTable_add( thiz->exportSymbols, toExport );

    return 0;
}

Symbol * Program_getSymbol( Program * thiz, const char * name )
{
    Symbol * sym = SymbolTable_retrieve( thiz, name );
    return sym;
}

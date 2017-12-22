#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <btn/ds/ivmap.h>
#include <btn/ds/vbst.h>
#include <btn/ds/vvector.h>

#include "as.h"
#include "symbol.h"

struct Symbol_str
{
    char * name;
    Code * code;    // the code this is associated with
    int ref;        // reference count
};

Symbol * Symbol_new( const char * name, Code * code )
{
    Symbol * thiz = (Symbol *)malloc( sizeof(Symbol) );
    thiz->name   = strdup( name );
    thiz->code  = code;
    thiz->ref    = 1;
    return thiz;
}

Symbol * Symbol_ref( Symbol * thiz )
{
    thiz->ref += 1;
    return thiz;
}

void Symbol_deref( Symbol * thiz )
{
    thiz->ref -= 1;
    if (thiz->ref == 0)
    {
        // destroy it
        free( thiz->name );
    }
}

const char * Symbol_name( Symbol * thiz ) { return thiz->name; }
Code * Symbol_code( Symbol * thiz ) { return thiz->code; }
bool Symbol_imported( Symbol * thiz ) { return (thiz->code == NULL); }

struct SymbolTable_str
{
    IVmap * map;
};

SymbolTable * SymbolTable_new( void )
{
    SymbolTable * thiz = (SymbolTable *)malloc( sizeof(SymbolTable) );

    // Use Symbol's name as key so no deleting it, deref the symbol to remove
    thiz->map = Vbst_IVmap_new_reg( strcmp, NULL, Symbol_deref );

    return thiz;
}

void SymbolTable_delete( SymbolTable * thiz )
{
    IVmap_delete( thiz->map );
    free( thiz );
}

int SymbolTable_add( SymbolTable * thiz, Symbol * sym )
{
    // insert the symbol: refcount++
    if (sym)
    {
        return IVmap_insert( thiz->map, sym->name, Symbol_ref(sym) );
    }
    return 0;
}

Symbol * SymbolTable_retrieve( SymbolTable * thiz, const char * name )
{
    Symbol * sym;
    int retVal = IVmap_find( thiz->map, name, &sym );
    if (retVal == 0)
    {
        return NULL;
    }
    return sym;
}

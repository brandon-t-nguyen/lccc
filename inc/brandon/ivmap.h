#ifndef __BRANDON_IVMAP_H__
#define __BRANDON_IVMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#define IVmap(key,value) IVmap

typedef struct IVmap_ops_str
{
    int (* insert)( void * thiz, void * key, void * val );
    int (* assign)( void * thiz, void * key, void * val );
    int (* find)( void * thiz, void * key, void ** pVal );
    int (* remove)( void * thiz, void * key, void ** pVal );
    void (* delete)( void * thiz );
} IVmap_ops;

typedef struct IVmap_str
{
    void * base;            // base object
    const IVmap_ops * ops;  // virtual table
} IVmap;

static inline
int IVmap_insert( IVmap * ivmap, void * key, void * val )
{
    return ivmap->ops->insert( ivmap->base, key, val );
}

static inline
int IVmap_assign( IVmap * ivmap, void * key, void * val )
{
    return ivmap->ops->assign( ivmap->base, key, val );
}

static inline
int IVmap_find( IVmap * ivmap, void * key, void ** pVal )
{
    return ivmap->ops->find( ivmap->base, key, pVal );
}

static inline
int IVmap_remove( IVmap * ivmap, void * key, void ** pVal )
{
    return ivmap->ops->remove( ivmap->base, key, pVal );
}

static inline
void IVmap_delete( IVmap * ivmap )
{
    ivmap->ops->delete( ivmap );
}

#ifdef __cplusplus
}
#endif

#endif//__BRANDON_IVMAP_H__

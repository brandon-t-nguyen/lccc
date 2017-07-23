#ifndef __BRANDON_VBST_H__
#define __BRANDON_VBST_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <brandon/ivmap.h>

// The Vbst (void BST) is a Map-able data structure

#define Vbst(key,value) Vbst
struct vbst_str;
typedef struct vbst_str Vbst;

/**
 * Creates a new void BST
 * @param[in] comp      Comparator function
 * @param[in] delKey    Deletor function for key
 * @param[in] delVal    Deletor function for value
 */
Vbst * Vbst_new( int (* comp)(void *, void *) );
Vbst * Vbst_new_reg( int (* comp)(void *, void *),
                     void (* delKey)(void *),
                     void (* delVal)(void *) );
void Vbst_delete( Vbst * thiz );

/**
 * IVmap interface new and delete
 */
IVmap * Vbst_IVmap_new( int (* comp)(void *, void *) );
IVmap * Vbst_IVmap_new_reg( int (* comp)(void *, void *),
                            void (* delKey)(void *),
                            void (* delVal)(void *) );

/**
 * Registers deletor functions
 */
void Vbst_registerDelete( Vbst * thiz,
                          void (* delKey)(void *),
                          void (* delVal)(void *) );

/**
 * Inserts item to tree
 * Does not allow duplicates
 * @return 1 if success, 0 if failure
 */
int Vbst_insert( Vbst * thiz, void * key, void * val );

/**
 * Finds an element via a key
 * @param[in] key   The key to use
 * @ret       0 if not found, 1 if found
 */
int Vbst_find( Vbst * thiz, void * key, void ** pVal );

/**
 * Finds an element via a key and removes it
 * @param[in] key   The key to use
 * @ret       0 if not found, 1 if found
 */
int Vbst_remove( Vbst * thiz, void * key, void ** pVal );
#ifdef __cplusplus
}
#endif

#endif//__BRANDON_VBST_H__

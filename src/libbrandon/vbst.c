#include <stdlib.h>

#include <brandon/vbst.h>
#include <brandon/ivmap.h>


static
void Vbst_IVmap_delete( IVmap * ivmap );

static const IVmap_ops mapOps =
{
    .insert = Vbst_insert,
    .assign = Vbst_insert,
    .find   = Vbst_find,
    .remove = Vbst_remove,
    .delete = Vbst_IVmap_delete
};

IVmap * Vbst_IVmap_new( int (* comp)(void *, void *) )
{
    IVmap * ivmap = (IVmap *)malloc( sizeof(IVmap) );
    ivmap->ops = &mapOps;
    ivmap->base = Vbst_new( comp );
    return ivmap;
}

IVmap * Vbst_IVmap_new_reg( int (* comp)(void *, void *), void (* del)(void *) )
{
    IVmap * ivmap = (IVmap *)malloc( sizeof(IVmap) );
    ivmap->ops = &mapOps;
    ivmap->base = Vbst_new_reg( comp, del );
    return ivmap;
}

void Vbst_IVmap_delete( IVmap * ivmap )
{
    Vbst_delete( ivmap->base );
    free( ivmap );
}

struct Node_str;
typedef struct Node_str Node;
struct Node_str
{
    void * key;
    void * val;
    Node * l;
    Node * r;
};

struct vbst_str
{
    int (* comp)(void *, void *);
    void (* del)(void *);
    Node * root;
};

static
Node * Node_new( void * key, void * val )
{
    Node * node = (Node *)malloc(sizeof(Node));

    node->key = key;
    node->val = val;
    node->l = NULL;
    node->r = NULL;

    return node;
}

static
void Node_delete( Node * thiz, void (* del)(void *) )
{
    if (thiz)
    {
        if (del)
        {
            del( thiz->val );
        }
        Node_delete( thiz->l, del );
        Node_delete( thiz->r, del );
        free( thiz );
    }
}

static
void  Vbst_ctor( Vbst * thiz, int (* comp)(void *, void *) )
{
    thiz->comp = comp;
    thiz->del = NULL;
    thiz->root = NULL;
}

static
void Vbst_dtor( Vbst * thiz )
{
    Node_delete( thiz->root, thiz->del );
}

/**
 * Creates a new void BST
 * @param[in] comp  Comparator function
 * @param[in] del   Deletor function
 */
Vbst * Vbst_new( int (* comp)(void *, void *) )
{
    Vbst * vbst = (Vbst *)malloc( sizeof(Vbst) );
    Vbst_ctor( vbst, comp );
    return vbst;
}

Vbst * Vbst_new_reg( int (* comp)(void *, void *), void (* del)(void *) )
{
    Vbst * vbst = Vbst_new( comp );
    Vbst_registerDelete( vbst, del );
    return vbst;
}

void Vbst_delete( Vbst * thiz )
{
    Vbst_dtor( thiz );
    free( thiz );
}

/**
 * Registers a deletor function
 */
void Vbst_registerDelete( Vbst * thiz, void (* del)(void *) )
{
    thiz->del = del;
}

int Vbst_insert( Vbst * thiz, void * key, void * val )
{
    if (thiz->root)
    {
        Node * curr = thiz->root;
        Node * place = NULL;    // node to place
        while (!place)
        {
            int comparison = thiz->comp( key, curr->key );
            if (comparison < 0)
            {
                if (curr->l)
                {
                    curr = curr->l;
                }
                else
                {
                    place = Node_new( key, val );
                    curr->l = place;
                }
            }
            else if (comparison > 0)
            {
                if (curr->r)
                {
                    curr = curr->r;
                }
                else
                {
                    place = Node_new( key, val );
                    curr->r = place;
                }
            }
            else
            {
                return -1;
            }

        }

    }
    else
    {
        thiz->root = Node_new( key, val );
    }
    return 0;
}

int Vbst_find( Vbst * thiz, void * key, void ** pVal )
{
    Node * curr = thiz->root;

    while (curr)
    {
        int comparison = thiz->comp( key, curr->key );
        if (comparison == 0)
        {
            *pVal = curr->val;
            return 1;
        }
        curr = (comparison < 0)? curr->l : curr->r;
    }

    *pVal = NULL;
    return 0;
}


int Vbst_remove( Vbst * thiz, void * key, void ** pVal )
{
}

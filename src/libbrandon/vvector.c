#include <stdlib.h>

#include <brandon/vvector.h>

struct vvector_str
{
    int length;     // length of elements
    int size;       // # of total spots
    void** array;   // array of void ptrs
    void (*del)(void *);    // function to call deletors
};

#define MIN_SIZE 8

VVector* VVector_new(int length)
{
    if (length < MIN_SIZE)
    {
        length = MIN_SIZE;
    }

    VVector* vec = malloc(sizeof(VVector));         // Allocate vvector
    vec->length = 0;
    vec->size = length;
    vec->array = malloc(sizeof(void*) * length);    // Allocate array space
    for(int i = 0; i < length; i++)
    {
        vec->array[i] = 0x0;    // Null init
    }
    vec->del = free;
    return vec;
}

VVector* VVector_new_reg(int length, void (*func)(void *))
{
    VVector* vec = VVector_new(length);
    VVector_registerDelete(vec, func);
    return vec;
}

void VVector_registerDelete( VVector *thiz, void (*func)(void *))
{
    thiz->del = func;
}

void VVector_delete(VVector* thiz)
{
    VVector_deleteLite(thiz);
}

void VVector_deleteLite(VVector* thiz)
{
    free(thiz->array);   //free the array
    free(thiz);          //free the pointer
}

void VVector_deleteFull(VVector* thiz)
{
    int length = thiz->length;
    for(int i = 0; i < length; i++)
    {
        if(thiz->array[i] != 0)
            thiz->del(thiz->array[i]);
    }
    VVector_deleteLite(thiz);
}

//Will double size
void VVector_realloc(VVector* thiz, int size)
{
    //Break if we're making it even smaller
    if(size <= thiz->length)
        return;
    void** oldArr = thiz->array;
    int oldSize = thiz->size;
    int newSize = size;

    void** newArr = malloc(sizeof(void*) * newSize);   // alloc new memory
    int i = 0;
    for(; i < oldSize; i++)
    {
        newArr[i] = oldArr[i];  //set to old void*
    }
    for(; i < newSize; i++)
    {
        newArr[i] = 0x0;    //null init
    }

    thiz->array = newArr;    //Set the new array
    thiz->size = newSize;    //Set new size
    free(oldArr); //Free the old array
}

void VVector_push(VVector* thiz, void* ptr)
{
    if(thiz->length == thiz->size)
    {
        VVector_realloc(thiz,thiz->size*2);    //amortize doubling
    }
    thiz->array[thiz->length] = ptr;  //set the value
    thiz->length++;                  //increment the length
}

void* VVector_pop(VVector* thiz)
{
    if(thiz->length > 0)
    {
        thiz->length -= 1;
        return thiz->array[thiz->length]; //get the last value; lol already decremented for thiz
    }
    return 0x0;
}

void* VVector_get(VVector* thiz, int index)
{
    if(index < thiz->length)
        return thiz->array[index];
    return 0x0;
}

int VVector_find(VVector* thiz, void * thing)
{
    int length = thiz->length;
    void **arr = thiz->array;
    for( int i = 0; i < length; i++ )
    {
        if(thing == arr[i])
        {
            return i;
        }
    }
    return -1;
}

void VVector_removeAt(VVector* thiz, int index )
{
    int length = thiz->length;
    if( index < 0 || length <= index || length < 1 )
        return;

    void **arr = thiz->array;
    for( int i = index; i < length-1; i++ )
    {
        arr[i] = arr[i+1];
    }
    arr[length-1] = NULL;
}

void VVector_remove(VVector* thiz, void * thing)
{
    int loc = VVector_find(thiz, thing);
    VVector_removeAt(thiz, loc);
}

const void * const * VVector_toArray(VVector* thiz)
{
    return thiz->array;
}

void** VVector_toArray_cpy(VVector* thiz)
{
    int length = thiz->length;
    void** output = malloc(sizeof(void*) * length);
    void** array = thiz->array;
    for(int i = 0; i < length; i++)
    {
        output[i] = array[i];
    }
    return output;
}


int VVector_length(VVector* thiz)
{
    return thiz->length;
}

int VVector_capacity(VVector* thiz)
{
    return thiz->size;
}

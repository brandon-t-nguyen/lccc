#ifndef __VVECTOR_H__
#define __VVECTOR_H__

//VVector: void vector

struct vvector_str;
typedef struct vvector_str VVector;

VVector* VVector_new(int length);            // null initializes a vector of length
VVector* VVector_new_reg(int length, void (*func)(void *));   // null initializes a vector of length, registers a delete func
void VVector_registerDelete(VVector *this, void (*func)(void *)); // register a delete function
void VVector_delete(VVector* vector);        // uses lite
void VVector_deleteLite(VVector* vector);    // free vector structure but not internal contents
void VVector_deleteFull(VVector* vector);    // free vector and all internal contents

void VVector_push(VVector* vector, void* ptr);   // Adds value to end
void* VVector_pop(VVector* vector);              // Pops off value off end

void* VVector_get(VVector* vector, int index);   // Get value from index
int VVector_find(VVector* vector, void * thing );   // Find index from pointer
void VVector_removeAt( VVector* vector, int thing );   // remove at an index
void VVector_remove( VVector* vector, void * thing );   // remove a thing

void VVector_realloc(VVector* vector, int newSize); //Reallocates the vector to a new size. Will only make it bigger.

const void * const * VVector_toArray(VVector* vector);        //Returns an array with the values;
void** VVector_toArray_cpy(VVector* vector);    //Returns an array with the values, shallow copied;

int VVector_length(VVector* vector);    //reports length of the vector
int VVector_length(VVector* vector);    //reports size of the vector

#endif  //__VVECTOR_H__


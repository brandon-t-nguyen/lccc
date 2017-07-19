# Little Computer Compiler Collection
This is for a set of compiling tools for Yale Patt's Little Computer architectures

## Output binaries
Support binaries are:
* .obj : the classic object code file, with a .sym symbol table produced separately
* .lo : "little object", a non-executable, unlinked piece of object code containing an internal symbol table and supporting data and text sections. Symbols can be unresolved and handled at link time
* .lexe : "little executable" a .lo that has been linked. Unresolved symbols can be resolved at process load time (or handled a simulator that supports run-time symbol resolution)

## Type sizes

### LC-3
char        -> 2 bytes
short       -> 2 bytes
int         -> 2 bytes
long        -> 4 bytes
long long   -> 8 bytes

### LC-3b
char        -> 1 byte
short       -> 2 bytes
int         -> 2 bytes
long        -> 4 bytes
long long   -> 8 bytes

## lccc ABI
The calling convention is as follows:

### Register saving
There are no caller saved (i.e. scratch) registers: all used registers must be saved and restored by the caller.
R5 is a frame pointer

### Stack pointer
R6 is the stack pointer

### Parameter passing
There are two different methods of passing parameters:

#### Standard: register and stack arguments
R0 -> R3 are for the first 4 function inputs
The rest of the arguments are pushed onto the stack from right to left.
1 byte parameters will consume a register and memory location.
If a parameter passes the 2-byte boundary, then more words will be given to it.

#### Stack-only arguments:
This will be triggered by functions with variable length argument lists or when
an argument that is longer than 8-bytes is passed

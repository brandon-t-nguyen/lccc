# .obj format
by Brandon Nguyen

## Introduction
The .obj format is the traditional object code format for the LC-3.
It is an extremely simple format, supporting only a single contiguous section
with a pre-defined starting location and no embedded symbols. Instead, the
symbols were put into a separate .sym file

## Structure
The .obj is a binary file composed of big-endian 2-byte values: thus, the high order
byte is in the lower position of the file. The first value is the starting position:
if loaded into a simulator this would correspond to where to start placing the values.
The McGraw-Hill and most simulators will automatically set the **PC** to this location
as well.

For a program containing *n* used words
```
=============
|generic.obj|
=============
<starting pos>
<value 0>
<value 1>
<value 2>
...
<value n-1>
=============
```


If viewed in a hex editor, one could see something like this
if split by 2-byte values:
```
=============
|example.obj|
=============
30 00
12 14
f0 25
=============
```
which corresponds to:

```
=============
|example.asm|
=============
.orig x3000
add r1, r0, #4
trap x25
=============
```

## Other information
The .obj does not on its own contain any symbol information. A separate .sym
file is used for this purpose: a simulator would load the .obj and find the
corresponding .sym file based on the name before the ".obj".

The McGraw-Hill provided assembler also generated a plaintext hex and binary
version of the .obj: some simulators may support loading these directly.

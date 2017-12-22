# LCCC Assembler Syntax
by Brandon Nguyen

## Introduction
The LCCC Assembler (lasm) introduces an updated syntax
for LC-3 assembly in order to take advantage of the new
functionality of LLF. It is very similar to the original
assembly language, except with additional constructs

## Basics
Operands are typically read right to left: the destination registers are on the left
Memory access instructions have the register that data is written to or read from on the left.

Numeric literals are marked with prefixes: `#` for decimal and `0x` or `x` for hexadecimal
(`x` is supported for easier porting from original LC-3 assembly code). The negative sign
is placed *after* the prefix: e.g. `#-10` and `0x-A`.

## Standard LC-3 mnemonics
These are the traditional 1 to 1 assembly-machinecode mapped mnemonics for the opcodes.

| Logic/Math | Control | Memory | System |
|:-----------|:--------|:-------|:-------|
|`add`       |`br`     |`ld`    |`trap`  |
|`and`       |`jmp`    |`st`    |`rti`   |
|`not`       |`jsr`    |`ldr`   |        |
|            |`jsrr`   |`str`   |        |
|            |`ret`    |`ldi`   |        |
|            |`nop`    |`sti`   |        |

---
#### `add`
Adds two operands and places it in the destination register.
##### Syntax
Register-register: `add <dest reg>, <src reg 1>, <src reg 2>`
Register-immediate: `add <dest reg>, <src reg 1>, <immediate>`
##### Example
```
add r2, r1, r0
add r2, r1, #10
add r2, r1, 0x10
```
>TODO: finish; all these are effectively the same as the original LC-3

---

## Extended LCCC mnemonics
These are additional helpful mnemonics that provide higher level functionality that the LC-3 ISA doesn't support.
May or may not use multiple instructions.

| Mnemonics    |
|:-------------|
| `mov`        |
| `or`         |
| `neg`        |
| `push`/`pop` |

---
#### `mov`
Moves the contents of a register or a constant to a register
##### Syntax
Register-register: `mov <dest reg>, <src reg 1>, <src reg 2>`
Register-immediate: `mov <dest reg>, <src reg 1>, <immediate>`
##### Example
```
mov r2, r1, r0
mov r2, r1, #10
```

---
#### `neg`
Negates the number. This will produce one instruction
##### Syntax
`neg <dest reg>, <src reg>`
##### Example
```
neg r0, r0
neg r1, r0
```

---
#### `or`
Does a logical OR of two registers puts it in a destination register.
This will produce multiple instructions.
##### Syntax
`or <dest reg>, <src reg 1>, <src reg 2>`
##### Example
```
or r2, r1, r0
```

---
#### `push`/`pop`
Does a reg-list push and pop.
Will push registers in the order specified,
and pop them in the order specified. The last
register in the list pushed will have
the higher address. This will produce multiple
instructions.

Registers used in corresponding pushes and pops
do *not* have to have the same registers
##### Syntax
`push {<reg 1>, <reg 2>, ..., <reg n-1>`
`pop {<reg 1>, <reg 2>, ..., <reg n-1>`
##### Example
```
push {r0, r1, r2, r3}
pop {r0, r1, r2, r3}
```
---


## Assembler Directives
| Directives |
|:-----------|
| `.orig`    |
| `.blkw`    |
| `.fill`    |
| `.stringz`/`.string`  |
| `.stringp` |
| `.define`  |
| `.section` |
| `.export`  |
| `.import`  |
---
#### `.blkw`
Allocates words
##### Syntax
`.blkw <amount to allocate>`
##### Example
```
buffer  .blkw   0x100
```
The symbol `buffer` is now associated with 0x100 words

---
#### `.fill`
Places a constant in a word of memory
##### Syntax
`.fill <value to place>`
##### Example
```
MCR_LOC .fill   0xFFFE
ANSWER  .fill   42
```

---
#### `.stringz`/`.string`
Places a null-terminated string in memory. Due to word addressability,
each character will take up a single memory location.
`.stringz` is for backwards compatibility of old code.
##### Syntax
`.stringz "<string>"`
`.string "<string>"`
##### Example
```
.stringz "Hello world\n"
.string "Hello world\n"
```

---
#### `.stringp`
Places a null-terminated string in memory. Each character
will take a byte, and on word addressable machines (e.g. LC-3),
the string will be packed with two characters per word.
The first character will be in the low order byte.
##### Syntax
`.stringp "<string>"`
##### Example
```
.stringp "Hello world\n"
```

---
#### `.define`
Simple text repleacement macro a la C
##### Syntax
`.define <searched text> <replacement text>`
##### Example
```
.define SIZE 16
ldr r3,r1,#SIZE`
```
Instances of `SIZE` will be replaced with `16` after the `.define`

---
#### `.section`
Specifies a section of code or data.
Attributes will be supported in the future
##### Syntax
`.define <name>,<attributes>`
##### Example
```
.section .text
```

---
#### `.export`
Exports a label/symbol, making it visible to linkers.
##### Syntax
`.export <label/symbol name>`
##### Example
```
.export somefunc
somefunc
    add r0,r1,r0
    ret
```

---
#### `.import`
Imports a label/symbol that is visible from other object code.
##### Syntax
`.import <label/symbol name>`
##### Example
```
.import somefunc
main
    ld r3,somefunc_addr
    jsrr r3
    ret
somefunc_addr
    .fill somefunc
```
---


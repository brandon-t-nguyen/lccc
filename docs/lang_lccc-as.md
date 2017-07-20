# LCCC Assembler Syntax
by Brandon Nguyen

## Introduction
The LCCC Assembler (lasm) introduces an updated syntax
for LC-3 assembly in order to take advantage of the new
functionality of LLF. It is very similar to the original
assembly language, except with additional constructs

## Standard LC-3 mnemonics
These are the traditional 1 to 1 assembly-machinecode mapped mnemonics for the opcodes.

| Mnemonics   |
|:------------|
| `add`       |
| `and`       |
| `br`        |
| `jmp`       |
| `jsr`       |
| `jsrr`      |
| `ld`        |
| `ldi`       |
| `ldr`       |
| `not`       |
| `rti`       |
| `st`        |
| `sti`       |
| `str`       |
| `trap`      |

#### `add`
##### Syntax
Register-register: `add <dest reg>, <src reg 1>, <src reg 2>`
Register-immediate: `add <dest reg>, <src reg 1>, <immediate>`
##### Example
```
add r2, r1, r0
add r2, r1, #10
add r2, r1, 0x10
```

## Extended LCCC mnemonics
These are additional helpful mnemonics that provide higher level functionality that the LC-3 ISA doesn't support.

| Mnemonics    |
|:-------------|
| `mov`        |
| `or`         |
| `neg`        |
| `push`/`pop` |

## Assembler Directives
| Directives |
|:-----------|
| `.define`  |

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

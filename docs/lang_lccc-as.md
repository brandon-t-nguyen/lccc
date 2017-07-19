# LCCC Assembler Syntax
by Brandon Nguyen

## Introduction
The LCCC Assembler (lasm) introduces an updated syntax
for LC-3 assembly in order to take advantage of the new
functionality of LLF. It is very similar to the original
assembly language, except with additional constructs

## Standard LC-3 mnemonics
These are the traditional 1 to 1 assembly-machinecode mapped mnemonics for the opcodes.

## Extended LCCC mnemonics
These are additional helpful mnemonics that provide higher level functionality that the LC-3 ISA doesn't support.

| Mnemonic    |
|:------------|
| `MOV`       |
| `NEG`       |
| `PUSH`/`POP`|

## Assembler Directives
| Directive |
|:----------|
| `.define` |

#### `.define`
Simple text repleacement macro a la C
##### Syntax
`.define <searched text> <replacement text>`
##### Example
`.define SIZE 16`<br>`ldr r3,r1,#SIZE`
Instances of `HEAPSIZE` will be replaced with `16` after the `.define`

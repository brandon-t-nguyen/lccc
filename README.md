# Little Computer Compiler Collection
This is for a set of compiling tools for Yale Patt's Little Computer architectures.

## Cloning
This repository utilizes Git submodules. In order to clone this repository, you
will need to utilize the `--recurse-submodules` flag in your clone command e.g.
```
git clone --recurse-submodules https://github.com/aeturnus/lccc.git
```

## Dependencies
Right off the bat, the Makefile supports GCC and GNU Binutils. If you want to cross-compile for Windows (i.e. from a \*nix system), the Makefile also supports the MinGW-w64 toolchain.

## Supported systems
Right off the bat, \*nix systems should have no problems compiling this provided they have GCC and GNU Binutils installed (or an equivalent that symlinks the commands e.g. clang). This has only been testing on Linux and Windows, however.

Since this goes by the C99 standard, it *should* work on other \*nix systems that don't have some archaic toolchain.

## Compiling
Utilize the `vars.mk` file to configure your build variables, such as PLATFORM (\*nix or Windows)

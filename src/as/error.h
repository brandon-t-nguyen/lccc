#ifndef __ERROR_H__
#define __ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <btn/ansi.h>
#include <btn/print.h>

#define eprintf(fmt, ...) afprintf(stderr,\
                                   ANSI_F_BWHT "lccc-as: " ANSI_RESET \
                                   ANSI_F_BRED "error: " ANSI_RESET fmt "\n"\
                                   ,\
                                   ##__VA_ARGS__)
// Note: ##__VA_ARGS__ is not really portable, but is supported by
// gcc, llvm/clang, and VC++, so it's portable "enough"

#ifdef __cplusplus
}
#endif

#endif//__ERROR_H__

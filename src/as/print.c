#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <btn/ansi.h>
#include <btn/print.h>

#include "print.h"

static bool ansi_enable = true;
static msg_level base_level = M_INFO;

void enable_ansi(void)
{
    ansi_enable = true;
}

void disable_ansi(void)
{
    ansi_enable = false;
}

void msg_set_level(msg_level level)
{
    base_level = level;
}

static
const char * msg_level_str(msg_level level)
{
    switch(level) {
    case M_FATAL:
        return ANSI_RESET ANSI_F_BRED "fatal error" ANSI_RESET;
    case M_ERROR:
        return ANSI_RESET ANSI_F_BRED "error" ANSI_RESET;
    case M_WARN:
        return ANSI_RESET ANSI_F_BMAG "warning" ANSI_RESET;
    case M_INFO:
        return ANSI_RESET ANSI_F_BWHT "info" ANSI_RESET;
    case M_DEBUG:
        return ANSI_RESET ANSI_F_BGRN "debug" ANSI_RESET;
    case M_VERBOSE:
        return ANSI_RESET ANSI_F_BBLU "verbose" ANSI_RESET;
    default:
        return "<no level>";
    }
};

#define BUF_SIZE 1024

static
void vfmsg(FILE * file, const char * src, msg_level level, const char * fmt,
           va_list ap)
{
    if (level > base_level)
        return;

    char * buf = (char *) malloc(sizeof(char) * BUF_SIZE);
    snprintf(buf, BUF_SIZE, ANSI_RESET ANSI_F_BWHT "%s: %s: %s\n" ANSI_RESET,
             src, msg_level_str(level), fmt);
    aetvfprintf(ansi_enable, file, buf, ap);
    free(buf);
}

void msg(const char * src, msg_level level, const char * fmt, ...)
{
    FILE * stream = stdout;
    if (level <= M_WARN)
        stream = stderr;
    va_list ap;
    va_start(ap, fmt);
    vfmsg(stream, src, level, fmt, ap);
    va_end(ap);
};

void fmsg(FILE * file, const char * src, msg_level level, const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfmsg(file, src, level, fmt, ap);
    va_end(ap);
};

#ifndef __PRINT_H__
#define __PRINT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdarg.h>

#include <btn/ansi.h>
#include <btn/print.h>

void enable_ansi(void);
void disable_ansi(void);

typedef enum _msg_level
{
    M_FATAL = 0,
    M_ERROR,
    M_WARN,
    M_INFO,
    M_DEBUG,
    M_VERBOSE,
    M_LEVELS
} msg_level;

/**
 * Set the msg system verbosity level
 */
void msg_set_level(msg_level level);

/**
 * Prints a message in the format: "<src>: <level>: <format string>"
 * For warning and below, goes to stderr
 * For info and above, goes to stdout
 */
void msg(const char * src, msg_level level, const char * fmt, ...);

/**
 * Explicit file to write to
 */
void fmsg(FILE * file, const char * src, msg_level level, const char * fmt, ...);

/**
 * Full control msg
 */
void vfmsg(FILE * file, const char * src, msg_level level, const char * fmt,
           va_list ap);

#ifdef __cplusplus
}
#endif

#endif//__PRINT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <btn/vector.h>
#include <btn/cstr.h>
#include <btn/bst.h>

#include "as.h"
#include "asm.h"
#include "print.h"

void asm_string_error(const char * line,
                      size_t hlight_beg, size_t hlight_end, size_t pos)
{
    size_t len = strlen(line);

    if (hlight_beg == SIZE_MAX && hlight_end == SIZE_MAX) {
        printf("%s\n", line);
    } else {
        for (size_t i = 0; i < hlight_beg; ++i) {
            fputc(line[i], stdout);
        }
        printf(ANSI_F_BMAG);
        for (size_t i = hlight_beg; i <= hlight_end; ++i) {
            fputc(line[i], stdout);
        }
        printf(ANSI_RESET "%s\n", line + hlight_end + 1);
    }

    if (pos == SIZE_MAX)
        return;


    for (size_t i = 0; i < pos; ++i)
        fputc(' ', stdout);

    printf(ANSI_F_BMAG "^" ANSI_RESET);

    printf(ANSI_BOLD ANSI_F_BWHT);
    for (size_t i = 0; i < hlight_end - hlight_beg; ++i)
        fputc('~', stdout);
    printf(ANSI_RESET "\n"); // TODO: OS agnostic line separator macro?
}

void asm_line_error(const asm_line * line, size_t hlight_beg, size_t hlight_end, size_t pos)
{
    asm_string_error(line->raw, hlight_beg, hlight_end, pos);
}

void asm_line_token_error(const asm_line * line, const asm_token * token)
{
    size_t token_len = strlen(token->str);
    asm_string_error(line->raw, token->idx, token->idx + token_len - 1, token->idx);
}

#define MSG_PREFIX ANSI_BOLD ANSI_F_BWHT "%s:%d" ANSI_RESET ": "
static const char * msg_prefix = MSG_PREFIX;
static size_t msg_prefix_size = sizeof(MSG_PREFIX); // includes null term

void asm_msg_line(const asm_source * src,
                  const asm_line * line,
                  msg_level level, const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    // format print the file name and line number first
    char * buffer = (char *) malloc(sizeof(char) *
                                    (strlen(fmt) +
                                     msg_prefix_size +
                                     strlen(src->name) + 10)
                                    ); // +10 to cover the line number
    sprintf(buffer, msg_prefix, src->name, line->number);

    // append the caller's format string and call vfmsg with its formatting
    strcat(buffer, fmt);
    vfmsg(stdout, M_AS, level, buffer, ap);
    asm_line_error(line, SIZE_MAX, SIZE_MAX, SIZE_MAX);

    va_end(ap);

    free(buffer);
}

void asm_msg_line_token(const asm_source * src,
                        const asm_line * line,
                        const asm_token * token,
                        msg_level level, const char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    // format print the file name and line number first
    char * buffer = (char *) malloc(sizeof(char) *
                                    (strlen(fmt) +
                                     msg_prefix_size +
                                     strlen(src->name) + 10)
                                    ); // +10 to cover the line number
    sprintf(buffer, msg_prefix, src->name, line->number);

    // append the caller's format string and call vfmsg with its formatting
    strcat(buffer, fmt);
    vfmsg(stdout, M_AS, level, buffer, ap);
    asm_line_token_error(line, token);

    va_end(ap);

    free(buffer);
}

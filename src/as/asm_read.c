#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <btn/vector.h>
#include <btn/cstr.h>
#include <btn/bst.h>

#include "as.h"
#include "asm.h"
#include "print.h"

typedef enum _rd_state
{
    RD_SEARCH,      // searches for non-whitespace
    RD_GRAB,        // grabbing valid characters
    RD_QUOTE,       // grabbing quotes
    RD_IGNORE,      // ignores rest of line
} rd_state;

static
char * read_line(FILE * f)
{
    size_t cap = 64;
    size_t size = 0;
    char * line = (char *) malloc(sizeof(char) * cap + 1);

    char c  = fgetc(f);

    if (c == EOF) {
        free(line);
        return NULL;
    }

    while (c != EOF && c != '\n') {
        line[size] = c;
        ++size;

        // resize
        if (size >= cap) {
            cap = cap * 2;
            line = (char *) realloc(line, cap + 1);
        }
        c = fgetc(f);
    }

    // when we get here, check to see if last char is an \r
    // if it is, turn it into the NULL character
    if (line[size-1] == '\r' && c == EOF) {
        line[size-1] = '\0';
    } else {
        line[size] = '\0';
    }

    return line;
}

static inline
void push_token(asm_line * line, const char * str, size_t start, size_t end)
{
    asm_token token;
    token.str = stridup(str, start, end);
    token.idx = start;
    vector_push_back(&line->tokens, &token);
}

as_ret asm_source_read(asm_source * code, const char * path, FILE * f)
{
    code->name = path;

    unsigned int line_no = 1;

    char * raw_line = read_line(f);
    while (raw_line != NULL) {
        asm_line line;
        asm_line_ctor(&line);

        rd_state state = RD_SEARCH;
        size_t rd_head = 0;
        size_t s_head = 0;
        size_t e_head = 0;

        char c = 1;
        while (c != '\0' && state != RD_IGNORE) {
            c = raw_line[rd_head];
            switch (state) {
            case RD_SEARCH:
                if (c == ' ' || c == '\t' || c == '\0') {
                    ++rd_head;
                } else if (c == '"') {
                    s_head = rd_head;
                    ++rd_head;
                    state = RD_QUOTE;
                } else if (c == ';') {
                    state = RD_IGNORE;
                } else {
                    s_head = rd_head;
                    ++rd_head;
                    state = RD_GRAB;
                }
                break;
            case RD_GRAB:
                if (c == ' ' || c == '\t' || c == '\0') {
                    e_head = rd_head - 1;
                    push_token(&line, raw_line, s_head, e_head);
                    ++rd_head;
                    state = RD_SEARCH;
                } else if (c == ',') {
                    // comma is special: separates operands, doesn't need spacing
                    e_head = rd_head - 1;
                    push_token(&line, raw_line, s_head, e_head);
                    s_head = rd_head;
                    e_head = rd_head;
                    push_token(&line, raw_line, s_head, e_head);
                    ++rd_head;
                    state = RD_SEARCH;
                } else if (c == ';') {
                    e_head = rd_head - 1;
                    push_token(&line, raw_line, s_head, e_head);
                    state = RD_IGNORE;
                } else {
                    ++rd_head;
                }
                break;
            case RD_QUOTE:
                if (c == '"') {
                    if (raw_line[rd_head-1] != '\\') {
                        e_head = rd_head;
                        push_token(&line, raw_line, s_head, e_head);
                        ++rd_head;
                        state = RD_SEARCH;
                    } else {
                        ++rd_head;
                    }
                } else if (c == '\0') {
                    msg(M_AS, M_ERROR,
                         ANSI_BOLD ANSI_F_BWHT "%s:%d" ANSI_RESET ": "
                         "Unterminated string",
                         path, line_no);
                    asm_string_error(raw_line, s_head, rd_head, rd_head);
                    return AS_RET_BAD_INPUT;
                } else {
                    ++rd_head;
                }
                break;
            case RD_IGNORE:
                break;
            }
        }

        // done with this line: store it in the source
        if (vector_size(&line.tokens) > 0) {
            // keep the raw, read-in line for printing
            line.raw = raw_line;
            line.number = line_no;
            // code->lines manages the line now
            vector_push_back(&code->lines, &line);
        } else {
            asm_line_dtor(&line);
            free(raw_line);
        }

        raw_line = read_line(f);
        ++line_no;
    }

    return AS_RET_OK;
}

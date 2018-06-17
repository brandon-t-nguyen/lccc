// simple assembler for Patt syntax
// two-pass assembly
// This follows the syntax presented in Introduction to Computing, 2nd Ed.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <btn/vector.h>
#include <btn/cstr.h>

#include "as.h"
#include "asm.h"
#include "print.h"
#include "option.h"

typedef struct _program
{
    asm_source src;

    uint16_t entry;
    vector(uint16_t) data;

    struct _program * next; // simple linking
} program;


static
program * program_new(void)
{
    program * prog = (program *) malloc(sizeof(program));

    asm_source_ctor(&prog->src);
    prog->entry = 0xFFFF;
    vector_ctor(&prog->data, sizeof(uint16_t), NULL, NULL);
    prog->next = NULL;

    return prog;
}

static
void program_delete(program * prog)
{
    asm_source_dtor(&prog->src);
    vector_dtor(&prog->data);
    free(prog);
}

static inline
size_t program_size(program * prog) {return vector_size(&prog->data);}

// inserts p into the list header forms
// also checks for linkage issues (e.g. previous program overlaps next program)
static
int insert(program * header, program * p)
{
    program * curr = header->next;
    program * prev = header;

    if (curr == NULL) {
        header->next = p;
        return 0;
    }

    // normal case
    if (program_size(p) > program_size(prev) &&
        program_size(p) < program_size(curr)) {

        // check for linkage issues
        // see if p overlaps with prev
        if (prev != header &&
            p->entry < (prev->entry + program_size(prev))
            ) {
            msg(M_AS, M_FATAL,
                "Unable to link " ANSI_F_BWHT "%s" ANSI_RESET
                ": entry point of " ANSI_F_BWHT "%s" ANSI_RESET "(x%04X) is located in a region of memory used by "
                ANSI_F_BWHT "%s" ANSI_RESET, p->src.name, p->src.name, p->entry, prev->src.name);
            return 1;
        }

        // see if p overlaps with curr
        if (p->entry + program_size(p) >= curr->entry) {
            msg(M_AS, M_FATAL,
                "Unable to link " ANSI_F_BWHT "%s" ANSI_RESET
                ": " ANSI_F_BWHT "%s" ANSI_RESET " overruns entry point of "
                ANSI_F_BWHT "%s" ANSI_RESET " (x%04X)",
                p->src.name, p->src.name, curr->src.name, curr->entry);
            return 1;
        }

    }

    return 0;
}

static
int assemble(program * p)
{
    // TODO
    return 0;
}

typedef enum _rd_state
{
    RD_SEARCH,      // searches for non-whitespace
    RD_GRAB,        // grabbing valid characters
    RD_QUOTE,       // grabbing quotes
    RD_IGNORE,      // ignores rest of line
} rd_state;

#define TILDE_COUNT 4
static
void show_line_error(const char * line,
                     size_t hlight_beg, size_t hlight_end, size_t pos)
{
    size_t len = strlen(line);

    for (size_t i = 0; i < hlight_beg; ++i) {
        fputc(line[i], stdout);
    }
    printf(ANSI_F_BMAG);
    for (size_t i = hlight_beg; i <= hlight_end; ++i) {
        fputc(line[i], stdout);
    }
    printf(ANSI_RESET "%s\n", line + hlight_end + 1);

    if (pos > TILDE_COUNT) {
        for (size_t i = 0; i < pos - 1 - TILDE_COUNT; ++i) {
            fputc(' ', stdout);
        }
    }
    printf(ANSI_BOLD ANSI_F_BWHT);
    for (size_t i = (pos > TILDE_COUNT) ? pos - 1 - TILDE_COUNT : 0; i < pos - 1; ++i) {
        fputc('~', stdout);
    }
    printf(ANSI_F_BMAG "^\n" ANSI_RESET);
}

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
    char * token = stridup(str, start, end);
    vector_push_back(&line->tokens, &token);
}

static
int read_file(const char * path, FILE * f, asm_source * code)
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
                if (c == ' ' || c == '\t') {
                    e_head = rd_head - 1;
                    push_token(&line, raw_line, s_head, e_head);
                    ++rd_head;
                    state = RD_SEARCH;
                } else if (c == '"') {
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
                    show_line_error(raw_line, s_head, rd_head, rd_head);
                    return 1;
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
            // code->lines manages the line now
            vector_push_back(&code->lines, &line);
        } else {
            asm_line_dtor(&line);
            free(raw_line);
        }

        raw_line = read_line(f);
        ++line_no;
    }

    return 0;
}

int asm_patt(const as_params * params, const vector(char *) * file_paths, const vector(FILE *) * files)
{
    int ret = 0;
    program header; // blank program that points to the first actual program
    header.next = NULL;

    // go through each file, read it, assemble it, and dump it to its program
    for (int i = 0; i < (int) vector_size(files) && ret == 0; ++i) {
        program * prog = program_new();

        // tokenize the file
        FILE * f;
        const char * path;
        vector_get(files, i, &f);
        vector_get(file_paths, i, &path);
        read_file(path, f, &prog->src);

        // assemble the file
        ret = assemble(prog);

        if (ret == 0) {
            ret = insert(&header, prog);
        }

        // TODO REMOVE; DEBUG PURPOSES
        for (int i = 0; i < vector_size(&prog->src.lines); ++i) {
            asm_line * line = vector_getp(&prog->src.lines, i);

            printf("%4d: ", i);
            for (int j = 0; j < vector_size(&line->tokens); ++j) {
                printf("%s ", *(char **) vector_getp(&line->tokens, j));
            }
            printf("\n");
        }
    }


    // clean up the programs
    program * curr = header.next;
    program * next = NULL;
    while (curr != NULL) {
        next = curr->next;
        program_delete(curr);
        curr = next;
    }
    return ret;
}

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

static
char * read_line(FILE * f)
{
    size_t cap = 64;
    size_t size = 0;
    char * line = (char *) malloc(sizeof(char) * cap + 1);

    // TODO: remember to handle dos line endings
    char c  = fgetc(f);
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

static
void read_file(const char * path, FILE * f, asm_source * code)
{
    asm_line line;
    asm_line_ctor(&line);

    rd_state state = RD_SEARCH;
    unsigned int line_no = 0;

    char * raw_line = read_line(f);
    while (raw_line[0] != '\0') {
        ++line_no;
        printf("%4d: %s\n", line_no, raw_line);

        raw_line = read_line(f);
    }

    /*
    while (1) {
        switch (state) {
        case RD_SEARCH:
            break;
        case RD_GRAB:
            break;
        case RD_QUOTE:
            break;
        case RD_IGNORE:
            break;
        }
    }
    */
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
    }


    // clean up the programs
    for (program * curr = header.next; curr != NULL; curr = curr->next) {
        program_delete(curr);
    }
    return ret;
}

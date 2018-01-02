#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <btn/vector.h>
#include <btn/cstr.h>

#include "as.h"
#include "print.h"

#define DEF_PATT
//#define DEF_LCCC

static const as_params default_patt = {
    .format = AS_OF_OBJ,
    .syntax = AS_SYNTAX_PATT
};

static const as_params default_lccc  = {
    .format = AS_OF_LLF,
    .syntax = AS_SYNTAX_LCCC
};

#if defined DEF_PATT
#define DEFAULT_PARAMS default_patt
#define COMPILE_STRING "Classic version: defaults to the Patt's assembler syntax and McGraw-Hill toolchain .obj format\n"
#elif defined DEF_LCCC
#define DEFAULT_PARAMS default_lccc
#define COMPILE_STRING "lccc version: defaults to the lccc assembler syntax and LLF binary output format\n"
#endif

extern const as_opt as_options[];
extern size_t as_num_options;

void print_version(void)
{
    printf("LCCC LC-3 assembler v0.01a\n"
           "Copyright (C) 2017 Brandon Nguyen\n"
           "\n"
           COMPILE_STRING);
}

void print_opt(const as_opt * opt)
{
    #define INDENT 2
    #define DETAIL_INDENT 25
    static const char * detail_indent = "                           ";
    size_t opt_len = 0;
    fputs("  ", stdout);
    if (opt->s_arg) {
        fputs(opt->s_arg, stdout);
        opt_len += strlen(opt->s_arg);
        if (opt->l_arg) {
            fputs(", ", stdout);
            opt_len += 2;
        } else if (opt->p_str) {
            if (opt->assign)
                fputs(" ", stdout);
            else
                fputs(" ", stdout);
            opt_len += 1;
        }
    }
    if (opt->l_arg) {
        fputs(opt->l_arg, stdout);
        opt_len += strlen(opt->l_arg);
        if (opt->p_str) {
            if (opt->assign)
                fputs(" ", stdout);
            else
                fputs(" ", stdout);
            opt_len += 1;
        }
    }
    if (opt->p_str) {
        fputs(opt->p_str, stdout);
        opt_len += strlen(opt->p_str);
    }

    for (size_t i = opt_len; i < DETAIL_INDENT; ++i) {
        fputs(" ", stdout);
    }

    // print out details string
    if (opt_len >= DETAIL_INDENT) {
        fputs("\n", stdout);
        fputs(detail_indent, stdout);
    }
    size_t i = 0;
    char c;
    while ((c = opt->d_str[i]) != '\0') {
        if (c == '\n') {
            puts("");   // sys independent newline
            fputs(detail_indent, stdout);
        } else {
            fputc(c, stdout);
        }
        ++i;
    }
    puts("");
}

void print_help(void)
{
    printf("Usage: lccc-as [options...] [assembly source files...]\n");
    printf("Options:\n");
    bool section = false;
    for (size_t i = 0; i < as_num_options; ++i) {
        const as_opt * opt = &as_options[i];
        if (opt->func == NULL) {
            if (opt->assign) {
                if (section) {
                    // already in section: end section
                    puts("");
                    section = false;
                } else {
                    // not section: start a new section
                    puts("");
                    puts(opt->d_str);
                    section = true;
                }
            } else {
                puts(opt->d_str);
            }
            continue;
        }
        print_opt(opt);
    }
}

static inline
bool check_opt(const as_opt * opt, int * parg_i, const char * arg,
               as_params * params, vector * files, int argc, char ** argv)
{
    bool match = false;
    // typical case: match against the short arg or long arg
    if (!opt->assign) {
        if ((opt->s_arg && strcmp(opt->s_arg, arg) == 0) ||
            (opt->l_arg && strcmp(opt->l_arg, arg) == 0)) {
            int ret = opt->func(params, arg, NULL);
            if (ret != 0)
                exit(ret);
            match = true;
        }
    } else {
        const char * assign = NULL;

        if (opt->s_arg != NULL && arg[1] == opt->s_arg[1]) {
            match = true;
            // for short args
            // opt-arg can either come from arg itself or next arg
            if (arg[2] != '\0') {
                assign = &arg[2];
            } else {
                // next arg
                (*parg_i) += 1;
                if (*parg_i < argc)
                    assign = argv[*parg_i];
            }
        } else if (opt->l_arg != NULL) {
            // long args are a bit more complicated: can use = or space
            size_t equal_idx = strcfind(arg, '=', 0);
            if (equal_idx != SIZE_MAX) {
                // found an equal: need to use strncmp
                if (strncmp(opt->l_arg, arg, equal_idx) == 0) {
                    match = true;
                    assign = &arg[equal_idx + 1];
                }
            } else {
                // no equal: try straight strcmp
                if (strcmp(opt->l_arg, arg) == 0) {
                    match = true;
                    // next arg
                    (*parg_i) += 1;
                    if (*parg_i < argc)
                        assign = argv[*parg_i];
                }
            }
        }
        if (match) {
            if (assign != NULL && assign[0] == '\0')
                assign = NULL;
            int ret = opt->func(params, arg, assign);
            if (ret != 0)
                exit(ret);
        }
    }
    return match;
}

void parse_opts(as_params * params, vector * files, int argc, char ** argv)
{
    for (int arg_i = 1; arg_i < argc; ++arg_i) {
        const char * arg = argv[arg_i];
        bool is_opt = false;
        bool check_opts = true;

        // check against the options if arg starts with a dash
        if (check_opts && arg[0] == '-') {
            // "--" will turn off future opt checking
            if (strcmp("--", arg) == 0) {
                check_opts = false;
                continue;
            }

            for (size_t opt_i = 0; !is_opt && opt_i < as_num_options; ++opt_i) {
                const as_opt * opt = &as_options[opt_i];
                if (opt->func == NULL)  // is printing metadata
                    continue;
                is_opt = check_opt(opt, &arg_i, arg, params, files, argc, argv);
            }

            if (!is_opt) {
                msg(M_AS, M_ERROR,
                    "Unrecognized command line option " ANSI_F_BWHT "'%s'",
                    arg);
            }
        } else {
            // interpret this as a file
            vector_push_back(files, arg);
        }
    }
}

int main(int argc, char ** argv)
{
    // construct the params
    as_params driver_params = DEFAULT_PARAMS;
    driver_params.output_file = "out.obj";
    vector(const char *) files;
    vector_ctor(&files, sizeof(const char *), NULL, NULL);

    parse_opts(&driver_params, &files, argc, argv);

    if (vector_size(&files) == 0) {
        msg(M_AS, M_FATAL, "No input files");
        exit(AS_RET_NO_INPUT);
    }
}

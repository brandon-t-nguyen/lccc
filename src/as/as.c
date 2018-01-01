#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <btn/vector.h>

#include "as.h"

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
            fputs(" ", stdout);
            opt_len += 1;
        }
    }
    if (opt->l_arg) {
        fputs(opt->l_arg, stdout);
        opt_len += strlen(opt->l_arg);
        if (opt->p_str) {
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
    for (size_t i = 0; i < as_num_options; ++i) {
        print_opt(&as_options[i]);
    }
}

void parse_opts(as_params * params, vector * files, int argc, char ** argv)
{
    for (int arg_i = 1; arg_i < argc; ++arg_i) {
        const char * arg = argv[arg_i];
        bool is_opt = false;

        // check against the options if arg starts with a dash
        if (arg[0] == '-') {
            for (size_t opt_i = 0; opt_i < as_num_options; ++opt_i) {
                const as_opt * opt = &as_options[opt_i];
                // typical case: match against the short arg or long arg
                if (!opt->assign) {
                    if ((opt->s_arg && strcmp(opt->s_arg, arg) == 0) ||
                        (opt->l_arg && strcmp(opt->l_arg, arg) == 0)) {
                        opt->func(params, &arg_i, argc, argv);
                        is_opt = true;
                        break;
                    }
                } else {
                    // TODO: logic for assignment
                    if (false) {
                    }
                }
            }
        }

        if (!is_opt) {
            // interpret this as a file
            // TODO: remove
            printf("DBG: Added file %s\n", arg);
            vector_push_back(files, arg);
        }
    }
}
#include <unistd.h>
#include <btn/print.h>
#include <btn/ansi.h>
int main(int argc, char ** argv)
{
    // construct the params
    as_params driver_params = DEFAULT_PARAMS;
    driver_params.output_file = "out.obj";
    vector(const char *) files;
    vector_ctor(&files, sizeof(const char *), NULL, NULL);

    parse_opts(&driver_params, &files, argc, argv);
}

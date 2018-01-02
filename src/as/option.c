#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <btn/cstr.h>
#include <btn/print.h>
#include <btn/vector.h>

#include "print.h"
#include "option.h"

void print_option(const option * opt)
{
    #define INDENT 2
    #define DETAIL_INDENT 25
    static const char * detail_indent = "                           ";
    size_t opt_len = 0;
    fputs("  ", stdout);
    if (opt->s_arg) {
        fputs(opt->s_arg, stdout);
        opt_len += strlen(opt->s_arg);

        // print opt-arg if there
        if (opt->p_str) {
            printf(" %s", opt->p_str);
            opt_len += strlen(opt->p_str) + 1;
        }

        if (opt->l_arg) {
            fputs(", ", stdout);
            opt_len += 2;
        }
    }
    if (opt->l_arg) {
        fputs(opt->l_arg, stdout);
        opt_len += strlen(opt->l_arg);

        // print out the opt-arg with =
        if (opt->p_str) {
            printf("=%s", opt->p_str);
            opt_len += strlen(opt->p_str) + 1;
        }
    }

    // print out the default
    if (opt->pd_str) {
        printf(" (default '%s')", opt->pd_str);
        opt_len += strlen(opt->pd_str) + 13;
    }

    // fill out spaces until the details
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

void print_options(const option * options, size_t num_options)
{
    bool section = false;
    for (size_t i = 0; i < num_options; ++i) {
        const option * opt = &options[i];
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
        print_option(opt);
    }
}

static inline
bool check_option(const option * opt, int * parg_i, const char * arg,
                  void * aux, vector * non_options, int argc, char ** argv)
{
    bool match = false;
    // typical case: match against the short arg or long arg
    if (!opt->assign) {
        if ((opt->s_arg && strcmp(opt->s_arg, arg) == 0) ||
            (opt->l_arg && strcmp(opt->l_arg, arg) == 0)) {
            int ret = opt->func(aux, arg, NULL);
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
            int ret = opt->func(aux, arg, assign);
            if (ret != 0)
                exit(ret);
        }
    }
    return match;
}

void parse_options(const char * src, const option * options, size_t num_options,
                   void * aux, vector * non_options, int argc, char ** argv)
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

            for (size_t opt_i = 0; !is_opt && opt_i < num_options; ++opt_i) {
                const option * opt = &options[opt_i];
                if (opt->func == NULL)  // is printing metadata
                    continue;
                is_opt = check_option(opt, &arg_i, arg, aux, non_options, argc, argv);
            }

            if (!is_opt) {
                msg(src, M_ERROR,
                    "Unrecognized command line option " ANSI_F_BWHT "'%s'",
                    arg);
            }
        } else {
            // interpret this as a non-option
            vector_push_back(non_options, &arg);
        }
    }
}

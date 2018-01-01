#include <stdlib.h>
#include <stdbool.h>

#include <btn/ansi.h>
#include <btn/print.h>
#include <btn/cstr.h>

#include "as.h"
#include "print.h"

// This file contains the functions that modify the opts

#define ARRAY_LEN(x) (sizeof(x)/sizeof(x[0]))

static inline
const char * get_assign(const char * arg) {
    size_t equal_idx = strcfind(arg, '=', 0);
    const char * select = arg + equal_idx + 1;
    return select;
}

extern void print_help(void);
extern void print_version(void);

static
int opt_help(as_params * params, int * arg_idx, int argc, char ** args)
{
    print_help();
    exit(AS_RET_OK);
}

static
int opt_version(as_params * params, int * arg_idx, int argc, char ** args)
{
    print_version();
    exit(AS_RET_OK);
}

static
int opt_color(as_params * params, int * arg_idx, int argc, char ** args)
{
    enable_ansi();
    return AS_RET_OK;
}

static
int opt_no_color(as_params * params, int * arg_idx, int argc, char ** args)
{
    disable_ansi();
    return AS_RET_OK;
}

static
int opt_output(as_params * params, int * arg_idx, int argc, char ** args)
{
    // take the next arg as the objfile
    *arg_idx += 1;
    if (*arg_idx >= argc) {
        msg(M_AS, M_FATAL, "No output file provided");
        return AS_RET_OTHER;
    }

    return AS_RET_OK;
}

static
int opt_syntax(as_params * params, int * arg_idx, int argc, char ** args)
{
    const char * arg    = args[*arg_idx];
    const char * select = get_assign(arg);

    if (strcmp(select, "patt") == 0) {
        params->syntax = AS_SYNTAX_PATT;
    } else if (strcmp(select, "lccc") == 0) {
        params->syntax = AS_SYNTAX_LCCC;
    } else {
        msg(M_AS, M_FATAL,
            "Unrecognized syntax \"" ANSI_F_BRED "%s" ANSI_RESET
            "\" from argument \"%s\""
            ,select, arg);
        return AS_RET_OTHER;
    }

    return AS_RET_OK;
}

static
int opt_out_format(as_params * params, int * arg_idx, int argc, char ** args)
{
    const char * arg    = args[*arg_idx];
    const char * select = get_assign(arg);

    if (strcmp(select, "obj") == 0) {
        params->syntax = AS_OF_OBJ;
    } else if (strcmp(select, "llf") == 0) {
        params->syntax = AS_OF_LLF;
    } else {
        msg(M_AS, M_FATAL,
            "Unrecognized format \"" ANSI_F_BRED "%s" ANSI_RESET
            "\" from argument \"%s\""
            ,select, arg);
        return AS_RET_OTHER;
    }

    return AS_RET_OK;
}

const as_opt as_options[] = {
    {"-h", "--help",        false,  opt_help,           NULL,
        "Shows this help prompt and exits"},
    {NULL, "--version",     false,  opt_version,        NULL,
        "Shows the version information"},
    {NULL, "--color",       false,  opt_color,          NULL,
        "Turns on color printing"},
    {NULL, "--no-color",    false,  opt_no_color,       NULL,
        "Turns off color printing"},
    {"-o", "--output",      false,  opt_output,         "OBJFILE",
        "Set the name of the object file (defaults to \"out.obj\")"},
    {NULL, "--syntax",      true,   opt_syntax,         "[patt|lccc]",
        "Sets the assembler syntax to Patt or LCCC"},
    {"-O", "--out-format",  true,   opt_out_format,     "[obj|llf]",
        "Sets the output format to .obj or LLF"},
};
size_t as_num_options = ARRAY_LEN(as_options);

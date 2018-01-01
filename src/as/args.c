#include <stdbool.h>
#include <btn/ansi.h>
#include <btn/print.h>

#include "as.h"
#include "error.h"

// This file contains the functions that modify the opts

#define ARRAY_LEN(x) (sizeof(x)/sizeof(x[0]))

extern void print_help(void);
extern void print_version(void);

static
int opt_help(as_params * params, int * arg_idx, int argc, char ** args)
{
    print_help();
    return AS_RET_OK;
}

static
int opt_version(as_params * params, int * arg_idx, int argc, char ** args)
{
    print_version();
    return AS_RET_OK;
}

static
int opt_output(as_params * params, int * arg_idx, int argc, char ** args)
{
    // take the next arg as the objfile
    *arg_idx += 1;
    if (*arg_idx >= argc) {
        eprintf("No output file provided");
        return false;
    }

    return AS_RET_OK;
}

const as_opt as_options[] = {
    {"-h", "--help", false, opt_help, NULL, "Shows this help prompt and exits"},
    {NULL, "--version", false, opt_version, NULL, "Shows the version information"},
    {"-o", "--output", false, opt_output, "OBJFILE", "Set the name of the object file (defaults to \"out.obj\")"},
    {NULL, "-msyntax", true, NULL, "[patt|lccc]", "Set the name of the object file (defaults to \"out.obj\")"},
};
size_t as_num_options = ARRAY_LEN(as_options);

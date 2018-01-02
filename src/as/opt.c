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
int opt_help(as_params * params, const char * arg, const char * assign)
{
    print_help();
    exit(AS_RET_OK);
}

static
int opt_version(as_params * params, const char * arg, const char * assign)
{
    print_version();
    exit(AS_RET_OK);
}

static
int opt_color(as_params * params, const char * arg, const char * assign)
{
    enable_ansi();
    return AS_RET_OK;
}

static
int opt_no_color(as_params * params, const char * arg, const char * assign)
{
    disable_ansi();
    return AS_RET_OK;
}

static
int opt_output(as_params * params, const char * arg, const char * assign)
{
    // take the next arg as the objfile
    if (assign == NULL) {
        msg(M_AS, M_FATAL, "No output file provided");
        return AS_RET_OTHER;
    }

    return AS_RET_OK;
}

static
int opt_syntax(as_params * params, const char * arg, const char * assign)
{
    if (strcmp(assign, "patt") == 0) {
        params->syntax = AS_SYNTAX_PATT;
    } else if (strcmp(assign, "lccc") == 0) {
        params->syntax = AS_SYNTAX_LCCC;
    } else {
        msg(M_AS, M_FATAL,
            "Unrecognized syntax '" ANSI_F_BWHT "%s" ANSI_RESET
            "' from argument " ANSI_F_BWHT "'%s'"
            ,assign, arg);
        return AS_RET_OTHER;
    }

    return AS_RET_OK;
}

static
int opt_out_format(as_params * params, const char * arg, const char * assign)
{
    if (strcmp(assign, "obj") == 0) {
        params->syntax = AS_OF_OBJ;
    } else if (strcmp(assign, "llf") == 0) {
        params->syntax = AS_OF_LLF;
    } else {
        msg(M_AS, M_FATAL,
            "Unrecognized format '" ANSI_F_BWHT "%s" ANSI_RESET
            "' from argument " ANSI_F_BWHT "'%s'"
            ,assign, arg);
        return AS_RET_OTHER;
    }

    return AS_RET_OK;
}

static
int opt_in_format(as_params * params, const char * arg, const char * assign)
{
    return AS_RET_OK;
}

static
int opt_obj_hex(as_params * params, const char * arg, const char * assign)
{
    return AS_RET_OK;
}

static
int opt_obj_bin(as_params * params, const char * arg, const char * assign)
{
    return AS_RET_OK;
}

static
int opt_obj_sym(as_params * params, const char * arg, const char * assign)
{
    return AS_RET_OK;
}

static
int opt_obj_lst(as_params * params, const char * arg, const char * assign)
{
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
    {"-o", "--output",      true,   opt_output,         "OBJFILE",
        "Set the name of the object file (defaults to \"out.obj\")"},
    {NULL, "--syntax",      true,   opt_syntax,         "[patt|lccc]",
        "Sets the assembler syntax to Patt or LCCC"},
    {NULL, "--out-format",  true,   opt_out_format,     "[obj|llf]",
        "Sets the output format to .obj or LLF"},


    {NULL, NULL, true, NULL, NULL,
        "These following options are relevant when the output format is .obj"},
    {NULL, "--in-format",   true,   opt_in_format,     "[asm|bin|hex] (default asm)",
        "Sets the input format to assemble/translate"},
    {NULL, "--hex",         false,  opt_obj_hex,        NULL,
        "Produce plaintext hexadecimal file\n"
        "Output file retains the filename except with a\n"
        ".hex extension"},
    {NULL, "--bin",         false,  opt_obj_bin,        NULL,
        "Produce plaintext binary file\n"
        "Output file retains the filename except with a\n"
        ".bin extension"},
    {NULL, "--sym",         false,  opt_obj_sym,        NULL,
        "Produce a symtol table file\n"
        "Output file retains the filename except with a\n"
        ".sym extension"},
    {NULL, "--lst",         false,  opt_obj_lst,        NULL,
        "Produce a symtol table file\n"
        "Output file retains the filename except with a\n"
        ".sym extension"},
    {NULL, NULL, true, NULL, NULL, NULL},

};
size_t as_num_options = ARRAY_LEN(as_options);

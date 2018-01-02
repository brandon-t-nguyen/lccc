#include <stdlib.h>
#include <stdbool.h>

#include <btn/ansi.h>
#include <btn/print.h>
#include <btn/cstr.h>

#include "as.h"
#include "print.h"
#include "option.h"

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
int opt_help(void * aux, const char * arg, const char * assign)
{
    print_help();
    exit(AS_RET_OK);
}

static
int opt_version(void * aux, const char * arg, const char * assign)
{
    print_version();
    exit(AS_RET_OK);
}

static
int opt_color(void * aux, const char * arg, const char * assign)
{
    enable_ansi();
    return AS_RET_OK;
}

static
int opt_no_color(void * aux, const char * arg, const char * assign)
{
    disable_ansi();
    return AS_RET_OK;
}

static
int opt_output(void * aux, const char * arg, const char * assign)
{
    as_params * params = (as_params *) aux;
    // take the next arg as the objfile
    if (assign == NULL) {
        msg(M_AS, M_FATAL, "No output file provided");
        return AS_RET_OTHER;
    }

    params->output_file = assign;

    return AS_RET_OK;
}

static
int opt_syntax(void * aux, const char * arg, const char * assign)
{
    as_params * params = (as_params *) aux;
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
int opt_out_format(void * aux, const char * arg, const char * assign)
{
    as_params * params = (as_params *) aux;
    if (strcmp(assign, "obj") == 0) {
        params->syntax = AS_OF_OBJ;
    } else if (strcmp(assign, "llf") == 0) {
        params->syntax = AS_OF_LLF;
    } else {
        msg(M_AS, M_FATAL,
            "Unrecognized output format '" ANSI_F_BWHT "%s" ANSI_RESET
            "' from argument " ANSI_F_BWHT "'%s'"
            ,assign, arg);
        return AS_RET_OTHER;
    }

    return AS_RET_OK;
}

static
int opt_in_format(void * aux, const char * arg, const char * assign)
{
    as_params * params = (as_params *) aux;
    as_ret ret = AS_RET_OK;
    if (strcmp(assign, "asm") == 0) {
        params->iformat = AS_IF_ASM;
    }
    else if (strcmp(assign, "hex") == 0) {
        params->iformat = AS_IF_HEX;
    }
    else if (strcmp(assign, "bin") == 0) {
        params->iformat = AS_IF_BIN;
    } else {
        msg(M_AS, M_FATAL,
            "Unrecognized input format '" ANSI_F_BWHT "%s" ANSI_RESET
            "' from argument " ANSI_F_BWHT "'%s'"
            ,assign, arg);
        ret = AS_RET_OTHER;
    }
    return ret;
}

static
int opt_obj_hex(void * aux, const char * arg, const char * assign)
{
    as_params * params = (as_params *) aux;
    params->out_hex = true;
    return AS_RET_OK;
}

static
int opt_obj_bin(void * aux, const char * arg, const char * assign)
{
    as_params * params = (as_params *) aux;
    params->out_bin = true;
    return AS_RET_OK;
}

static
int opt_obj_sym(void * aux, const char * arg, const char * assign)
{
    as_params * params = (as_params *) aux;
    params->out_sym = true;
    return AS_RET_OK;
}

static
int opt_obj_lst(void * aux, const char * arg, const char * assign)
{
    as_params * params = (as_params *) aux;
    params->out_lst = true;
    return AS_RET_OK;
}

#if defined DEF_PATT
#define DEF_SYNTAX "patt"
#define DEF_FORMAT "obj"
#elif defined DEF_LCCC
#define DEF_SYNTAX "lccc"
#define DEF_FORMAT "llf"
#endif

const option as_options[] = {
    {"-h", "--help",        false,  opt_help,       NULL,   NULL,
        "Shows this help prompt and exits"},
    {NULL, "--version",     false,  opt_version,    NULL,   NULL,
        "Shows the version information"},
#if !(defined PLATFORM_WINDOWS)
    {NULL, "--color",       false,  opt_color,      NULL,   NULL,
        "Turns on color printing"},
    {NULL, "--no-color",    false,  opt_no_color,   NULL,   NULL,
        "Turns off color printing"},
#endif
    {"-o", "--output",      true,   opt_output,     "OBJFILE",      "out.obj",
        "Set the name of the object file"},
    {NULL, "--syntax",      true,   opt_syntax,     "[patt|lccc]",  DEF_SYNTAX,
        "Sets the assembler syntax to Patt or LCCC"},
    {NULL, "--out-format",  true,   opt_out_format, "[obj|llf]",    DEF_FORMAT,
        "Sets the output format to .obj or LLF"},
    {NULL, "--lst",         false,  opt_obj_lst,    NULL,   NULL,
        "Produce a assembly listing file\n"
        "Output file retains the file name except with a .lst extension"},

    {NULL, NULL, true, NULL, NULL, NULL,
        "These following options are relevant when the output format is .obj"},
    {NULL, "--in-format",   true,   opt_in_format,  "[asm|bin|hex]", "asm",
        "Sets the input format to assemble/translate"},
    {NULL, "--hex",         false,  opt_obj_hex,    NULL,   NULL,
        "Produce plaintext hexadecimal file\n"
        "Output file retains the file name except with a .hex extension"},
    {NULL, "--bin",         false,  opt_obj_bin,    NULL,   NULL,
        "Produce plaintext binary file\n"
        "Output file retains the file name except with a .bin extension"},
    {NULL, "--sym",         false,  opt_obj_sym,    NULL,   NULL,
        "Produce a symbol table file\n"
        "Output file retains the file name except with a .sym extension"},
    {NULL, NULL, true, NULL, NULL, NULL, NULL},

};
size_t as_num_options = ARRAY_LEN(as_options);

#ifndef __AS_H__
#define __AS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef enum _as_output_format
{
    AS_OF_OBJ,
    AS_OF_LLF,
    AS_OF_NUM_FORMATS
} as_output_format;

typedef enum _as_syntax
{
    AS_SYNTAX_PATT,
    AS_SYNTAX_LCCC,
    AS_SYNTAX_NUM_SYNTAX
} as_syntax;

typedef struct _as_params
{
    as_output_format    format;
    as_syntax           syntax;
    const char * output_file;
} as_params;

typedef enum _as_ret
{
    AS_RET_OK,
    AS_RET_NO_INPUT,
} as_ret;

typedef struct _as_opt
{
    const char * s_arg; // short arg
    const char * l_arg; // long form
    bool assign;        // is an assignment: looks for =
    int (* func)(as_params * params, int * arg_idx, int argc, char ** args);
    const char * p_str; // help parameter for option
    const char * d_str; // help details for option
} as_opt;

#ifdef __cplusplus
}
#endif

#endif//__AS_H__

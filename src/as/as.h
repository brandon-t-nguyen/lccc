#ifndef __AS_H__
#define __AS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define M_AS "lccc-as"

typedef enum _as_output_format
{
    AS_OF_OBJ,
    AS_OF_LLF,
    AS_OF_NUM_FORMATS
} as_output_format;

typedef enum _as_input_format
{
    AS_IF_ASM,
    AS_IF_HEX,
    AS_IF_BIN,
    AS_IF_NUM_FORMATS
} as_input_format;

typedef enum _as_syntax
{
    AS_SYNTAX_PATT,
    AS_SYNTAX_LCCC,
    AS_SYNTAX_NUM_SYNTAX
} as_syntax;

typedef struct _as_params
{
    as_input_format     iformat;
    as_output_format    oformat;
    as_syntax           syntax;
    const char *        output_file;
    bool                out_hex;
    bool                out_bin;
    bool                out_sym;
    bool                out_lst;
} as_params;

typedef enum _as_ret
{
    AS_RET_OK,
    AS_RET_NO_INPUT,
    AS_RET_OTHER,
} as_ret;

typedef struct _as_opt
{
    const char * s_arg; // short arg
    const char * l_arg; // long form
    bool assign;        // is an assignment: looks for =
    // function to run when this option is used
    // has control to increment arg_idx to consume additional spaced arguments
    // if NULL, this option becomes annotation text for --help
    //      if assign is true, it starts a section of arguments or ends one if
    //      it is already in a section. In the case a section has already started,
    //      d_str will be irrelevant
    int (* func)(as_params * params, const char * arg, const char * assign);
    const char * p_str; // help parameter for option
    const char * pd_str;// default option (NULL if not relevant)
    const char * d_str; // help details for option
} as_opt;

#ifdef __cplusplus
}
#endif

#endif//__AS_H__

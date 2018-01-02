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
    AS_RET_BAD_INPUT,
    AS_RET_OTHER,
} as_ret;

#ifdef __cplusplus
}
#endif

#endif//__AS_H__

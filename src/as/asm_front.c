#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <btn/vector.h>
#include <btn/cstr.h>
#include <btn/bst.h>

#include "as.h"
#include "asm.h"
#include "print.h"

typedef struct match_op
{
    const char * str;
    // provide it an already constructed op
    // if label is true, then tokens[0] is a label: use tokens[1]
    as_ret (* func)(asm_op * op, const asm_source * src, const asm_line * line, bool label);
} match_op;

#define TOKEN(line,idx) ((asm_token *)vector_getp(&(line)->tokens, idx))
#define TOKEN_STR(line,idx) TOKEN(line,idx)->str;
#define NUM_TOKENS(line) (vector_size(&(line)->tokens))

// initialize token iteration
#define TOKEN_IT_INIT() \
    vector_it it;\
    asm_token tok;\
    it_begin(&line->tokens, &it);\
    if (label)\
        it_next(&it, 1);\
    it_read(&it, &tok);

static
as_ret parse_add_and(asm_op * op, const asm_source * src,  const asm_line * line, bool label)
{
    TOKEN_IT_INIT();
    return AS_RET_OK;
}

static
as_ret parse_orig(asm_op * op, const asm_source * src, const asm_line * line, bool label)
{
}

const match_op patt_ops[] =
{
    // instruction mnemonics
    {"add", parse_add_and},
    {"and", parse_add_and},

    // directives
    {".orig", parse_add_and},
};

const match_op lccc_ops[] =
{
};

#define ARRAY_LEN(x) (sizeof(x)/sizeof(x[0]))
// TODO: since one line can have a symbol def AND an instruction
//       maybe this function should directly push an op
static
as_ret parse_line(asm_context * context, asm_program * prog, const asm_line * line)
{
    size_t num_tokens = NUM_TOKENS(line);
    bool label = false;
    const asm_source * src = &prog->src;
    as_ret ret;
    asm_token * token = NULL;
    char * str = NULL;

    asm_op op;
    asm_op_ctor(&op);

    // assume not token
    token = TOKEN(line, 0);
    str = token->str;
    for (size_t i = 0; i < ARRAY_LEN(patt_ops); ++i) {
        if (!strcmp_caseless(patt_ops[i].str, str)) {
            ret = patt_ops[i].func(&op, src, line, false);
            if (ret == AS_RET_OK)
                vector_push_back(&prog->ops, &op);
            else
                asm_op_dtor(&op);
            return ret;
        }
    }

    // LCCC syntax is an extension of Patt
    if (context->params.syntax == AS_SYNTAX_LCCC) {
        for (size_t i = 0; i < ARRAY_LEN(lccc_ops); ++i) {
            if (!strcmp_caseless(lccc_ops[i].str, str)) {
                ret = lccc_ops[i].func(&op, src, line, false);
                if (ret == AS_RET_OK)
                    vector_push_back(&prog->ops, &op);
                else
                    asm_op_dtor(&op);
                return ret;
            }
        }
    }

    // 0th was unsuccesful: try index 1 for a mnemonic
    label = true;
    if (num_tokens == 1) {
        // if only one token, it's probably a symbol declaration
        return AS_RET_OK;
    }

    token = TOKEN(line, 1);
    str = token->str;
    for (size_t i = 0; i < ARRAY_LEN(patt_ops); ++i) {
        if (!strcmp_caseless(patt_ops[i].str, str)) {
            ret = patt_ops[i].func(&op, src, line, false);
            if (ret == AS_RET_OK)
                vector_push_back(&prog->ops, &op);
            else
                asm_op_dtor(&op);
            return ret;
        }
    }

    // LCCC syntax is an extension of Patt
    if (context->params.syntax == AS_SYNTAX_LCCC) {
        for (size_t i = 0; i < ARRAY_LEN(lccc_ops); ++i) {
            if (!strcmp_caseless(lccc_ops[i].str, str)) {
                ret = lccc_ops[i].func(&op, src, line, false);
                if (ret == AS_RET_OK)
                    vector_push_back(&prog->ops, &op);
                else
                    asm_op_dtor(&op);
                return ret;
            }
        }
    }

fail:
    // TODO: predict if token 0 is intended to be a label?
    token = TOKEN(line,0);
    asm_msg_line_token(src, line, token, M_ERROR, "Unknown opcode/directive " ANSI_F_BMAG "'%s'" ANSI_RESET, token->str);
    return AS_RET_BAD_INPUT;
}

as_ret parse_lines(asm_context * context, asm_program * prog)
{
    vector_it line_it;
    it_begin(&prog->src.lines, &line_it);

    asm_line * line;
    as_ret ret = AS_RET_OK;
    while (ret == AS_RET_OK && !it_is_end(&line_it)) {
        line = it_ptr(&line_it);
        ret = parse_line(context, prog, line);
        it_next(&line_it, 1);
    }
    return ret;
}

as_ret asm_front(asm_context * context)
{
    as_ret ret = AS_RET_OK;

    // go through each file, read and tokenize it, and produce assembler ops from it
    int num_files = (int) vector_size(&context->files);
    for (int i = 0; i <  num_files && ret == AS_RET_OK; ++i) {
        asm_program prog;
        asm_program_ctor(&prog);

        // tokenize the file
        FILE * f;
        const char * path;
        vector_get(&context->files, i, &f);
        vector_get(&context->file_paths, i, &path);
        asm_source_read(&prog.src, path, f);

        // go through each line and process it
        ret = parse_lines(context, &prog);
        if (ret != AS_RET_OK)
            break;

        // TODO REMOVE; DEBUG PURPOSES
        /*
        for (int i = 0; i < vector_size(&prog.src.lines); ++i) {
            asm_line * line = vector_getp(&prog.src.lines, i);

            printf("%4d: ", i);
            for (int j = 0; j < vector_size(&line->tokens); ++j) {
                printf("%s ", *(char **) vector_getp(&line->tokens, j));
            }
            printf("\n");
        }
        */
    }

    return ret;
}

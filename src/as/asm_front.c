#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
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
    bool (* func)(asm_op * op, const asm_context * context, const asm_source * src, const asm_line * line, bool label);
} match_op;
#define MATCH_OP(_name) static bool _name(asm_op * op, const asm_context * context, const asm_source * src,  const asm_line * line, bool label)


typedef enum parse_type
{
    PARSE_INV,
    PARSE_DEC,
    PARSE_HEX
} parse_type;

static
parse_type parse_num(const char * str, int * val)
{
    int cnt = 0;
    cnt = sscanf(str, "x%X", val); if (cnt > 0) return PARSE_HEX;
    cnt = sscanf(str, "0x%X", val); if (cnt > 0) return PARSE_HEX;
    cnt = sscanf(str, "x%x", val); if (cnt > 0) return PARSE_HEX;
    cnt = sscanf(str, "0x%x", val); if (cnt > 0) return PARSE_HEX;
    cnt = sscanf(str, "#%d", val); if (cnt > 0) return PARSE_DEC;
    return PARSE_INV;
}

#define INT_BIT (sizeof(int) * CHAR_BIT)
int isoz(int data, int hi, int lo)
{
	int output = ((unsigned int) data) << (INT_BIT - 1 - hi);
	output = (((unsigned int)output) >> (INT_BIT - 1 - hi + lo));
	return output;
}

int isos(int data, int hi, int lo)
{
	int output = data << (INT_BIT - 1 - hi);
    // TODO: more compiler independent way
	output = output >> (INT_BIT - 1 - hi + lo);
	return output;
}

// if bits > 0, then it'll sign extend the value in those bits
static
asm_optype parse_operand(asm_operand * oper,
                         const asm_source * src,
                         const asm_line * line,
                         const asm_token * token,
                         int  bits,
                         bool sign)
{
    parse_type ret;
    oper->token = token;
    ret = parse_num(token->str, &oper->data.imm);
    if (ret != PARSE_INV) {
        // imm
        oper->type = OPERAND_IMM;
        if (bits > 0 && ret == PARSE_HEX) {
            if (oper->data.imm > 0 && sign) {
                // if it's positive and exceeds the bits, don't bother isos:
                if (oper->data.imm >> bits == 0)
                    oper->data.imm = isos(oper->data.imm, bits - 1, 0);
            }
        }
    } else if (sscanf(token->str, "r%u", &oper->data.reg) > 0) {
        // reg
        if (0 <= oper->data.reg && oper->data.reg <= 7) {
            oper->type = OPERAND_REG;
        } else {
            oper->type = OPERAND_INV;
            asm_msg_line_token(src, line, token, M_ERROR,
                               "Register operands must fall between 0 and 7, inclusive");
        }
    } else {
        // string is the catchall
        oper->type = OPERAND_STR;
        oper->data.str = token->str;
    }

    return oper->type;
}

#define TOKEN(line,idx) ((asm_token *)vector_getp(&(line)->tokens, idx))
#define TOKEN_STR(line,idx) TOKEN(line,idx)->str;
#define NUM_TOKENS(line) (vector_size(&(line)->tokens))

// initialize token iteration
#define TOK_IT_INIT()\
    vector_it it;\
    asm_token tok;\
    asm_operand oper;\
    it_begin(&line->tokens, &it);\
    if (label)\
        it_next(&it, 1);\
    it_read(&it, &tok);

#define TOK_IT_NEXT()\
    it_next(&it, 1);\
    if (it_is_end(&it)) {\
        asm_msg_line_token(src, line, &tok, M_ERROR,\
                           "Expected more operands");\
        return false;\
    }\
    it_read(&it, &tok);

#define TOK_OPER_PARSE()\
    if (parse_operand(&oper, src, line, &tok, 0, false) == OPERAND_INV)\
        return false;

// parse with bit isolation in imm
#define TOK_OPER_PARSE_ISO(_bits, _signed)\
    if (parse_operand(&oper, src, line, &tok, _bits, _signed) == OPERAND_INV)\
        return false;

#define TOK_OPER_ASSERT_REG()\
    if (oper.type != OPERAND_REG) {\
        asm_msg_line_token(src, line, &tok, M_ERROR,\
                           "Expected register operand");\
        return false;\
    }

#define TOK_OPER_ASSERT_IMM()\
    if (oper.type != OPERAND_IMM) {\
        asm_msg_line_token(src, line, &tok, M_ERROR,\
                           "Expected immediate/offset operand");\
        return false;\
    }

#define TOK_OPER_ASSERT_IMM_BOUNDS(hi, lo)\
    if (oper.type == OPERAND_IMM &&\
        (oper.data.imm > hi || oper.data.imm < lo)) {\
        asm_msg_line_token(src, line, &tok, M_ERROR,\
                           "Immediate/offset value must be between %d and %d inclusive", lo, hi);\
        return false;\
    }

#define TOK_OPER_PUSH()\
    vector_push_back(&op->operands, &oper);

#define TOK_ASSERT_COMMA()\
    if (strcmp(",", tok.str)) {\
        asm_msg_line_token(src, line, &tok, M_ERROR,\
                           "Expected a comma to separate operands");\
        return false;\
    }

#define TOK_ASSERT_DONE()\
    it_next(&it, 1);\
    if (!it_is_end(&it)) {\
        it_read(&it, &tok);\
        asm_msg_line_token(src, line, &tok, M_ERROR,\
                           "More tokens than expected");\
        return false;\
    }

// ALU
MATCH_OP(parseop_add_and)
{
    TOK_IT_INIT();

    if (!strcmp_caseless("add", tok.str))
        op->asop = OP_ADD;
    else
        op->asop = OP_AND;

    TOK_IT_NEXT(); TOK_OPER_PARSE(); TOK_OPER_ASSERT_REG(); TOK_OPER_PUSH();
    TOK_IT_NEXT(); TOK_ASSERT_COMMA();
    TOK_IT_NEXT(); TOK_OPER_PARSE(); TOK_OPER_ASSERT_REG(); TOK_OPER_PUSH();
    TOK_IT_NEXT(); TOK_ASSERT_COMMA();

    TOK_IT_NEXT(); TOK_OPER_PARSE_ISO(5, true);
    if (oper.type != OPERAND_REG && oper.type != OPERAND_IMM) {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "Expected a register or immediate operand");
        return false;
    }
    TOK_OPER_ASSERT_IMM_BOUNDS(15, -16); TOK_OPER_PUSH();

    TOK_ASSERT_DONE();
    return true;
}

MATCH_OP(parseop_not)
{
    TOK_IT_INIT();

    TOK_IT_NEXT(); TOK_OPER_PARSE(); TOK_OPER_ASSERT_REG(); TOK_OPER_PUSH();
    TOK_IT_NEXT(); TOK_ASSERT_COMMA();
    TOK_IT_NEXT(); TOK_OPER_PARSE(); TOK_OPER_ASSERT_REG(); TOK_OPER_PUSH();

    TOK_ASSERT_DONE();
    return true;
}

// load/store
MATCH_OP(parseop_mem_offset)
{
    TOK_IT_INIT();

    if (!strcmp_caseless("ld", tok.str))
        op->asop = OP_LD;
    else if (!strcmp_caseless("ldi", tok.str))
        op->asop = OP_LDI;
    else if (!strcmp_caseless("st", tok.str))
        op->asop = OP_ST;
    else if (!strcmp_caseless("sti", tok.str))
        op->asop = OP_STI;
    else
        op->asop = OP_LEA;

    TOK_IT_NEXT(); TOK_OPER_PARSE(); TOK_OPER_ASSERT_REG(); TOK_OPER_PUSH();
    TOK_IT_NEXT(); TOK_ASSERT_COMMA();
    TOK_IT_NEXT(); TOK_OPER_PARSE_ISO(9, true);
    if (oper.type != OPERAND_IMM && oper.type != OPERAND_STR) {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "Expected 9-bit offset or symbol/label");
        return false;
    }
    TOK_OPER_ASSERT_IMM_BOUNDS(255, -256); TOK_OPER_PUSH();

    TOK_ASSERT_DONE();
    return true;
}

MATCH_OP(parseop_mem_boffset)
{
    TOK_IT_INIT();

    if (!strcmp_caseless("ldr", tok.str))
        op->asop = OP_LDR;
    else
        op->asop = OP_STR;

    TOK_IT_NEXT(); TOK_OPER_PARSE(); TOK_OPER_ASSERT_REG(); TOK_OPER_PUSH();
    TOK_IT_NEXT(); TOK_ASSERT_COMMA();
    TOK_IT_NEXT(); TOK_OPER_PARSE(); TOK_OPER_ASSERT_REG(); TOK_OPER_PUSH();
    TOK_IT_NEXT(); TOK_ASSERT_COMMA();
    TOK_IT_NEXT(); TOK_OPER_PARSE_ISO(6, true);
    if (oper.type != OPERAND_IMM) {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "Expected 6-bit offset");
        return false;
    }
    TOK_OPER_ASSERT_IMM_BOUNDS(31, -32); TOK_OPER_PUSH();

    TOK_ASSERT_DONE();
    return true;
}

// directives
MATCH_OP(parseop_orig)
{
    TOK_IT_INIT();

    op->asop = OP_ORIG;

    TOK_IT_NEXT();
    TOK_OPER_PARSE();
    if (oper.type != OPERAND_IMM) {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "Operand is not a valid address");
        return false;
    }

    TOK_ASSERT_DONE();
    return true;
}

const match_op patt_ops[] =
{
    // instruction mnemonics
    {"add", parseop_add_and},
    {"and", parseop_add_and},
    {"not", parseop_not},

    {"lea", parseop_mem_offset},
    {"ld", parseop_mem_offset},
    {"ldi", parseop_mem_offset},
    {"ldr", parseop_mem_boffset},
    {"st", parseop_mem_offset},
    {"sti", parseop_mem_offset},
    {"str", parseop_mem_boffset},

    // directives
    {".orig", parseop_orig},
};

const match_op lccc_ops[] =
{
};

#define ARRAY_LEN(x) (sizeof(x)/sizeof(x[0]))

#define CHECK_OPS(_ops)\
    for (size_t i = 0; i < ARRAY_LEN(_ops); ++i) {\
        if (!strcmp_caseless(_ops[i].str, str)) {\
            ret = _ops[i].func(&op, context, src, line, false);\
            if (ret)\
                vector_push_back(&prog->ops, &op);\
            else\
                asm_op_dtor(&op);\
            return ret;\
        }\
    }

static
bool parse_line(asm_context * context, asm_program * prog, const asm_line * line)
{
    size_t num_tokens = NUM_TOKENS(line);
    bool label = false;
    const asm_source * src = &prog->src;
    as_ret ret;
    asm_token * token = NULL;
    char * str = NULL;

    asm_op op;
    asm_op_ctor(&op);
    op.line = line;

    // assume not token
    token = TOKEN(line, 0);
    str = token->str;
    CHECK_OPS(patt_ops);
    CHECK_OPS(lccc_ops); // LCCC is an extension of Patt

    // 0th was unsuccesful: try index 1 for a mnemonic
    label = true;
    if (num_tokens == 1) {
        // if only one token, it's probably a symbol declaration
        return true;
    }

    token = TOKEN(line, 1);
    str = token->str;
    CHECK_OPS(patt_ops);
    CHECK_OPS(lccc_ops); // LCCC is an extension of Patt

fail:
    // TODO: predict if token 0 is intended to be a label?
    token = TOKEN(line,0);
    asm_msg_line_token(src, line, token, M_ERROR, "unknown opcode/directive " ANSI_F_BMAG "'%s'" ANSI_RESET, token->str);
    asm_op_dtor(&op);
    return false;
}

int parse_lines(asm_context * context, asm_program * prog)
{
    vector_it line_it;
    it_begin(&prog->src.lines, &line_it);

    asm_line * line;
    int error_count = 0;
    while (!it_is_end(&line_it)) {
        line = it_ptr(&line_it);

        if (!parse_line(context, prog, line))
            ++error_count;
        it_next(&line_it, 1);
    }
    return error_count;
}

as_ret asm_front(asm_context * context)
{
    as_ret ret = AS_RET_OK;
    asm_program prog;
    int error_count = 0;

    // go through each file, read and tokenize it, and produce assembler ops from it
    int num_files = (int) vector_size(&context->files);
    for (int i = 0; i <  num_files && ret == AS_RET_OK; ++i) {
        asm_program_ctor(&prog);

        // tokenize the file
        FILE * f;
        const char * path;
        vector_get(&context->files, i, &f);
        vector_get(&context->file_paths, i, &path);
        asm_source_read(&prog.src, path, f);

        // go through each line and process it
        error_count += parse_lines(context, &prog);

        vector_push_back(&context->progs, &prog);
    }

    if (error_count > 0) {
        msg(M_AS, M_FATAL, "%d errors found",error_count);
    }
    return ret;
}

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

#define ARRAY_LEN(x) (sizeof(x)/sizeof(x[0]))

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
    oper.token = (const asm_token *) it_ptr(&it);\
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

// generates a function that generates a predefined asm operation
//static bool _name(asm_op * op, const asm_context * context, const asm_source * src,  const asm_line * line, bool label){
#define GEN_OP(_name, _optype, _operand_list)\
    MATCH_OP(_name)\
    {\
        TOK_IT_INIT();\
        op->asop = _optype;\
        asm_operand operands[] = _operand_list;\
        for (size_t i = 0; i < ARRAY_LEN(operands); ++i) {\
            vector_push_back(&op->operands, &operands[i]);\
        }\
        TOK_ASSERT_DONE();\
        return true;\
    }

// protection macro: curly brackets don't escape commas for macros
#define ARRAY(...) {__VA_ARGS__}

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

    op->asop = OP_NOT;
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

// control flow
MATCH_OP(parseop_br)
{
    TOK_IT_INIT();

    op->asop = OP_BR;

    // get condition codes
    oper.type = OPERAND_COND;
    if (!strcmp_caseless("br", tok.str)) {
        oper.data.cond.n = 1; oper.data.cond.z = 1; oper.data.cond.p = 1;
    }
    else {
        oper.data.cond.n = 0; oper.data.cond.z = 0; oper.data.cond.p = 0;
        if (strcfind(tok.str, 'n', 2) != SIZE_MAX)
            oper.data.cond.n = 1;
        if (strcfind(tok.str, 'z', 2) != SIZE_MAX)
            oper.data.cond.z = 1;
        if (strcfind(tok.str, 'p', 2) != SIZE_MAX)
            oper.data.cond.p = 1;
    }
    TOK_OPER_PUSH();

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
static asm_cc nop = {.n = 1, .z = 1, .p = 1};
GEN_OP(parseop_nop, OP_BR, ARRAY({.type = OPERAND_COND, .data.cond = nop}))

MATCH_OP(parseop_jmp_jsrr)
{
    TOK_IT_INIT();

    if (!strcmp_caseless("jmp", tok.str))
        op->asop = OP_JMP;
    else
        op->asop = OP_JSRR;

    TOK_IT_NEXT(); TOK_OPER_PARSE(); TOK_OPER_ASSERT_REG(); TOK_OPER_PUSH();
    TOK_ASSERT_DONE();
    return true;
}

GEN_OP(parseop_ret, OP_JMP, ARRAY({.type = OPERAND_REG, .data.reg = 7}))

MATCH_OP(parseop_jsr)
{
    TOK_IT_INIT();

    op->asop = OP_JSR;

    TOK_IT_NEXT(); TOK_OPER_PARSE_ISO(11, true);
    if (oper.type != OPERAND_IMM && oper.type != OPERAND_STR) {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "Expected 11-bit offset or symbol/label");
        return false;
    }
    TOK_OPER_ASSERT_IMM_BOUNDS(1023, -1024); TOK_OPER_PUSH();

    TOK_ASSERT_DONE();
    return true;
}

MATCH_OP(parseop_trap)
{
    TOK_IT_INIT();

    op->asop = OP_TRAP;

    TOK_IT_NEXT();
    TOK_OPER_PARSE();
    if (oper.type != OPERAND_IMM) {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "Operand is not a valid 8-bit trap vector");
        return false;
    }

    if (oper.data.imm < 0 || oper.data.imm > 255) {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "Operand exceeds the 8-bit trap vector range of 0 to 255");
        return false;
    }
    TOK_OPER_PUSH();

    TOK_ASSERT_DONE();
    return true;
}

MATCH_OP(parseop_trap_mnem)
{
    TOK_IT_INIT();

    op->asop = OP_TRAP;

    oper.type = OPERAND_IMM;
    if (!strcmp_caseless("halt", tok.str))
        oper.data.imm = 0x25;
    else if (!strcmp_caseless("getc", tok.str))
        oper.data.imm = 0x20;
    else if (!strcmp_caseless("out", tok.str))
        oper.data.imm = 0x21;
    else if (!strcmp_caseless("puts", tok.str))
        oper.data.imm = 0x22;
    else if (!strcmp_caseless("in", tok.str))
        oper.data.imm = 0x23;
    else if (!strcmp_caseless("putsp", tok.str))
        oper.data.imm = 0x24;
    TOK_OPER_PUSH();

    TOK_ASSERT_DONE();
    return true;
}


MATCH_OP(parseop_rti)
{
    TOK_IT_INIT();
    op->asop = OP_RTI;
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
    TOK_OPER_PUSH();

    TOK_ASSERT_DONE();
    return true;
}

MATCH_OP(parseop_end)
{
    TOK_IT_INIT();
    op->asop = OP_END;
    TOK_ASSERT_DONE();
    return true;
}

MATCH_OP(parseop_fill)
{
    TOK_IT_INIT();

    op->asop = OP_FILL;

    TOK_IT_NEXT();
    TOK_OPER_PARSE_ISO(16, true);
    if (oper.type != OPERAND_IMM && oper.type != OPERAND_STR) {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "Operand is not a valid 16-bit constant or a symbol/label");
        return false;
    }

    if (oper.type == OPERAND_IMM) {
        if ( (oper.data.imm < 0 && oper.data.imm < -0x8000) ||
             (oper.data.imm > 0 && (oper.data.imm >> 16) != 0)) {
            asm_msg_line_token(src, line, &tok, M_ERROR,
                               "Constant exceeds 16-bits");
        }
    }
    TOK_OPER_PUSH();

    TOK_ASSERT_DONE();
    return true;
}

MATCH_OP(parseop_blkw)
{
    TOK_IT_INIT();

    op->asop = OP_BLKW;

    TOK_IT_NEXT();
    TOK_OPER_PARSE_ISO(16, false);
    if (oper.type != OPERAND_IMM && oper.type != OPERAND_STR) {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "Operand is not a valid number of words to allocate");
        return false;
    }

    if (oper.type == OPERAND_IMM) {
        if ((oper.data.imm > 0xFFFF)) {
            asm_msg_line_token(src, line, &tok, M_ERROR,
                               "Number of words exceeds the 16-bit address space");
            return false;
        }
    }
    TOK_OPER_PUSH();

    TOK_ASSERT_DONE();
    return true;
}

MATCH_OP(parseop_string)
{
    TOK_IT_INIT();

    if (tok.str[7] == 'z' || tok.str[7] == 'Z')
        op->asop = OP_STRINGZ;
    else
        op->asop = OP_STRINGP;

    TOK_IT_NEXT(); TOK_OPER_PARSE();
    if (oper.type != OPERAND_STR) {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "Operand is not a string literal");
        return false;
    }

    size_t len = strlen(oper.data.str);
    if (len < 2 ||
        oper.data.str[0] != '"' ||
        oper.data.str[len - 1] != '"') {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "String literals require a double quote (\") at the beginning and ending");
        return false;
    }

    /*
    // Turns out this is handled at the parsing step
    if (oper.data.str[len - 1] == '"' &&
        oper.data.str[len - 2] == '\\') {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "End quote is escaped with a \\: string literal is unterminated");
        return false;
    }
    */

    TOK_OPER_PUSH();
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

    {"br", parseop_br},
    {"brn", parseop_br},
    {"brz", parseop_br},
    {"brp", parseop_br},
    {"brnz", parseop_br},
    {"brzp", parseop_br},
    {"brnp", parseop_br},
    {"brnzp", parseop_br},
    {"nop", parseop_nop},

    {"jmp", parseop_jmp_jsrr},
    {"ret", parseop_ret},

    {"jsr", parseop_jsr},
    {"jsrr", parseop_jmp_jsrr},

    {"rti", parseop_rti},
    {"trap", parseop_trap},
    {"halt", parseop_trap_mnem},
    {"getc", parseop_trap_mnem},
    {"out", parseop_trap_mnem},
    {"puts", parseop_trap_mnem},
    {"in", parseop_trap_mnem},
    {"putsp", parseop_trap_mnem},

    // directives
    {".orig", parseop_orig},
    {".end", parseop_end},
    {".fill", parseop_fill},
    {".blkw", parseop_blkw},
    {".stringz", parseop_string},
};

const match_op lccc_ops[] =
{
    {".stringp", parseop_string},
};

static
bool validate_symbol(const char * str)
{
    // first character must be alphabetic
    if (!('A' <= str[0] && str[0] <= 'Z') &&
        !('a' <= str[0] && str[0] <= 'z'))
        return false;

    // alphanumeric and '_' are the only allowed symbols
    char c = *str;
    while (c != '\0') {
        if (!('A' <= c && c <= 'Z') &&
            !('a' <= c && c <= 'z') &&
            !('0' <= c && c <= '9') &&
            !(c == '_')
           )
            return false;

        ++str;
        c = *str;
    }

    return true;
}

MATCH_OP(parseop_label)
{
    TOK_IT_INIT();

    op->asop = OP_DEFINE_SYM;

    oper.type = OPERAND_STR;
    oper.data.str = tok.str;
    if (!validate_symbol(oper.data.str)) {
        asm_msg_line_token(src, line, &tok, M_ERROR,
                           "Invalid label '%s': first character must be alphabetic and the rest of characters must be alphanumeric or and underscore ('_')", oper.data.str);
        return false;
    }
    TOK_OPER_PUSH();
    return true;
}

#define CHECK_OPS(_ops)\
    for (size_t i = 0; i < ARRAY_LEN(_ops); ++i) {\
        if (!strcmp_caseless(_ops[i].str, str)) {\
            ret = _ops[i].func(&op, context, src, line, label);\
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
    if (context->params.syntax == AS_SYNTAX_LCCC)
        CHECK_OPS(lccc_ops); // LCCC is an extension of Patt

    // 0th was unsuccesful: try index 1 for a mnemonic
    label = true;
    ret = parseop_label(&op, context, src, line, false);
    if (ret) {
        vector_push_back(&prog->ops, &op);
    } else {
        asm_op_dtor(&op);
        return false;
    }

    if (num_tokens == 1) {
        // if only one token, it's probably a symbol declaration
        return true;
    }

    asm_op_ctor(&op);
    op.line = line;
    token = TOKEN(line, 1);
    str = token->str;
    CHECK_OPS(patt_ops);
    if (context->params.syntax == AS_SYNTAX_LCCC)
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

        // early end if last pushed op is .end in patt syntax
        if (context->params.syntax == AS_SYNTAX_PATT) {
            asm_op * op = (asm_op *) vector_backp(&prog->ops);
            if (op->asop == OP_END)
                break;
        }

        it_next(&line_it, 1);
    }
    return error_count;
}

int asm_front(asm_context * context)
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

    context->error_count += error_count;

    return error_count;
}

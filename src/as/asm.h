#ifndef __ASM_H__
#define __ASM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

#include <btn/vector.h>
#include <btn/cstr.h>
#include <btn/bst.h>

#include "as.h"
#include "print.h"

// these structs help get the source files into
// a workable state

typedef struct _asm_token
{
    char * str; // actual string
    size_t idx; // index in the line it's associated with
} asm_token;

typedef struct _asm_line
{
    unsigned int number;
    vector(asm_token) tokens;
    char * raw;
} asm_line;

void asm_line_ctor(asm_line * line);
void asm_line_dtor(asm_line * line);

typedef struct _asm_source
{
    const char * name;
    vector(asm_line) lines;
} asm_source;

void asm_source_ctor(asm_source * code);
void asm_source_dtor(asm_source * code);

/**
 * Assembler operation
 */
typedef enum _asm_asop
{
    // assembly opcodes
    OP_ADD,
    OP_AND,
    OP_NOT,
    OP_LEA,
    OP_LD,
    OP_LDR,
    OP_LDI,
    OP_ST,
    OP_STR,
    OP_STI,
    OP_BR,
    OP_JMP,
    OP_JSR,
    OP_JSRR,
    OP_TRAP,
    OP_RTI,

    // directives
    OP_ORIG,
    OP_END,
    OP_FILL,
    OP_BLKW,
    OP_STRINGZ,
    OP_STRINGP,

    // special internal ops
    OP_DEFINE_SYM,    // define a local/static symbol
    OP_GLOBAL_SYM,    // specify symbol as exportable
    OP_EXTERN_SYM,    // refer to external symbol

    OP_INVALID
} asm_asop;

/**
 * Assembler operand type
 */
typedef enum _asm_optype
{
    OPERAND_IMM,    // immediate
    OPERAND_REG,    // register
    OPERAND_COND,   // condition code
    OPERAND_STR,    // string
    OPERAND_INV     // invalid operand
} asm_optype;

typedef struct _asm_cc
{
    int n:1;
    int z:1;
    int p:1;
} asm_cc;

// operands are PODs: they don't own data
typedef struct _asm_opdata
{
    int           imm;
    int           reg;
    const char *  str;
    asm_cc  cond;
} asm_opdata;

typedef struct _asm_operand
{
    asm_optype type;
    asm_opdata data;

    const asm_token * token; // for error printing
} asm_operand;

/**
 * Assembler operation
 */
typedef struct _asm_op
{
    asm_asop asop;
    vector(asm_operand) operands;

    const asm_line * line; // for error printing
} asm_op;
void asm_op_ctor(asm_op * op);
void asm_op_dtor(asm_op * op);

/**
 * Symbols are held in a key-val map
 * Keys are char *: this struct is the value
 */
typedef struct _asm_symbol_val
{
    uint16_t addr;

    // for error reporting
    const asm_line * line;
} asm_symbol_val;

typedef struct _asm_symbol_table
{
    bst table;
} asm_symbol_table;
void asm_symbol_table_ctor(asm_symbol_table * table);
void asm_symbol_table_dtor(asm_symbol_table * table);
bool asm_symbol_table_put(asm_symbol_table * table, const char * sym, asm_symbol_val * sym_val);
bool asm_symbol_table_get(asm_symbol_table * table, const char * sym, asm_symbol_val * sym_val);

/**
 * Section are composed of a bunch of operations associated with it
 * Patt assembly has no way to define more segments
 */
typedef struct asm_section
{
    uint16_t addr;
    vector(asm_op) ops;
} asm_section;
void asm_section_ctor(asm_section * section);
void asm_section_dtor(asm_section * section);

typedef struct _asm_program
{
    asm_source src;
    vector(asm_op) ops;     // raw operations: source to these
                            // pass "zero" looks for sectioning/region ops

    uint16_t entry;
    vector(asm_section) sections;


    asm_symbol_table local;
    asm_symbol_table global;    // TODO: implement at some point
} asm_program;
void asm_program_ctor(asm_program * program);
void asm_program_dtor(asm_program * program);

typedef struct _asm_context
{
    as_params           params;
    vector(char *)      file_paths;
    vector(FILE *)      files;
    vector(asm_program) progs;

    int error_count;
} asm_context;
void asm_context_ctor(asm_context * context);
void asm_context_dtor(asm_context * context);

/**
 * Assembler frontend
 */
int asm_front(asm_context * context);

/**
 * Assembler backend: handles emitting code
 */
int asm_back(asm_context * context);

/**
 * Prints out the line, highlighting text and providing an arrow to
 * the place of the line in question
 * Provide -1 to not specify highlighting or a position
 */
void asm_string_error(const char * str,
                      ssize_t hlight_beg, ssize_t hlight_end, ssize_t pos);

void asm_line_error(const asm_line * line,
                    ssize_t hlight_beg, ssize_t hlight_end, ssize_t pos);

/**
 * Helper that can highlight a token
 */
void asm_line_token_error(const asm_line * line, const asm_token * token);

/**
 * Wrapper for a msg print and highlighting a token
 */
void asm_msg_line_token(const asm_source * src,
                        const asm_line * line,
                        const asm_token * token,
                        msg_level level, const char * fmt, ...);

/**
 * Reads an opened file
 * @param[in] code  Source struct to populate
 * @param[in] path  The location of the file
 */
as_ret asm_source_read(asm_source * code, const char * path, FILE * f);

#ifdef __cplusplus
}
#endif

#endif//__ASM_H__

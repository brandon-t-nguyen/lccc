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

#define _(x)\
    case OP_##x:\
        return #x;

const char * asop_str(asm_asop asop) {
    switch (asop) {
    _(ADD)
    _(AND)
    _(NOT)
    _(LEA)
    _(LD)
    _(LDR)
    _(LDI)
    _(ST)
    _(STR)
    _(STI)
    _(BR)
    _(JMP)
    _(JSR)
    _(JSRR)
    _(TRAP)
    _(RTI)
    _(ORIG)
    _(END)
    _(FILL)
    _(BLKW)
    _(STRINGZ)
    _(STRINGP)
    _(DEFINE_SYM)
    _(GLOBAL_SYM)
    _(EXTERN_SYM)
    _(INVALID)
    default:
        return "invalid asop";
    }
}
#undef _

#define GET_OPER(_i) ((asm_operand *) vector_getp(&op->operands, _i))

static
int op_size(const asm_op * op)
{
    if (op->asop == OP_ORIG ||
        op->asop == OP_END  ||
        op->asop == OP_DEFINE_SYM ||
        op->asop == OP_GLOBAL_SYM ||
        op->asop == OP_EXTERN_SYM)
        return 0;

    asm_operand * oper;
    if (op->asop == OP_BLKW) {
        oper = GET_OPER(0);
        return oper->data.imm;
    }

    if (op->asop == OP_STRINGZ ||
        op->asop == OP_STRINGP) {
        oper = GET_OPER(0);

        int len = 0;
        const char * str = oper->data.str;
        char c = *str;
        bool escaped = false;
        while (c != '\0') {
            if (!escaped && c == '\\') {
                escaped = true;
            }
            else if (escaped) {
                escaped = false;
                ++len;
            } else {
                ++len;
            }

            ++str;
            c = *str;
        }
        len = len - 2; // compensate for start and end quote

        if (op->asop == OP_STRINGZ)
            return len + 1; // compensate for null term

        // STRINGP is packed: div by 2
        return (len / 2) + 1; // compensate for packing and the null term
                              // +1 is outside since we round up

    }

    return 1;
}

// consume sectioning commands and populate symbol table
static
int asm_pass_one(asm_context * context, asm_program * prog)
{
    int error_count = 0;

    asm_op * op;
    asm_operand * oper;

    asm_section new_sec; // local section to populate then push
    asm_section * sec = NULL;   // current sectino we're working with

    uint16_t addr;
    size_t num_ops = vector_size(&prog->ops);
    for (size_t i = 0; i < num_ops; ++i) {
        op = (asm_op *) vector_getp(&prog->ops, i);

        // orig: gives us a new code section and sets an entry point
        if (op->asop == OP_ORIG) {
            oper = GET_OPER(0);
            prog->entry = (uint16_t) oper->data.imm;

            asm_section_ctor(&new_sec);
            new_sec.addr = prog->entry;
            vector_push_back(&prog->sections, &new_sec);
            sec = (asm_section *) vector_backp(&prog->sections);

            // set our current address to the section's start address
            addr = sec->addr;
        } else if (op->asop == OP_END) {
            // nop
        } else if (op->asop == OP_DEFINE_SYM) {
            asm_symbol_val sym_val;
            sym_val.addr = addr;
            sym_val.line = op->line;

            oper = GET_OPER(0);
            if (!asm_symbol_table_put(&prog->local, oper->data.str, &sym_val)) {
                asm_symbol_val old_sym_val;
                asm_symbol_table_get(&prog->local, oper->data.str, &old_sym_val);

                asm_msg_line_token(&prog->src, op->line, oper->token, M_ERROR,
                                   "Symbol '%s' previously declared at line %d", oper->data.str, old_sym_val.line->number);
                ++error_count;
            }
        } else {
            // copy the op to the section
            if (sec == NULL) {
                    asm_msg_line(&prog->src,
                                 op->line,
                                 M_ERROR, "Section has not been specified: have you used a sectioning directive?");

                ++error_count;
                break;
            }

            vector_push_back(&sec->ops, op);
            addr += op_size(op);
        }
    }

    return error_count;
}

// helpers for assembly functions
static inline
uint16_t section_next(asm_section * section)
{
    return section->addr + (uint16_t) vector_size(&section->code);
}

// returns false if unable to find symbol
static
bool symbol_pcoff(asm_program * prog, asm_section * sec, const char * sym, int * off)
{
    int pc = section_next(sec) + 1;
    asm_symbol_val sym_val;
    if (!asm_symbol_table_get(&prog->local, sym, &sym_val)) {
        return false;
    }

    int target = sym_val.addr;
    *off = (target - pc);
    return true;
}

// shorthand macro for the function signature
#define ASOP(_name) static bool _name(asm_context * context, asm_program * prog, asm_section * sec, asm_op * op)
#define ASOP_OPER(_i) ((asm_operand *) vector_getp(&op->operands, _i))
#define ASOP_INIT(_num_oper)\
    uint16_t inst = 0x0000;\
    int pcoff = 0;\
    asm_operand * oper[_num_oper];\
    for (int i = 0; i < _num_oper; ++i) {\
        oper[i] = ASOP_OPER(i);\
    }

#define ASOP_PUSH_INST()\
    if (section_next(sec) == 0xFFFF) {\
        asm_msg_line(&prog->src, op->line, M_ERROR, "Instruction/directive will run past end of address space");\
        return false;\
    } else {\
        vector_push_back(&sec->code, &inst);\
        inst = 0x0000;\
    }

#define ASOP_CALC_PCOFF(_oper_num, _bits) {\
    if (oper[_oper_num]->type == OPERAND_IMM) {\
        pcoff = oper[_oper_num]->data.imm;\
    } else {\
        const char * sym = oper[_oper_num]->data.str;\
        if (!symbol_pcoff(prog, sec, sym, &pcoff)) {\
            asm_msg_line_token(&prog->src, op->line, oper[_oper_num]->token, M_ERROR,\
                               "Undeclared symbol '%s'", sym);\
            return false;\
        }\
    }\
    int lo = (~0) << _bits;\
    int hi = (1) << _bits;\
    if (pcoff < lo || hi < pcoff) {\
        asm_msg_line_token(&prog->src, op->line, oper[_oper_num]->token, M_ERROR,\
                           "Resulting PC-offset will be %d, exceeding %d bits", pcoff, _bits);\
        return false;\
    }\
}

#define ASOP_SET_OPCODE(_opcode) inst |= (_opcode & 0xF) << 12
#define ASOP_SET_REG(_reg, _off) inst |= (_reg & 0x7) << _off
#define ASOP_SET_DR(_dr) ASOP_SET_REG(_dr, 9)
#define ASOP_SET_SR1(_sr1) ASOP_SET_REG(_sr1, 6)
#define ASOP_SET_SR2(_sr2) ASOP_SET_REG(_sr2, 0)
#define ASOP_SET_IMM(_val) inst |= (0x1 << 5) | (_val & 0x1F);
#define ASOP_SET_PCOFF9(_val) inst |= (_val & 0x1FF);
#define ASOP_SET_PCOFF11(_val) inst |= (_val & 0x7FF);
#define ASOP_SET_TRAPVEC(_val) inst |= (_val & 0xFF);

ASOP(asop_add_and)
{
    ASOP_INIT(3);

    int opcode = op->asop == OP_ADD ? 0x1 : 0x5;
    ASOP_SET_OPCODE(opcode);
    ASOP_SET_DR(oper[0]->data.reg);
    ASOP_SET_SR1(oper[1]->data.reg);

    if (oper[2]->type == OPERAND_IMM) {
        ASOP_SET_IMM(oper[2]->data.imm);
    } else {
        ASOP_SET_SR2(oper[2]->data.reg);
    }
    ASOP_PUSH_INST();
    return true;
}

ASOP(asop_not)
{
    ASOP_INIT(1);
    ASOP_SET_OPCODE(0x9);
    ASOP_SET_DR(oper[0]->data.reg);
    ASOP_SET_SR1(oper[1]->data.reg);
    inst |= 0x3F;
    ASOP_PUSH_INST();
    return true;
}

ASOP(asop_mem_pcoff)
{
    ASOP_INIT(2);
    int opcode;
    switch (op->asop) {
    case OP_LEA:
        opcode = 0xE;
        break;
    case OP_LD:
        opcode = 0x2;
        break;
    case OP_LDI:
        opcode = 0xA;
        break;
    case OP_ST:
        opcode = 0x3;
        break;
    case OP_STI:
        opcode = 0xB;
        break;
    default:
        break;
    }
    ASOP_SET_OPCODE(opcode);
    ASOP_SET_DR(oper[0]->data.reg);

    ASOP_CALC_PCOFF(1, 9);
    ASOP_SET_PCOFF9(pcoff);

    ASOP_PUSH_INST();
    return true;
}

ASOP(asop_br)
{
    ASOP_INIT(2);
    ASOP_SET_OPCODE(0x0);
    ASOP_CALC_PCOFF(1, 9);
    ASOP_SET_PCOFF9(pcoff);

    // set condition codes
    if (oper[0]->data.cond.n) inst |= 1 << 11;
    if (oper[0]->data.cond.z) inst |= 1 << 10;
    if (oper[0]->data.cond.p) inst |= 1 << 9;

    ASOP_PUSH_INST();
    return true;
}

ASOP(asop_jmp_jsrr)
{
    ASOP_INIT(1);

    int opcode = op->asop == OP_JMP ? 0xC : 0x4;
    ASOP_SET_OPCODE(opcode);
    ASOP_CALC_PCOFF(0, 9);
    ASOP_SET_SR1(oper[0]->data.reg);
    ASOP_PUSH_INST();
    return true;
}

ASOP(asop_jsr)
{
    ASOP_INIT(1);
    ASOP_SET_OPCODE(0x4);
    ASOP_CALC_PCOFF(0, 11);
    ASOP_SET_PCOFF11(pcoff);
    inst |= 1 << 11;
    ASOP_PUSH_INST();
    return true;
}

ASOP(asop_trap)
{
    ASOP_INIT(1);
    ASOP_SET_OPCODE(0xF);
    ASOP_SET_TRAPVEC(oper[0]->data.imm);
    ASOP_PUSH_INST();
    return true;
}

ASOP(asop_rti)
{
    ASOP_INIT(0);
    inst = 0x8000;
    ASOP_PUSH_INST();
    return true;
}

ASOP(asop_fill)
{
    ASOP_INIT(1);
    inst = oper[0]->data.imm;
    ASOP_PUSH_INST();
    return true;
}

ASOP(asop_blkw)
{
    ASOP_INIT(1);
    int words = oper[0]->data.imm;
    for (int i = 0; i < words; ++i) {
        inst = 0;
        ASOP_PUSH_INST();
    }
    return true;
}

ASOP(asop_stringz)
{
    ASOP_INIT(1);
    const char * str = oper[0]->data.str;

    ++str; // skip the start quote
    bool escaped = false;
    char c = *str;
    while (!(!escaped && c == '"')) {
        if (!escaped && c == '\\') {
            escaped = true;
        } else {
            if (escaped && c == 'r')
                c = '\r';
            else if (escaped && c == 'n')
                c = '\n';
            else if (escaped && c == 't')
                c = '\t';
            inst = c & 0xFF;
            ASOP_PUSH_INST();
            escaped = false;
        }

        ++str;
        c = *str;
    }

    inst = 0;
    ASOP_PUSH_INST();
    return true;
}

#define ENTRY(_asop, _func)\
    case _asop:\
        return _func(context, prog, sec, op);

static
bool asm_assemble(asm_context * context, asm_program * prog, asm_section * sec, asm_op * op)
{
    switch(op->asop) {
    ENTRY(OP_ADD, asop_add_and)
    ENTRY(OP_AND, asop_add_and)
    ENTRY(OP_NOT, asop_not)
    ENTRY(OP_LEA, asop_mem_pcoff)
    ENTRY(OP_LD,  asop_mem_pcoff)
    ENTRY(OP_LDI, asop_mem_pcoff)
    ENTRY(OP_ST,  asop_mem_pcoff)
    ENTRY(OP_STI, asop_mem_pcoff)
    ENTRY(OP_BR,  asop_br)
    ENTRY(OP_JMP,  asop_jmp_jsrr)
    ENTRY(OP_JSRR,  asop_jmp_jsrr)
    ENTRY(OP_JSR,  asop_jsr)
    ENTRY(OP_TRAP,  asop_trap)
    ENTRY(OP_RTI,  asop_rti)
    ENTRY(OP_FILL,  asop_fill)
    ENTRY(OP_BLKW,  asop_blkw)
    ENTRY(OP_STRINGZ,  asop_stringz)
    default:
        msg(M_AS, M_WARN, "Op %s not implemented", asop_str(op->asop));
        break;
    }
    return true;
}

// go through sections and assemble the code
static
int asm_pass_two(asm_context * context, asm_program * prog)
{
    int error_count = 0;

    asm_section * sec;
    size_t num_secs = vector_size(&prog->sections);
    for (size_t i = 0; i < num_secs; ++i) {
        sec = (asm_section *) vector_getp(&prog->sections, i);
        asm_op * op;
        size_t num_ops = vector_size(&sec->ops);
        for (size_t j = 0; j < num_ops; ++j) {
            op = (asm_op *) vector_getp(&sec->ops, j);
            if (!asm_assemble(context, prog, sec, op)) {
                ++error_count;
            }
        }
    }

    return error_count;
}

int asm_back(asm_context * context)
{
    as_ret ret = AS_RET_OK;
    int error_count = 0;

    // go through each program and parse
    int num_progs = (int) vector_size(&context->progs);
    for (int i = 0; i <  num_progs && ret == AS_RET_OK; ++i) {
        asm_program * prog = (asm_program *) vector_getp(&context->progs, i);
        error_count += asm_pass_one(context, prog);
        error_count += asm_pass_two(context, prog);
    }
    context->error_count += error_count;

    return error_count;
}

#ifndef __CODETREE_H__
#define __CODETREE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <btn/ds/vvector.h>

#include "as.h"
//#include "symbol.h"

struct CodeNode_str;
typedef struct CodeNode_str CodeNode;

struct OperandNode_str;
typedef struct OperandNode_str OperandNode;

typedef enum OperandType_enum
{
    OperandType_Register,
    OperandType_Immediate,
    OperandType_String,
    OperandType_Symbol
}

struct CodeNode_str
{
    int32_t offset;
    int32_t length;
    int lineNumber;
    const AsmOpProp * prop
    CodeNode * prev;
    CodeNode * next;
    VVector(OperandNode*) operands;
};

typedef union OperandValue_union
{
    int32_t integer;
    char *  string;
} OperandValue;

struct OperandNode_str
{
    OperandType type;
    OperandValue value;
};

CodeNode * CodeNode_new( CodeNode * prev, const AsmOpProp * prop );
void CodeNode_delete( CodeNode * thiz );

#ifdef __cplusplus
}
#endif

#endif//__CODETREE_H__

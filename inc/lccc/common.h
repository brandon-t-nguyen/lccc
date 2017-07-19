#ifndef __LCCC_COMMON_H__
#define __LCCC_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum BinaryFormat_enum
{
    LCCC_obj,   // traditional obj format: big-endian, single section
    LCCC_llf,   // "little linking format" simple linking format a la ELF
} BinaryFormat;

#ifdef __cplusplus
}
#endif

#endif//__LCCC_COMMON_H__

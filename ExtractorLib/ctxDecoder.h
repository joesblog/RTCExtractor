#pragma once

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "IDAtypes.h"

#include "MemStream.h"
/* Exact workspace as the original used: */
typedef struct RawCtx {
  /* 0x00 */ uint32_t outTarget;   // *ths
  /* 0x04 */ uint32_t unk1;
  /* 0x08 */ uint32_t unk2;
  /* 0x0C */ uint32_t outPos;      // *((DWORD*)ths + 3)
  /* 0x10 */ uint32_t inPos;       // *((DWORD*)ths + 4)
  /* 0x14 */ char* outBuf;      // *((DWORD*)ths + 5)
  /* 0x18 */ char* inBuf; // *((DWORD*)ths + 6)
  /* 0x1C */ uint32_t inSize;      // *((DWORD*)ths + 7)
  /* 0x20.. */ uint8_t scratch[66376 - 32];  // everything else, including window & coder state
} RawCtx;
#define LZAR_WORKSPACE_BYTES 66376
/* helpers for absolute offsets (all relative to ctx base) */
#define OFF(ctx,off)       (*(uint8_t*)((uint8_t*)(ctx) + (off)))
#define PTR8(ctx,off)      ((uint8_t*)((uint8_t*)(ctx) + (off)))
#define U16(ctx,off)       (*(uint16_t*)((uint8_t*)(ctx) + (off)))
#define U32(ctx,off)       (*(uint32_t*)((uint8_t*)(ctx) + (off)))

/* Coder registers at fixed offsets */
#define BITREG(ctx)        U16(ctx, 66358)
#define BITMASK(ctx)       U16(ctx, 66360)
#define LOW(ctx)           U32(ctx, 66364)
#define HIGH(ctx)          U32(ctx, 66368)
#define CODE(ctx)          U32(ctx, 66372)

/* Literal/len model base (from your decomp): */
#define LIT_CUM_LO(ctx,i)  U16(ctx, 57532 + 2*(i))   /* cum low  */
#define LIT_CUM_HI(ctx,i)  U16(ctx, 57534 + 2*(i))   /* cum high */
#define WIN_BYTE(ctx,idx)  OFF(ctx, 32 + ((idx) & 0x0FFF)) /* window base +32 */



/* Literal/length model arrays around 57532/57534 */
#define LIT_CUM_LO(ctx,i)  U16(ctx, 57532 + 2*(i))   /* cum low  */
#define LIT_CUM_HI(ctx,i)  U16(ctx, 57534 + 2*(i))   /* cum high */
#define LIT_SYM_MAP(ctx,i) U32(ctx, 55644 + 4*(i))   /* symbol map -> decoded value */



/* Distance model cumulative starts around 58164 in your code: */
#define DIST_TOTAL(ctx)    U16(ctx, 58164)            /* total (also part of cum array) */
#define DIST_CUM(ctx,i)    U16(ctx, 58164 + 2*(i))    /* cum[] boundaries */


uint32_t CTX_DecompresFromMemoryStream( MemStream* src, char** outData, uint32_t* outSize);
char* CTX_Decompress(RawCtx * c, char* src, uint32_t srcSize, uint32_t * outSize );
bool  CTX_ReadOutpusizeLE(RawCtx* ths);
char CTX_DecodeStream(RawCtx* c);
int  CTX_ResetCoderState(RawCtx* ths, int a2);
BOOL   CTX_PrimeCoderFromBits(RawCtx* ths);
BOOL CTX_ReadBit(RawCtx* ths);
int16   CTX_InitModels(RawCtx* ths);
int   CTX_DecodeLitLenSymbol(RawCtx* ths);
int   CTX_LitLenLowerBound(RawCtx* ths, uint16 a2);
int   CTX_DecodeMatchDistance(RawCtx* ths);
int   CTX_DistUpperBound(RawCtx* ths, uint16 a2);

int   CTX_EmitByte(RawCtx* ths, char a2);

int  CTX_UpdateLitLenModel(RawCtx* ths, int a2);

/*
| Old name   | New name                 |
| ---------- | ------------------------ |
| sub_47D480 | LZAR_Decompress          |
| sub_47D510 | LZAR_ResetCoderState     |
| sub_47D560 | LZAR_EmitByte            |
| sub_47D5A0 | LZAR_ReadOutputSizeLE    |
| sub_47D600 | LZAR_InitModels          |
| sub_47D680 | LZAR_UpdateLitLenModel   |
| sub_47D770 | LZAR_ReadBit             |
| sub_47D7D0 | LZAR_LitLenLowerBound    |
| sub_47D810 | LZAR_DistUpperBound      |
| sub_47D850 | LZAR_PrimeCodeFromBits   |
| sub_47D880 | LZAR_DecodeLitLenSymbol  |
| sub_47D9D0 | LZAR_DecodeMatchDistance |
| sub_47DB10 | LZAR_DecodeStream        |

*/
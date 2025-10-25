#pragma once
#include <stdint.h>
#include <string.h>

enum { UTIL_TAG_CAP = 100, ALLOC_NODE_CAP = 100 };


#define SET_LOWORD(a, value) ((a) = ((a) & 0xFFFF0000) | (value))
#define SET_HIWORD(a, value) ((a) = ((a) & 0x0000FFFF) | ((value) << 16))


/* ---- byte getters (32-bit value) ---- */
#define LOBYTE(x)   ((uint8_t)((uint32_t)(x)        & 0xFFu))
#define BYTE1(x)    ((uint8_t)(((uint32_t)(x) >> 8)  & 0xFFu))
#define BYTE2(x)    ((uint8_t)(((uint32_t)(x) >> 16) & 0xFFu))
#define HIBYTE(x)   ((uint8_t)(((uint32_t)(x) >> 24) & 0xFFu))

/* Generic getter (0..3 for 32-bit) */
#define BYTEN(x,n)  ((uint8_t)(((uint32_t)(x) >> (8u*(uint32_t)(n))) & 0xFFu))

/* ---- byte setters (modify an lvalue 32-bit integer) ---- */
#define SET_LOBYTE(x, b) \
    do { (x) = (uint32_t)((uint32_t)(x) & ~0xFFu) | (uint32_t)((uint8_t)(b)); } while (0)

#define SET_BYTE1(x, b) \
    do { (x) = (uint32_t)((uint32_t)(x) & ~(0xFFu << 8))  | ((uint32_t)(uint8_t)(b) << 8); } while (0)

#define SET_BYTE2(x, b) \
    do { (x) = (uint32_t)((uint32_t)(x) & ~(0xFFu << 16)) | ((uint32_t)(uint8_t)(b) << 16); } while (0)

#define SET_HIBYTE(x, b) \
    do { (x) = (uint32_t)((uint32_t)(x) & ~(0xFFu << 24)) | ((uint32_t)(uint8_t)(b) << 24); } while (0)

/* Generic setter (n = 0..3 for 32-bit) */
#define SET_BYTEN(x, n, b)                                                     \
    do {                                                                       \
        uint32_t __mask = (uint32_t)0xFFu << (8u*(uint32_t)(n));               \
        (x) = (uint32_t)((uint32_t)(x) & ~__mask)                              \
             | (uint32_t)(((uint32_t)(uint8_t)(b)) << (8u*(uint32_t)(n)));     \
    } while (0)


typedef struct AllocNode
{
  struct AllocNode* next;
  void* buffer;
  size_t size;
  const char* tagOne;
  const char* tagTwo;
  int paramA6;
} AllocNode;

typedef struct UtilBlockHeader
{
	struct UtilBlockHdr* next;
	void* user;
	uint32_t size;
}UtilBlockHeader;


extern UtilBlockHeader* g_tagLists[UTIL_TAG_CAP];
extern AllocNode* g_AllocNodes[ALLOC_NODE_CAP];
extern int g_size_5b47b8;
void Util_Free(char* buffer, char tag);
UtilBlockHeader* Util_FreeFindBlock(void* userPtr, uint8_t tag);
int  Util_Malloc(void** Buffer, size_t Size, char a3, char tagOne, char tagTwo, int a6);
void Util_StrAlloc(void** toAlloc, const char* stringToAlloc, int nextNodeIdx );
void Util_Malloc2(void** buffer, size_t size, int nextNodeIdx);

void __cdecl Util_StrAlloc_47ABA0(
  void** toAlloc,
  const char* stringToAlloc,
  char a3,
  char* strSource,
  const char* StrTag,
  int a6);

void  Util_Free_47A7A0(char* a1, char a2);
int  UtilFreeCheckBlock_47A8A0(int a1);
int  UtilFreeContents_47A930(char* a1, int a2);
void   UtilFreeBlock_47A990(char* a1);


#pragma once

#include <stdint.h>
#include <string.h>
#include <memory.h>
//#include "gameData.h"
 
typedef struct MemStream
{
	uint32_t _type_or_flags;
	char* buf;
	uint32_t size;
	uint32_t pos;
	uint32_t capacity;
} MemStream;


MemStream* MemStream_Constructor(MemStream* ths);
void MemStream_CreateBlank(MemStream* ths, size_t Size);

void MemStream_CreateFromMem(MemStream* ths, char* buffer, int fileSize);
uint32_t MemStream_StreamSeek(MemStream* ths, int offset);
char* MemStream_StreamTake(MemStream* ths, int count);
void MemStream_Destroy(MemStream* s);
void MemStream_Delete(MemStream* s);



uint32_t MemStream_DecompressSectionData(MemStream* s);

char* MemStream_getFirstOfString(MemStream* ths);


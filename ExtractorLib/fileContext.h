#pragma once

#include <stdint.h>
#include <string.h>
#include "MemStream.h"
#include "util.h"

typedef struct FileContext
{
	char* vptr;
	char* header;
	int sectionCount;
	MemStream* secStream[100];
	char* sectName[100];
}FileContext;
#define GSDATASIZE 1000000
extern char* gsdata[GSDATASIZE];
//gsData +4 = Header string
char* FileContext_getString(MemStream* s);
int FileContext_getInt(MemStream* ths);
void FileContext_Init(FileContext* f);
int FileContext_Load(FileContext* t, const char* FileName);
MemStream* FileContext_FindSection(FileContext* fc, const char* name);

int FileContext_ModuleIdFromHeader(const char * a1);
const char* FileContext_GetHeaderString(FileContext* ths);


void u4820a0(FileContext* f, int* t, MemStream* sec, int vid);
 
typedef struct RegLike {
 
  unsigned char pad_00_37[56];
  uint32_t field_38;   // or void* / uintptr_t if it's a pointer
} RegLike;

typedef struct GD {
 
  unsigned char data[GSDATASIZE];
} GD;
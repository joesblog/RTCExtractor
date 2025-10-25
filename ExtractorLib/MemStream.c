#include "MemStream.h"
#include "util.h"
#include <memory.h>
//#include "decoder.h"
#include "ctxDecoder.h"
void MemStream_CreateFromMem(MemStream* ths, char* buffer, int Size)
{
	char** p_buf;
	char* buf;
	if (!ths->size)
	{
		buf = (char*)ths->buf;
		p_buf = &ths->buf;
		if (buf)
		{
			Util_Free(buf, 0);
			*p_buf = 0;
		}
	}

	//Util_Malloc(&ths->buf, Size, 0);
	
	ths->buf = malloc(Size);

	

	memcpy(ths->buf, buffer, Size);
	ths->size = Size;
	ths->capacity = Size;
	MemStream_StreamSeek(ths, 0);
}

uint32_t MemStream_StreamSeek(MemStream* ths, int offset)
{
	uint32_t res;

	res = (uint32_t)ths->buf + offset;
	ths->pos = res;
	return res;
}

MemStream* MemStream_Constructor(MemStream* ths)
{
	ths->_type_or_flags = 0; //is normally in off_4DB4EC
	ths->buf = 0;
	ths->size = 0;
	MemStream_CreateBlank(ths, 1000);//3E8u
	return ths;
}
void MemStream_CreateBlank(MemStream* ths, size_t Size)
{
	char* buf;
	if (!ths->size)
	{
		buf = (char*)ths->buf;
		if (buf)
		{
			Util_Free(buf, 0);
			ths->buf = 0;
		}

		//Util_Malloc(&ths->buf, Size, 0);
		Util_Malloc(&ths->buf, Size, 0, "utilMemBlock", "CreateBlank", 98);
		ths->size = 0;
		ths->capacity = Size;
		MemStream_StreamSeek(ths, 0);
	}
}

char* MemStream_StreamTake(MemStream* ths, int count)
{
	char* result;
	result = (char*)ths->pos;
	ths->pos = result + count;
	return result;
}


void MemStream_Destroy(MemStream* s)
{
	if (!s) return;

	if (s->buf) {
		free(s->buf);          // or Util_Free(s->buf, 0), not sure if i cba to get that converted;
		s->buf = NULL;
	}
	s->size = 0;
	s->pos = 0;
	s->capacity = 0;
	s->_type_or_flags = 0;
}
void MemStream_Delete(MemStream* s)
{
	if (!s) return;
	MemStream_Destroy(s);
	free(s);
}


uint32_t MemStream_DecompressSectionData(MemStream* s)
{
int *p_size = &s->size;
char* v3;
uint32_t v4;
char Buffer[1000];
printf("Source Size: %i",s->size);

char* out = NULL;
uint32_t outLen = 0;

CTX_DecompresFromMemoryStream(s,&out, &outLen);

MemStream_StreamSeek(s,0);
return 1;
}
 
char*   MemStream_getFirstOfString(MemStream* ths)
{
	char* result; // eax
	int v2; // edx

	result = (char*)ths->pos;
	v2 = 0;
	if (*result)
	{
		while (result[++v2])
			;
	}
	ths->pos = (uint32_t)&result[v2 + 1];
	return result;
}
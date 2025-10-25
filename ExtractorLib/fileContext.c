

#include "fileContext.h"
#include "rtcConsts.h"
#include "IDAtypes.h"

char* gsdata[GSDATASIZE];

int FileContext_Load(FileContext* t, const char* FileName)
{

	MemStream mainStream;
	//MemStream** secStream;
	char* fileBuf;
	int fileSize = 0;
	int decryptResult = 0;
	char* headerString;

	int sectionCount;
	char** sectName;

	char sizesBuf[400];
	int* psizeBuf;
	//MemStream* secStream;
	FileContext* dbg = t;

	MemStream_Constructor(&mainStream);
	decryptResult = decode_ReadAndDecryptFile(FileName, &fileBuf, &fileSize);
	if (decryptResult) //swapped logic from original
	{
		MemStream_CreateFromMem(&mainStream, fileBuf, fileSize);

		headerString = FileContext_getString(&mainStream);
		Util_StrAlloc(&t->header, headerString, 0);
		sectionCount = FileContext_getInt(&mainStream);

		int iSC = 0; //v6
		char* sectString;

		//get Section Nmaes
		t->sectionCount = sectionCount;
		if (sectionCount > 0)
		{
			sectName = (char**)t->sectName;
			do {
				sectString = FileContext_getString(&mainStream);
				Util_StrAlloc(sectName, sectString, 0);
				++sectName;
				iSC++;
			} while (iSC < t->sectionCount);
		}
		//section Sizes
		iSC = 0;
		if (t->sectionCount > 0)
		{
			psizeBuf = (int*)sizesBuf;

			do
			{
				*psizeBuf = FileContext_getInt(&mainStream);
				++iSC;
				++psizeBuf;
			} while (iSC < t->sectionCount);
		}
		//section Streams
		iSC = 0;

		if (t->sectionCount > 0)
		{
			const uint32_t* psizeBuf = (const uint32_t*)sizesBuf;  // make sizesBuf uint32_t[100]
			MemStream** secStream = t->secStream;

			do {
				uint32_t curSizeBuf = *psizeBuf;
				if (curSizeBuf == 0) { *secStream = NULL; goto next; }

				const uint8_t* takeResult = MemStream_StreamTake(&mainStream, curSizeBuf);
				if (!takeResult) { /* handle truncated */ break; }

				MemStream* ms = (MemStream*)calloc(1, sizeof(MemStream));
				if (!ms) { /* handle OOM */ break; }

				ms = MemStream_Constructor(ms);
				if (!ms) { free(ms); /* handle ctor fail */ break; }

				MemStream_CreateFromMem(ms, takeResult, curSizeBuf);
				*secStream = ms;

			next:
				++iSC;
				++psizeBuf;
				++secStream;
			} while (iSC < t->sectionCount);
		}
		MemStream_Destroy(&mainStream);
		free(fileBuf);
	}

	int r = 0;


	return r;
}


void FileContext_Init(FileContext* f)
{
	FileContext* result;
	int v2;
	char** sectName;

	result = f;
	v2 = 100;
	//vptr here

	result->sectionCount = 0;
	sectName = result->sectName;

	do {
		*(sectName - 100) = 0;
		*sectName++ = 0;
		--v2;
	} while (v2);
	result->header = 0;
	return result;
}
void s47aba0()
{

}
int FileContext_getInt(MemStream* ths)
{
	char* pos; // eax
	char v2; // dl
	char v3; // dl
	char v4; // dl
	int v6; // [esp+0h] [ebp-4h]

	pos = (char*)ths->pos;
	v2 = *pos++;
	//LOBYTE(v6) = (_BYTE)v2;
	SET_LOBYTE(v6, v2);
	ths->pos = (uint32_t)pos;
	v3 = *pos++;
	//BYTE1(v6) = v3;
	SET_BYTE1(v6, v3);
	ths->pos = (uint32_t)pos;
	v4 = *pos++;
	//BYTE2(v6) = v4;
	SET_BYTE2(v6, v4);
	ths->pos = (uint32_t)pos;
	//HIBYTE(v6) = *pos;
	SET_HIBYTE(v6, *pos);
	ths->pos = (uint32_t)(pos + 1);
	return v6;
}



char* FileContext_getString(MemStream* s)
{
	char* result = NULL;
	int v2 = 0;

	result = (char*)s->pos;
	if (*result)
	{
		while (result[++v2]);
	}

	s->pos = (uint32_t)&result[v2 + 1];
	return result;
}


MemStream* FileContext_FindSection(FileContext* fc, const char* name)
{
	 
	int v2 = 0;
	char** i;


	v2 = 0;
	for (i = fc->sectName; !*(i - 100) || strcmp(*i, name); ++i)
	{
		if (++v2 >= 100)
			return 0;
	}

	return fc->secStream[v2];

}

const char* FileContext_GetHeaderString(FileContext* ths)
{
	return ths->header;
}


int FileContext_ModuleIdFromHeader(const char* a1)
{
	int v1, v2, result;
	//Future Joe: i should really not be so lazy and make this into a switch statement!
	v1 = -1;
	if (!strcmp(a1, aRtcmv22))
		v1 = 22;
	if (!strcmp(a1, aRtcsv22))
		v1 = 22;
	if (!strcmp(a1, aRtcmv27))
		v1 = 27;
	if (!strcmp(a1, aRtcsv27))
		v1 = 27;
	if (!strcmp(a1, aRtcmv29))
		v1 = 29;
	if (!strcmp(a1, aRtcsv29))
		v1 = 29;
	if (!strcmp(a1, aRtcmv33))
		v1 = 33;
	if (!strcmp(a1, aRtcsv33))
		v1 = 33;
	if (!strcmp(a1, aRtcmv35))
		v1 = 35;
	if (!strcmp(a1, aRtcsv35))
		v1 = 35;
	if (!strcmp(a1, aRtcmv36))
		v1 = 36;
	if (!strcmp(a1, aRtcsv36))
		v1 = 36;
	if (!strcmp(a1, aRtcmv37))
		v1 = 37;
	if (!strcmp(a1, aRtcsv37))
		v1 = 37;
	if (!strcmp(a1, aRtcmv38))
		v1 = 38;
	if (!strcmp(a1, aRtcsv38))
		v1 = 38;
	if (!strcmp(a1, aRtcmv39))
		v1 = 39;
	if (!strcmp(a1, aRtcsv39))
		v1 = 39;
	if (!strcmp(a1, aRtcmv40))
		v1 = 40;
	if (!strcmp(a1, aRtcsv40))
		v1 = 40;
	if (!strcmp(a1, aRtcmv41))
		v1 = 41;
	if (!strcmp(a1, aRtcsv41))
		v1 = 41;
	if (!strcmp(a1, aRtcmv42))
		v1 = 42;
	if (!strcmp(a1, aRtcsv42))
		v1 = 42;
	if (!strcmp(a1, aRtcmv43))
		v1 = 43;
	if (!strcmp(a1, aRtcsv43))
		v1 = 43;
	if (!strcmp(a1, aRtcmv44))
		v1 = 44;
	if (!strcmp(a1, aRtcsv44))
		v1 = 44;
	if (!strcmp(a1, aRtcmv45))
		v1 = 45;
	if (!strcmp(a1, aRtcsv45))
		v1 = 45;
	if (!strcmp(a1, aRtcmv46))
		v1 = 46;
	if (!strcmp(a1, aRtcsv46))
		v1 = 46;
	if (!strcmp(a1, aRtcmv47))
		v1 = 47;
	if (!strcmp(a1, aRtcsv47))
		v1 = 47;
	if (!strcmp(a1, aRtcmv48))
		v1 = 48;
	if (!strcmp(a1, aRtcsv48))
		v1 = 48;
	if (!strcmp(a1, aRtcmv49))
		v1 = 49;
	v2 = strcmp(a1, aRtcsv49);
	result = 49;
	if (v2)
		return v1;
	return result;
}



void u4820a0(FileContext* f, int* t, MemStream* sec, int vid)
{
	// F*** knows what ths does.. something to do with the title.
	const char* HeaderString_47C440; // eax
	int x3_Int32_47C400; // eax
	char* v6; // [esp-Ch] [ebp-18h]


	HeaderString_47C440 = MemStream_getFirstOfString(sec);
	//Util_StrAlloc_47ABA0((void**)t + 4, HeaderString_47C440, 1, v6, aDiolrh, 178);

	Util_StrAlloc(t + 4,HeaderString_47C440,1);
	t[10] = FileContext_getInt(sec);
	if (vid >= 39)
		t[11] = FileContext_getInt(sec);
	x3_Int32_47C400 = FileContext_getInt(sec);
	t[12] = x3_Int32_47C400;
	if (vid < 38)
		t[12] = (2 * x3_Int32_47C400);
	if (vid < 39)
		t[11] = t[12];



}

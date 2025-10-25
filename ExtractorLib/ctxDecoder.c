#include "ctxDecoder.h"
#include "IDAtypes.h"

uint32_t CTX_DecompresFromMemoryStream( MemStream* src, char** outData, uint32_t* outSize)
{
	uint32_t* p_size; // edi
	char* v3; // ebx
	uint32_t v4; // eax
	RawCtx* ctx = (RawCtx*)calloc(1, LZAR_WORKSPACE_BYTES);
	if (!ctx) return 0;
	p_size = &src->size;
	v3 = CTX_Decompress(ctx, (char*)src->buf, *p_size, &src->size);
	free(src->buf);

	v4= *p_size;
	src->buf = v3;
	src->capacity = v4;
	*outData = src->buf;
	*outSize = v4;

	return MemStream_StreamSeek(src,0);





}


char* CTX_Decompress(RawCtx* c, char* src, uint32_t srcSize, uint32_t* outSize)
{

	uint32_t  outTarget = 0;
	c->inBuf = src;
	c->inSize = srcSize;
	if (!CTX_ReadOutpusizeLE(c)) return 0;

	outTarget = c->outTarget;
	c->outBuf = 0;
	c->outBuf = (char*)calloc(outTarget, sizeof(char));

	if (c->outBuf)
	{
		CTX_DecodeStream(c);
		*outSize = c->outPos;
	}

	return c->outBuf;

}

char CTX_DecodeStream(RawCtx* c) {

	int v2; // esi
	int v3; // eax
	uint8_t v4; // bl
	int v5; // ebx
	int v6; // edi
	__int16 v7; // ax
	int v8; // edx
	int v9; // ebx
	uint32_t i1; // [esp+Ch] [ebp-Ch]
	__int16 v12; // [esp+10h] [ebp-8h]
	int v13; // [esp+14h] [ebp-4h]
	CTX_ResetCoderState(c, 0);
	CTX_PrimeCoderFromBits(c);
	CTX_InitModels(c);
	v2 = 4036;
	memset(c->scratch,0x20u, 0xFC4u);
	i1 = 0;
	if (!c->outTarget) return 1;
 

	do
	{
		v3 = CTX_DecodeLitLenSymbol(c);
		v4 = v3;
		if (v3 >= 256)
		{
			v5 = v3 - 253;
			v6 = 0;
			v7 = (v2 - CTX_DecodeMatchDistance((RawCtx*)c) - 1) & 0xFFF;
			v13 = v5;
			v12 = v7;
			if (v5 > 0)
			{
				i1 += v5;
				while (1)
				{
					v8 = ((_WORD)v6 + v7) & 0xFFF;
					v9 = c->scratch[v8];
					CTX_EmitByte(c, c->scratch[v8]);
					c->scratch[v2] = v9;
					v2 = ((_WORD)v2 + 1) & 0xFFF;
					if (++v6 >= v13)
						break;
					v7 = v12;
				}
			}
		}
		else{
			CTX_EmitByte(c, v3);
			c->scratch[v2] = v4;
			v2 = ((_WORD)v2 + 1) & 0xFFF;
			++i1;
		}


	}while( i1 < c->outTarget);

	return 1;
}



bool  CTX_ReadOutpusizeLE(RawCtx* ths)
{
	unsigned int v2; // ecx
	const char* inBuf; // esi
	int v4; // edx
	uint32_t v5; // edi

	v2 = 0;
	inBuf = ths->inBuf;
	ths->outTarget = 0;
	do
	{
		v4 = *(unsigned __int8*)inBuf << v2;
		v2 += 8;
		v5 = v4 + ths->outTarget;
		++inBuf;
		ths->outTarget = v5;
	} while (v2 < 0x20);
	return v5 != 0;
}
int  CTX_ResetCoderState(RawCtx* ths, int a2)

{
	int result; // eax

	*(_DWORD*)&ths->scratch[66332] = 0;
	*(_DWORD*)&ths->scratch[66340] = 0;
	*(_DWORD*)&ths->scratch[4164] = 0;
	ths->outPos = 0;
	*(_WORD*)&ths->scratch[66326] = 0;
	*(_DWORD*)&ths->scratch[66336] = 0x20000;
	result = a2 != 0 ? 0x80 : 0;
	ths->unk1 = 4;
	ths->inPos = 4;
	*(_WORD*)&ths->scratch[66328] = result;
	return result;
}


BOOL   CTX_PrimeCoderFromBits(RawCtx* ths)
{

	int v2; // edi
	BOOL result; // eax

	v2 = 17;
	do
	{
		result = CTX_ReadBit(ths);
		--v2;
		*(_DWORD*)&ths->scratch[66340] = result + 2 * *(_DWORD*)&ths->scratch[66340];
	} while (v2);
	return result;
}


BOOL CTX_ReadBit(RawCtx* ths)
{
	uint32_t inPos; // eax

	*(_WORD*)&ths->scratch[66328] >>= 1;
	if (!*(_WORD*)&ths->scratch[66328])
	{
		inPos = ths->inPos;
		if (inPos >= ths->inSize)
			*(_WORD*)&ths->scratch[66326] = -1;
		else
			*(_WORD*)&ths->scratch[66326] = (unsigned __int8)ths->inBuf[inPos];
		*(_WORD*)&ths->scratch[66328] = 128;
		ths->inPos = inPos + 1;
	}
	return (*(_WORD*)&ths->scratch[66328] & *(_WORD*)&ths->scratch[66326]) != 0;
}
int16   CTX_InitModels(RawCtx* ths)
{
	uint8_t* v1; // edx
	int v2; // eax
	uint8_t* v3; // esi
	int v4; // esi
	uint8_t* v5; // ecx
	__int16 result; // ax

	v1 = &ths->scratch[58130];
	*(_WORD*)&ths->scratch[58130] = 0;
	v2 = 314;
	v3 = &ths->scratch[56868];
	do
	{
		*((_DWORD*)v3 - 315) = v2--;
		*(_DWORD*)v3 = v2;
		*((_WORD*)v1 - 315) = 1;
		v3 -= 4;
		*((_WORD*)v1 - 1) = *(_WORD*)v1 + 1;
		v1 -= 2;
	} while (v2 >= 1);
	*(_WORD*)&ths->scratch[56872] = 0;
	*(_WORD*)&ths->scratch[66324] = 0;
	v4 = 4096;
	v5 = &ths->scratch[66322];
	do
	{
		v5 -= 2;
		result = *((_WORD*)v5 + 2) + 10000 / (v4 + 200);
		--v4;
		*((_WORD*)v5 + 1) = result;
	} while (v4 >= 1);
	return result;
}
int   CTX_DecodeLitLenSymbol(RawCtx* ths)
{


	int v2; // ecx
	unsigned int v3; // edi
	int v4; // ebp
	unsigned __int16 v5; // cx
	int v6; // ebx
	unsigned int v7; // eax
	int v8; // ecx
	int v9; // eax
	unsigned int v10; // ecx
	int v11; // eax
	int v12; // edi

	 v2 = *(_DWORD*)&ths->scratch[66332];
	 v3 = *(_DWORD*)&ths->scratch[66336] - v2;
	 v4 = CTX_LitLenLowerBound(ths, ((unsigned int)*(unsigned __int16*)&ths->scratch[57502] * (*(_DWORD*)&ths->scratch[66340] - v2 + 1) - 1)
		/ v3);

	 v5 = *(_WORD*)&ths->scratch[57502];
	 v6 = *(_DWORD*)&ths->scratch[66332];

	*(_DWORD*)&ths->scratch[66336] = v6 + v3 * *(unsigned __int16*)&ths->scratch[2 * v4 + 57500] / v5;
	*(_DWORD*)&ths->scratch[66332] = v6 + v3 * *(unsigned __int16*)&ths->scratch[2 * v4 + 57502] / v5;

	while(1)
	{
		v7 = *(_DWORD*)&ths->scratch[66332];

		if (v7 >= 0x10000)
		{
			v8 = *(_DWORD*)&ths->scratch[66340];
			*(_DWORD*)&ths->scratch[66332] = v7 - 0x10000;
			v9 = *(_DWORD*)&ths->scratch[66336];
			*(_DWORD*)&ths->scratch[66340] = v8 - 0x10000;
			*(_DWORD*)&ths->scratch[66336] = v9 - 0x10000;
			goto label8;
		}
		if (v7 >= 0x8000)
		{
			v10 = *(_DWORD*)&ths->scratch[66336];
			if (v10 <= 0x18000)
			{
				*(_DWORD*)&ths->scratch[66340] -= 0x8000;
				*(_DWORD*)&ths->scratch[66332] = v7 - 0x8000;
				*(_DWORD*)&ths->scratch[66336] = v10 - 0x8000;
				goto label8;
			}
		}
		if (*(_DWORD*)&ths->scratch[66336] > 0x10000u)
			break;

		label8:
		v11 = 2 * *(_DWORD*)&ths->scratch[66336];
		*(_DWORD*)&ths->scratch[66332] *= 2;
		*(_DWORD*)&ths->scratch[66336] = v11;
		*(_DWORD*)&ths->scratch[66340] = CTX_ReadBit(ths) + 2 * *(_DWORD*)&ths->scratch[66340];
	}
	v12 = *(_DWORD*)&ths->scratch[4 * v4 + 55612];
	CTX_UpdateLitLenModel(ths,v4);
	return v12;


}
int   CTX_LitLenLowerBound(RawCtx* ths, uint16 a2)
{
	int v2; // esi
	int v3; // edi
	int v4; // eax

	v2 = 1;
	v3 = 314;
	do
	{
		v4 = (v3 + v2) / 2;
		if (*(_WORD*)&ths->scratch[2 * v4 + 57502] <= a2)
			v3 = (v3 + v2) / 2;
		else
			v2 = v4 + 1;
	} while (v2 < v3);
	return v2;
}

int   CTX_DecodeMatchDistance(RawCtx* ths)
{
	int v2; // ecx
	unsigned int v3; // edi
	int v4; // ebp
	unsigned __int16 v5; // cx
	int v6; // ebx
	unsigned int v7; // eax
	int v8; // ecx
	int v9; // eax
	unsigned int v10; // ecx
	int v11; // eax

	v2 = *(_DWORD*)&ths->scratch[66332];
	v3 = *(_DWORD*)&ths->scratch[66336] - v2;
	v4 = CTX_DistUpperBound(
		ths,
		((unsigned int)*(unsigned __int16*)&ths->scratch[58132] * (*(_DWORD*)&ths->scratch[66340] - v2 + 1) - 1)
		/ v3);
	v5 = *(_WORD*)&ths->scratch[58132];
	v6 = *(_DWORD*)&ths->scratch[66332];
	*(_DWORD*)&ths->scratch[66336] = v6 + v3 * *(unsigned __int16*)&ths->scratch[2 * v4 + 58132] / v5;
	*(_DWORD*)&ths->scratch[66332] = v6 + v3 * *(unsigned __int16*)&ths->scratch[2 * v4 + 58134] / v5;
	while (1)
	{
		v7 = *(_DWORD*)&ths->scratch[66332];
		if (v7 >= 0x10000)
		{
			v8 = *(_DWORD*)&ths->scratch[66340];
			*(_DWORD*)&ths->scratch[66332] = v7 - 0x10000;
			v9 = *(_DWORD*)&ths->scratch[66336];
			*(_DWORD*)&ths->scratch[66340] = v8 - 0x10000;
			*(_DWORD*)&ths->scratch[66336] = v9 - 0x10000;
			goto label8;
		}
		if (v7 >= 0x8000)
		{
			v10 = *(_DWORD*)&ths->scratch[66336];
			if (v10 <= 0x18000)
			{
				*(_DWORD*)&ths->scratch[66340] -= 0x8000;
				*(_DWORD*)&ths->scratch[66332] = v7 - 0x8000;
				*(_DWORD*)&ths->scratch[66336] = v10 - 0x8000;
				goto label8;
			}
		}
		if (*(_DWORD*)&ths->scratch[66336] > 0x10000u)
			return v4;
	label8:
		v11 = 2 * *(_DWORD*)&ths->scratch[66336];
		*(_DWORD*)&ths->scratch[66332] *= 2;
		*(_DWORD*)&ths->scratch[66336] = v11;
		*(_DWORD*)&ths->scratch[66340] = CTX_ReadBit(ths) + 2 * *(_DWORD*)&ths->scratch[66340];
	}
}
int   CTX_DistUpperBound(RawCtx* ths, uint16 a2)
{
	int v2; // esi
	int v3; // edi
	int v4; // eax

	v2 = 1;
	v3 = 4096;
	do
	{
		v4 = (v3 + v2) / 2;
		if (*(_WORD*)&ths->scratch[2 * v4 + 58132] <= a2)
			v3 = (v3 + v2) / 2;
		else
			v2 = v4 + 1;
	} while (v2 < v3);
	return v2 - 1;
}
int   CTX_EmitByte(RawCtx* ths, char a2)
{
	ths->outBuf[ths->outPos++] = a2;
	return 0;
}


int  CTX_UpdateLitLenModel(RawCtx* ths, int a2)
{
	__int16 v2; // si
	uint8_t* v3; // edx
	int v4; // edi
	unsigned __int16 v5; // ax
	int v6; // eax
	int v7; // eax
	uint8_t* v8; // esi
	__int16 v9; // di
	int v10; // edi
	int v11; // esi
	int result; // eax
	uint8_t* v13; // ecx

	if (*(_WORD*)&ths->scratch[57502] >= 0x7FFFu)
	{
		v2 = 0;
		v3 = &ths->scratch[57500];
		v4 = 314;
		do
		{
			*((_WORD*)v3 + 315) = v2;
			v5 = *(_WORD*)v3;
			v3 -= 2;
			v6 = (v5 + 1) >> 1;
			*((_WORD*)v3 + 1) = v6;
			v2 += v6;
			--v4;
		} while (v4);
		*(_WORD*)&ths->scratch[57502] = v2;
	}
	v7 = a2;
	v8 = &ths->scratch[2 * a2 + 56870];
	if (*(_WORD*)&ths->scratch[2 * a2 + 56872] == *(_WORD*)v8)
	{
		do
		{
			v9 = *(_WORD*)v8;
			v8 -= 2;
			--v7;
		} while (v9 == *(_WORD*)v8);
		if (v7 < a2)
		{
			v10 = *(_DWORD*)&ths->scratch[4 * a2 + 55612];
			v11 = *(_DWORD*)&ths->scratch[4 * v7 + 55612];
			*(_DWORD*)&ths->scratch[4 * v7 + 55612] = v10;
			*(_DWORD*)&ths->scratch[4 * a2 + 55612] = v11;
			*(_DWORD*)&ths->scratch[4 * v11 + 54356] = a2;
			*(_DWORD*)&ths->scratch[4 * v10 + 54356] = v7;
		}
	}
	++ * (_WORD*)&ths->scratch[2 * v7 + 56872];
	result = v7 - 1;
	if (result >= 0)
	{
		v13 = &ths->scratch[2 * result++ + 57502];
		do
		{
			++*(_WORD*)v13;
			v13 -= 2;
			--result;
		} while (result);
	}
	return result;
}

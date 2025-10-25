#include "util.h"

 UtilBlockHeader* g_tagLists[UTIL_TAG_CAP];
 AllocNode* g_allocNodes[ALLOC_NODE_CAP];
 g_size_5b47b8 = 0;
void Util_Free(char* buffer, char tag)
{
	UtilBlockHeader* hdr = Util_FreeFindBlock(buffer, tag);


}

UtilBlockHeader* Util_FreeFindBlock(void* userPtr, uint8_t tag)
{
	UtilBlockHeader* prev = NULL;
	UtilBlockHeader* cur = g_tagLists[tag];
	while (cur && cur->user != userPtr) {
		prev = cur;
		cur = cur->next;
	}
	if (!cur) return NULL;                 // not found

	if (!prev)  g_tagLists[tag] = cur->next;  // remove head
	else        prev->next = cur->next;  // unlink middle/tail

	return cur;

}

void Util_Malloc2(void** buffer, size_t size, int nextNodeIdx)
{

	AllocNode* nde;
	void* bufferA;

	//alloc buffer
	bufferA = malloc(size);
	if (!bufferA) return;

	nde = malloc(sizeof * nde);
	if (!nde)
	{
		free(bufferA);
		return;
	}
	nde->buffer = bufferA;
	nde->size = size;
	nde->next = g_allocNodes[nextNodeIdx];

	g_allocNodes[nextNodeIdx] = nde;

	g_size_5b47b8 += size;
	*buffer = nde->buffer;

}

void Util_StrAlloc(void** toAlloc, const char* stringToAlloc, int nextNodeIdx)
{
Util_Malloc(toAlloc,strlen(stringToAlloc) +1,nextNodeIdx,"","",0);
strcpy(*toAlloc,stringToAlloc);

}

int  Util_Malloc(void** Buffer, size_t Size, char a3, char tagOne, char tagTwo, int a6)
{
	AllocNode* v7; // eax
	void* Buffera; // [esp+14h] [ebp+4h]

	//trace_init_47A080(dword_5B461C, 0, (int)aUtilmalloc);
	//trace_push_47A040();
	//trace_int_6_479FA0((int)Buffer);
	//trace_int_1_479E60(Size);
	//trace_byte_3_479EA0(a3);
	//trace_int_6_479FA0(a4);
	//trace_int_6_479FA0(a5);
	//trace_int_1_479E60(a6);
	Buffera = malloc(Size);
	g_size_5b47b8 += Size;
	v7 = (AllocNode*)malloc(sizeof(struct AllocNode));
	v7->tagTwo = tagTwo;
	v7->buffer = Buffera;
	v7->size = Size;
	v7->tagOne = tagOne;
	v7->paramA6 = a6;
	v7->next = g_allocNodes[a3];
	g_allocNodes[a3] = (int)v7;
	*Buffer = v7->buffer;
	return 0;
}

void __cdecl Util_StrAlloc_47ABA0(
	void** toAlloc,
	const char* stringToAlloc,
	char a3,
	char* strSource,
	const char* StrTag,
	int a6)
{
	/*trace_init_47A080(dword_5B461C, 0, (int)aUtilstralloc);
	trace_push_47A040();
	trace_int_6_479FA0((int)toAlloc);
	trace_maybe_479F60((int)stringToAlloc);
	trace_maybe_479F60((int)strSource);
	trace_maybe_479F60((int)StrTag);
	trace_int_1_479E60(a6);*/
	Util_Malloc(toAlloc, strlen(stringToAlloc) + 1, a3, (int)strSource, (int)StrTag, a6);
	strcpy((char*)*toAlloc, stringToAlloc);
	//trace_pop_47A060();
	//nullsub_1();
}

void  Util_Free_47A7A0(char* a1, char a2)
{
	char* v2; // esi

	//trace_init_47A080(dword_5B461C, 0, (int)aUtilfree);
	//trace_push_47A040();
	//trace_int_6_479FA0((int)a1);
	//trace_byte_3_479EA0(a2);
	if (a1)
	{
		v2 = (char*)Util_FreeFindBlock((int)a1, a2);
		UtilFreeCheckBlock_47A8A0((int)v2);
		UtilFreeContents_47A930(a1, (int)v2);
		UtilFreeBlock_47A990(v2);
	}
	//trace_pop_47A060();
//	nullsub_1();
}

int  UtilFreeCheckBlock_47A8A0(int a1)
{
	/*trace_init_47A080(dword_5B461C, 0, (int)aUtilfreecheckb);
	trace_push_47A040();
	trace_int_6_479FA0(a1);*/
	/*if (!a1)
		dword_5B47B8 = 1 / strlen(byte_5119B8);
	trace_pop_47A060();
	return nullsub_1();*/
	return 1;
}

int  UtilFreeContents_47A930(char* a1, int a2)
{
	//trace_init_47A080(dword_5B461C, 0, (int)aUtilfreeconten);
	//trace_push_47A040();
	//trace_int_6_479FA0((int)a1);
	//trace_int_6_479FA0(a2);
	//if (a1)
	//{
	//	dword_5B47B8 -= *(_DWORD*)(a2 + 8);
	//	sub_47AD50(a1, *(_DWORD*)(a2 + 8));
	//}
	//trace_pop_47A060();
	//return nullsub_1();
	return 1;
}

void   UtilFreeBlock_47A990(char* a1)
{
	//trace_init_47A080(dword_5B461C, 0, (int)aUtilfreeblock);
	//trace_push_47A040();
	//trace_int_6_479FA0((int)a1);
	//if (a1)
	//	sub_47AD50(a1, 24);
	//trace_pop_47A060();
	//nullsub_1();
}
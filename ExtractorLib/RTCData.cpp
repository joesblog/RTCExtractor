#include <stdio.h>
extern "C" {
#include "MemStream.h"
#include "fileContext.h"
#include "decode.h"
}
extern "C"
{
	_declspec(dllexport) const char*  test()
	{
		const char* inPath = "D:\\Games\\RTC\\Modules\\Examples\\Actions.rtc";
		FileContext fc;
		FileContext_Init(&fc);
		FileContext_Load(&fc,inPath);
		const char* strFileMagicHeader2 = FileContext_GetHeaderString(&fc);
		if (FileContext_ModuleIdFromHeader(strFileMagicHeader2) > 0x22)
		{
			MemStream* SectionMOD_47C870 = FileContext_FindSection(&fc, "MOD");
			MemStream_DecompressSectionData(SectionMOD_47C870);

			  char* ret = (  char*)calloc(SectionMOD_47C870->size,1);
			memcpy(ret,SectionMOD_47C870->buf, SectionMOD_47C870->size);
			return ret;
		}



 return inPath;
	}
  __declspec(dllexport)
    int GetModSection(const char* inPath, uint8_t** outData, size_t* outSize)
  {
    if (!outData || !outSize) return -1;
    *outData = nullptr;
    *outSize = 0;

    //const char* inPath = "D:\\Games\\RTC\\Modules\\Examples\\Actions.rtc";

    FileContext fc;
    FileContext_Init(&fc);
    FileContext_Load(&fc, inPath);
    //if (!FileContext_Load(&fc, inPath)) return -2;

    const char* hdr = FileContext_GetHeaderString(&fc);
    if (FileContext_ModuleIdFromHeader(hdr) <= 0x22) return -3;

    MemStream* sec = FileContext_FindSection(&fc, "MOD");
    if (!sec) return -4;

    if (!MemStream_DecompressSectionData(sec)) return -5;

    if (sec->size == 0 || !sec->buf) return 0; // empty section

    // allocate and copy
    void* buf = malloc(sec->size);
    if (!buf) return -6;

    memcpy(buf, sec->buf, sec->size);

    *outData = static_cast<uint8_t*>(buf);
    *outSize = sec->size;
    MemStream_Delete(sec);
    return 0;
  }
  __declspec(dllexport)
    void FreeBuffer(void* p)
  {
    if (p) free(p);
  }

}

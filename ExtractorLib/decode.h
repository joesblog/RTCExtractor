#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#if defined(_WIN32)
#  include <io.h>
#  include <fcntl.h>
#endif
 

 void decode_buffer(uint8_t* data, size_t len, int32_t seed);
 size_t safe_read(void* buf, size_t size, FILE* fp);


 
 int decode_rtc_file(const char* input_path, const char* output_path);

 
 int decode_ReadAndDecryptFile(const char* input_path, char** buffer, int* length);
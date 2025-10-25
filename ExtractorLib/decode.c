#include "decode.h"
void decode_buffer(uint8_t* data, size_t len, int32_t seed)
{
	for (size_t idx = 0; idx < len; ++idx) {
		uint8_t mask;

		switch (idx & 3) {
		case 0:
			mask = (uint8_t)(seed & 0xFF);
			seed += 1;
			break;
		case 1:
			mask = (uint8_t)((seed >> 8) & 0xFF);
			seed *= 13;
			break;
		case 2: {
			mask = (uint8_t)((seed >> 16) & 0xFF);
			uint8_t byte2 = (uint8_t)((seed >> 16) & 0xFF);
			seed ^= (int32_t)(0x01010101U * byte2);
			break;
		}
		case 3:
			mask = (uint8_t)((seed >> 24) & 0xFF);
			seed -= 1;
			break;
		default:
			mask = 0;
			break;
		}

		data[idx] ^= mask;
	}
}

size_t safe_read(void* buf, size_t size, FILE* fp)
{
	size_t total = 0;
	while (total < size) {
		size_t got = fread((uint8_t*)buf + total, 1, size - total, fp);
		if (got == 0) {
			if (feof(fp))
				break;
			if (ferror(fp)) {
				perror("Read error");
				clearerr(fp);
				break;
			}
		}
		total += got;
	}
	return total;
}

 
// Decode a single file
 

int decode_rtc_file(const char* input_path, const char* output_path)
{
#if defined(_WIN32)
	_set_fmode(_O_BINARY); // force binary mode
#endif

	FILE* fp_in = fopen(input_path, "rb");
	if (!fp_in) {
		fprintf(stderr, "Cannot open input: %s (%s)\n", input_path, strerror(errno));
		return 1;
	}

	// Read 8-byte header: checksum + seed
	uint32_t expected_checksum = 0;
	int32_t seed = 0;

	if (fread(&expected_checksum, 4, 1, fp_in) != 1 ||
		fread(&seed, 4, 1, fp_in) != 1) {
		fprintf(stderr, "File too short or unreadable header.\n");
		fclose(fp_in);
		return 2;
	}

	// Determine remaining file length
	if (fseek(fp_in, 0, SEEK_END) != 0) {
		perror("Seek end failed");
		fclose(fp_in);
		return 3;
	}
	long end_pos = ftell(fp_in);
	if (end_pos < 0) {
		perror("ftell failed");
		fclose(fp_in);
		return 4;
	}
	if (fseek(fp_in, 8, SEEK_SET) != 0) {
		perror("Seek data failed");
		fclose(fp_in);
		return 5;
	}

	size_t data_len = (size_t)(end_pos - 8);
	if (data_len == 0) {
		fprintf(stderr, "File is empty after header.\n");
		fclose(fp_in);
		return 6;
	}

	// Allocate buffer
	uint8_t* data = (uint8_t*)malloc(data_len);
	if (!data) {
		fprintf(stderr, "Memory allocation failed for %zu bytes.\n", data_len);
		fclose(fp_in);
		return 7;
	}

	// Read file data safely
	size_t read_bytes = safe_read(data, data_len, fp_in);
	if (read_bytes != data_len) {
		fprintf(stderr, "Read %zu of %zu bytes. File may be truncated.\n",
			read_bytes, data_len);
		free(data);
		fclose(fp_in);
		return 8;
	}

	fclose(fp_in);

	// Decode in place
	decode_buffer(data, data_len, seed);

	// Verify checksum
	int32_t checksum = 291074;

	for (int32_t idx = 0; idx < (int32_t)data_len; ++idx) {
		int8_t byte = (int8_t)data[idx];  // signed char!
		int32_t shift = idx & 0x1F;
		// force 32-bit signed overflow wrap
		checksum = (int32_t)(checksum + ((int32_t)byte << shift));
	}

	if ((uint32_t)checksum != expected_checksum) {
		fprintf(stderr, "Checksum mismatch! Expected %08X, got %08X\n",
			expected_checksum, checksum);
		free(data);
		return 9;
	}

	// Write decoded output
	FILE* fp_out = fopen(output_path, "wb");
	if (!fp_out) {
		fprintf(stderr, "Cannot open output: %s (%s)\n", output_path, strerror(errno));
		free(data);
		return 10;
	}

	size_t written = fwrite(data, 1, data_len, fp_out);
	if (written != data_len) {
		fprintf(stderr, "Write failed: wrote %zu of %zu bytes.\n", written, data_len);
		fclose(fp_out);
		free(data);
		return 11;
	}

	fclose(fp_out);
	free(data);

	printf("Decoded OK: %s → %s (%zu bytes)\n", input_path, output_path, data_len);
	return 0;
}

 
// Decode a single file to a buffer
 
int   decode_ReadAndDecryptFile(const char* input_path, char** buffer, int* length)
{
#if defined(_WIN32)
	_set_fmode(_O_BINARY); // force binary mode
#endif

	FILE* fp_in = fopen(input_path, "rb");
	if (!fp_in) {
		fprintf(stderr, "Cannot open input: %s (%s)\n", input_path, strerror(errno));
		return 0;
	}

	// Read 8-byte header: checksum + seed
	uint32_t expected_checksum = 0;
	int32_t seed = 0;

	if (fread(&expected_checksum, 4, 1, fp_in) != 1 ||
		fread(&seed, 4, 1, fp_in) != 1) {
		fprintf(stderr, "File too short or unreadable header.\n");
		fclose(fp_in);
		return 0;
	}

	// Determine remaining file length
	if (fseek(fp_in, 0, SEEK_END) != 0) {
		perror("Seek end failed");
		fclose(fp_in);
		return 0;
	}
	long end_pos = ftell(fp_in);
	if (end_pos < 0) {
		perror("ftell failed");
		fclose(fp_in);
		return 0;
	}
	if (fseek(fp_in, 8, SEEK_SET) != 0) {
		perror("Seek data failed");
		fclose(fp_in);
		return 0;
	}

	size_t data_len = (size_t)(end_pos - 8);
	if (data_len == 0) {
		fprintf(stderr, "File is empty after header.\n");
		fclose(fp_in);
		return 0;
	}

	// Allocate buffer
	uint8_t* data = (uint8_t*)malloc(data_len);
	if (!data) {
		fprintf(stderr, "Memory allocation failed for %zu bytes.\n", data_len);
		fclose(fp_in);
		return 0;
	}

	// Read file data safely
	size_t read_bytes = safe_read(data, data_len, fp_in);
	if (read_bytes != data_len) {
		fprintf(stderr, "Read %zu of %zu bytes. File may be truncated.\n",
			read_bytes, data_len);
		free(data);
		fclose(fp_in);
		return 0;
	}

	fclose(fp_in);

	decode_buffer(data, data_len, seed);
	// Verify checksum
	int32_t checksum = 291074;

	for (int32_t idx = 0; idx < (int32_t)data_len; ++idx) {
		int8_t byte = (int8_t)data[idx];  // signed char!
		int32_t shift = idx & 0x1F;
		// force 32-bit signed overflow wrap
		checksum = (int32_t)(checksum + ((int32_t)byte << shift));
	}

	if ((uint32_t)checksum != expected_checksum) {
		fprintf(stderr, "Checksum mismatch! Expected %08X, got %08X\n",
			expected_checksum, checksum);
		free(data);
		return 0;
	}

	*buffer = (char*)data;
	*length = data_len;

	
	return 1;

}
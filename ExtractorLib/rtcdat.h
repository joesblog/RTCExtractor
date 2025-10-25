// game_data.h
#pragma once
#include <stdint.h>

//#include <minwindef.h>
#define SET_LOWORD(a, value) ((a) = ((a) & 0xFFFF0000) | (value))
#define SET_HIWORD(a, value) ((a) = ((a) & 0x0000FFFF) | ((value) << 16))
#define SET_BYTE2(a, value) ((a) = ((a) & 0xFF00FFFF) | ((value) << 8))
#define SET_BYTE_N(a, bytepos, val) ((a) = ((a) & ~(0xFF << (bytepos * 8))) | ((val) << (bytepos * 8)))
#define SET_LOBYTE(a, value)  ((a) = ((a) & 0xFFFFFF00) | ((value) & 0xFF))


typedef struct rtcdat
{

	uint8_t byte_5118F4;
	uint8_t byte_4DD0D7;
	uint8_t byte_507590;
	uint8_t byte_51188C;
	uint8_t byte_50C3F4;
	uint8_t byte_50C420;
	uint8_t byte_50759A;
	uint8_t byte_50C550;
	uint8_t byte_50C570;
	uint8_t byte_4FD8F0;
	uint8_t byte_509CB0;
	uint8_t byte_511865;
	uint8_t byte_511864;
	uint8_t byte_507598;
	uint8_t byte_50C3FF;
	uint8_t byte_50C3FE;
	uint8_t byte_50C3FD;
	uint8_t byte_50759C;

	uint32_t dword_5118DC;
	uint32_t dword_507544[10];
	uint32_t dword_50C56C;
	uint32_t dword_50C3C4;
	uint32_t dword_50F148;
	uint32_t dword_5118CC;
	uint32_t dword_4FD8E8;
	uint32_t dword_50C3EC;
	uint32_t dword_50C3F0;
	uint32_t dword_4FD8F8;
	uint32_t dword_50C55C;
	uint32_t dword_50C54C;
	uint32_t dword_50C52C;
	uint32_t dword_50C548;
	uint32_t dword_50C51C;
	uint32_t dword_50C514;
	uint32_t dword_50C558;
	uint32_t dword_50C43C;
	uint32_t dword_507564;
	uint32_t dword_50755C;
	uint32_t dword_50C400;
	uint32_t dword_51199C;
	uint32_t dword_511924;


} rtcdat;

init_dat(rtcdat* dt);
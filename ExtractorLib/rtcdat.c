#include "rtcdat.h"


//HIWORD\((.*?)\)\s{0,}(.*?);
//SET_HIWORD(dt->$1,$2);\n

//LOWORD\((.*?)\)\s{0,}=\s{0,}(.*?);
//SET_LOWORD(dt->$1,$2);\n

init_dat(rtcdat* dt)
{
	dt->byte_5118F4 = 1;
	dt->byte_4DD0D7 = 1;
	dt->byte_507590 = 0;
	dt->byte_51188C = 0;
	dt->byte_50C3F4 = 0;
	dt->byte_50C420 = 1;
	dt->byte_50759A = 0;
	dt->byte_50C550 = 1;
	dt->byte_50C570 = 0;
	dt->byte_4FD8F0 = 0;
	dt->byte_509CB0 = 1;
	dt->byte_511865 = 1;
	dt->byte_511864 = 1;
	dt->byte_507598 = 1;


	dt->dword_50C56C = 0;
	dt->dword_50C3C4 = 0;
	dt->dword_50F148 = 0;
	dt->dword_5118CC = 7;
	dt->dword_4FD8E8 = 5;
	dt->dword_50C3EC = 640;
	dt->dword_50C3F0 = 400;
	dt->dword_4FD8F8 = -1;
	dt->dword_50C55C = 926299490;

	SET_LOWORD(dt->dword_507544[0],1);

	SET_HIWORD(dt->dword_50C54C, 25956);

	SET_LOWORD(dt->dword_50C52C, 353);
	SET_LOWORD(dt->dword_50C548, 26211);
	SET_LOWORD(dt->dword_50C514, 13106);
	SET_LOWORD(dt->dword_50C558, 14648);

	SET_LOBYTE(dt->dword_50C51C, 49);
	SET_LOBYTE(dt->dword_50C43C, 8);
	SET_LOBYTE(dt->dword_507564, 13);
	SET_LOBYTE(dt->dword_50755C, 9);
	SET_LOBYTE(dt->dword_50C400, 112);

	SET_BYTE2(dt->dword_50C514, 52);
	SET_BYTE2(dt->dword_50C558, 48);

	dt->byte_50C3FF = 113;
	dt->byte_50C3FE = 114;
	dt->byte_50C3FD = 115;


	//LOWORD(dt->dword_507544[0]) = 1;
}

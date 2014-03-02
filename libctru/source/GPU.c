#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/GSP.h>
#include <ctr/GX.h>
#include <ctr/GPU.h>
#include <ctr/svc.h>

const u32 gpuRegInitTable[]={0x1EF01000, 0x00000000,
						0x1EF01080, 0x12345678,
						0x1EF010C0, 0xFFFFFFF0,
						0x1EF010D0, 0x00000001};

const u32 gpuRegTopScreenInitTable[]={0x1EF00400, 0x000001C2,
								0x1EF00404, 0x000000D1,
								0x1EF00408, 0x000001C1,
								0x1EF0040C, 0x000001C1,
								0x1EF00410, 0x00000000,
								0x1EF00414, 0x000000CF,
								0x1EF00418, 0x000000D1,
								0x1EF0041C, 0x01C501C1,
								0x1EF00420, 0x00010000,
								0x1EF00424, 0x0000019D,
								0x1EF00428, 0x00000002,
								0x1EF0042C, 0x00000192,
								0x1EF00430, 0x00000192,
								0x1EF00434, 0x00000192,
								0x1EF00438, 0x00000001,
								0x1EF0043C, 0x00000002,
								0x1EF00440, 0x01960192,
								0x1EF00444, 0x00000000,
								0x1EF00448, 0x00000000,
								0x1EF0045C, 0x019000F0,
								0x1EF00460, 0x01C100D1,
								0x1EF00464, 0x01920002,
								0x1EF00470, 0x00080340,
								0x1EF0049C, 0x00000000,

								0x1EF00468, 0x18300000,
								0x1EF0046C, 0x18300000,
								0x1EF00494, 0x18300000,
								0x1EF00498, 0x18300000,
								0x1EF00478, 0x18300000};

Result writeRegisterValues(Handle* handle, u32* table, u32 num)
{
	if(!table || !num)return -1;
	int i;
	Result ret;
	for(i=0;i<num;i++)
	{
		if((ret=GSPGPU_WriteHWRegs(handle, GSP_REBASE_REG(table[0]), &table[1], 4)))return ret;
		table+=2;
	}
	return 0;
}

void GPU_Init(Handle *gsphandle)
{
	u32 data;
	u32 mask;

	writeRegisterValues(gsphandle, (u32*)gpuRegInitTable, sizeof(gpuRegInitTable)/8);
	writeRegisterValues(gsphandle, (u32*)gpuRegTopScreenInitTable, sizeof(gpuRegTopScreenInitTable)/8);

	data=0x00;
	mask=0xFF00;
	GSPGPU_WriteHWRegsWithMask(gsphandle, GSP_REBASE_REG(0x1EF00C18), &data, 4, &mask, 4);
	
	data=0x70100;
	GSPGPU_WriteHWRegs(gsphandle, GSP_REBASE_REG(0x1EF00004), &data, 4);

	data=0x00;
	mask=0xFF;
	GSPGPU_WriteHWRegsWithMask(gsphandle, GSP_REBASE_REG(0x1EF0001C), &data, 4, &mask, 4);

	mask=0xFF;
	GSPGPU_WriteHWRegsWithMask(gsphandle, GSP_REBASE_REG(0x1EF0002C), &data, 4, &mask, 4);
	
	data=0x22221200;
	GSPGPU_WriteHWRegsWithMask(gsphandle, GSP_REBASE_REG(0x1EF00050), &data, 4, &mask, 4);

	data=0x0000FF52;
	mask=0x00FF52FF;
	GSPGPU_WriteHWRegsWithMask(gsphandle, GSP_REBASE_REG(0x1EF00054), &data, 4, &mask, 4);
	
	data=0x10501;
	GSPGPU_WriteHWRegs(gsphandle, GSP_REBASE_REG(0x1EF00474), &data, 4);
}

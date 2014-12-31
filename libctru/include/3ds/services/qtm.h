#pragma once

//See also: http://3dbrew.org/wiki/QTM_Services

typedef struct {
	float x;
	float y;
} qtmHeadtrackingInfoCoord;

typedef struct {
	u8 flags[5];
	u8 padding[3];
	float floatdata_x08;//"not used by System_Settings."
	qtmHeadtrackingInfoCoord coords0[4];
	u32 unk_x2c[5];//"Not used by System_Settings."
} qtmHeadtrackingInfo;

Result qtmInit();
void qtmExit();
bool qtmCheckInitialized();

Result qtmGetHeadtrackingInfo(u64 val, qtmHeadtrackingInfo *out);//val is normally 0.
bool qtmCheckHeadFullyDetected(qtmHeadtrackingInfo *info);
Result qtmConvertCoordToScreen(qtmHeadtrackingInfoCoord *coord, float *screen_width, float *screen_height, u32 *x, u32 *y);//screen_* can be NULL to use the default values for the top-screen.


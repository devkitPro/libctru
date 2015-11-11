#include <3ds/services/cam.h>
#include <3ds/services/y2r.h>
#include <3ds/srv.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/ipc.h>

Handle camHandle;
static int camRefCount;

Result camInit(void) {
	Result ret = 0;

	if (AtomicPostIncrement(&camRefCount)) return 0;

	ret = srvGetServiceHandle(&camHandle, "cam:u");
	if (R_SUCCEEDED(ret))
	{
		ret = CAMU_DriverInitialize();
		if (R_FAILED(ret)) svcCloseHandle(camHandle);
	}
	if (R_FAILED(ret)) AtomicDecrement(&camRefCount);

	return ret;
}

void camExit(void) {
	if (AtomicDecrement(&camRefCount)) return;
	CAMU_DriverFinalize();
	svcCloseHandle(camHandle);
}

Result CAMU_StartCapture(u32 port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1,1,0); // 0x10040
	cmdbuf[1] = port;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_StopCapture(u32 port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2,1,0); // 0x20040
	cmdbuf[1] = port;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_IsBusy(bool* busy, u32 port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x3,1,0); // 0x30040
	cmdbuf[1] = port;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*busy = (bool) cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result CAMU_ClearBuffer(u32 port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x4,1,0); // 0x40040
	cmdbuf[1] = port;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_GetVsyncInterruptEvent(Handle* event, u32 port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x5,1,0); // 0x50040
	cmdbuf[1] = port;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*event = cmdbuf[3];
	return cmdbuf[1];
}

Result CAMU_GetBufferErrorInterruptEvent(Handle* event, u32 port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x6,1,0); // 0x60040
	cmdbuf[1] = port;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*event = cmdbuf[3];
	return cmdbuf[1];
}

Result CAMU_SetReceiving(Handle* event, void* dst, u32 port, u32 imageSize, s16 transferUnit) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x7,4,2); // 0x70102
	cmdbuf[1] = (u32) dst;
	cmdbuf[2] = port;
	cmdbuf[3] = imageSize;
	cmdbuf[4] = transferUnit;
	cmdbuf[5] = IPC_Desc_SharedHandles(1);
	cmdbuf[6] = CUR_PROCESS_HANDLE;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*event = cmdbuf[3];
	return cmdbuf[1];
}

Result CAMU_IsFinishedReceiving(bool* finishedReceiving, u32 port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x8,1,0); // 0x80040
	cmdbuf[1] = port;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*finishedReceiving = (bool) cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result CAMU_SetTransferLines(u32 port, s16 lines, s16 width, s16 height) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x9,4,0); // 0x90100
	cmdbuf[1] = port;
	cmdbuf[2] = lines;
	cmdbuf[3] = width;
	cmdbuf[4] = height;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_GetMaxLines(s16* maxLines, s16 width, s16 height) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xA,2,0); // 0xA0080
	cmdbuf[1] = width;
	cmdbuf[2] = height;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*maxLines = (s16) cmdbuf[2] & 0xFFFF;
	return cmdbuf[1];
}

Result CAMU_SetTransferBytes(u32 port, u32 bytes, s16 width, s16 height) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xB,4,0); // 0xB0100
	cmdbuf[1] = port;
	cmdbuf[2] = bytes;
	cmdbuf[3] = width;
	cmdbuf[4] = height;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_GetTransferBytes(u32* transferBytes, u32 port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xC,1,0); // 0xC0040
	cmdbuf[1] = port;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*transferBytes = cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_GetMaxBytes(u32* maxBytes, s16 width, s16 height) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xD,2,0); // 0xD0080
	cmdbuf[1] = width;
	cmdbuf[2] = height;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*maxBytes = cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetTrimming(u32 port, bool trimming) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xE,2,0); // 0xE0080
	cmdbuf[1] = port;
	cmdbuf[2] = trimming;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_IsTrimming(bool* trimming, u32 port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xF,1,0); // 0xF0040
	cmdbuf[1] = port;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*trimming = (bool) cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result CAMU_SetTrimmingParams(u32 port, s16 xStart, s16 yStart, s16 xEnd, s16 yEnd) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x10,5,0); // 0x100140
	cmdbuf[1] = port;
	cmdbuf[2] = xStart;
	cmdbuf[3] = yStart;
	cmdbuf[4] = xEnd;
	cmdbuf[5] = yEnd;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_GetTrimmingParams(s16* xStart, s16* yStart, s16* xEnd, s16* yEnd, u32 port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x11,1,0); // 0x110040
	cmdbuf[1] = port;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*xStart = (s16) cmdbuf[2] & 0xFFFF;
	*yStart = (s16) cmdbuf[3] & 0xFFFF;
	*xEnd = (s16) cmdbuf[4] & 0xFFFF;
	*yEnd = (s16) cmdbuf[5] & 0xFFFF;
	return cmdbuf[1];
}

Result CAMU_SetTrimmingParamsCenter(u32 port, s16 trimWidth, s16 trimHeight, s16 camWidth, s16 camHeight) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x12,5,0); // 0x120140
	cmdbuf[1] = port;
	cmdbuf[2] = trimWidth;
	cmdbuf[3] = trimHeight;
	cmdbuf[4] = camWidth;
	cmdbuf[5] = camHeight;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_Activate(u32 select) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x13,1,0); // 0x130040
	cmdbuf[1] = select;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SwitchContext(u32 select, CAMU_Context context) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x14,2,0); // 0x140080
	cmdbuf[1] = select;
	cmdbuf[2] = context;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetExposure(u32 select, s8 exposure) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x15,2,0); // 0x150080
	cmdbuf[1] = select;
	cmdbuf[2] = exposure;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetWhiteBalance(u32 select, CAMU_WhiteBalance whiteBalance) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x16,2,0); // 0x160080
	cmdbuf[1] = select;
	cmdbuf[2] = whiteBalance;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetWhiteBalanceWithoutBaseUp(u32 select, CAMU_WhiteBalance whiteBalance) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x17,2,0); // 0x170080
	cmdbuf[1] = select;
	cmdbuf[2] = whiteBalance;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetSharpness(u32 select, s8 sharpness) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x18,2,0); // 0x180080
	cmdbuf[1] = select;
	cmdbuf[2] = sharpness;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetAutoExposure(u32 select, bool autoExposure) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x19,2,0); // 0x190080
	cmdbuf[1] = select;
	cmdbuf[2] = autoExposure;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_IsAutoExposure(bool* autoExposure, u32 select) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1A,1,0); // 0x1A0040
	cmdbuf[1] = select;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*autoExposure = (bool) cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result CAMU_SetAutoWhiteBalance(u32 select, bool autoWhiteBalance) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1B,2,0); // 0x1B0080
	cmdbuf[1] = select;
	cmdbuf[2] = autoWhiteBalance;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_IsAutoWhiteBalance(bool* autoWhiteBalance, u32 select) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1C,1,0); // 0x1C0040
	cmdbuf[1] = select;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*autoWhiteBalance = (bool) cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}

Result CAMU_FlipImage(u32 select, CAMU_Flip flip, CAMU_Context context) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1D,3,0); // 0x1D00C0
	cmdbuf[1] = select;
	cmdbuf[2] = flip;
	cmdbuf[3] = context;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetDetailSize(u32 select, s16 width, s16 height, s16 cropX0, s16 cropY0, s16 cropX1, s16 cropY1, CAMU_Context context) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1E,8,0); // 0x1E0200
	cmdbuf[1] = select;
	cmdbuf[2] = width;
	cmdbuf[3] = height;
	cmdbuf[4] = cropX0;
	cmdbuf[5] = cropY0;
	cmdbuf[6] = cropX1;
	cmdbuf[7] = cropY1;
	cmdbuf[8] = context;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetSize(u32 select, CAMU_Size size, CAMU_Context context) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1F,3,0); // 0x1F00C0
	cmdbuf[1] = select;
	cmdbuf[2] = size;
	cmdbuf[3] = context;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetFrameRate(u32 select, CAMU_FrameRate frameRate) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x20,2,0); // 0x200080
	cmdbuf[1] = select;
	cmdbuf[2] = frameRate;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetPhotoMode(u32 select, CAMU_PhotoMode photoMode) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x21,2,0); // 0x210080
	cmdbuf[1] = select;
	cmdbuf[2] = photoMode;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetEffect(u32 select, CAMU_Effect effect, CAMU_Context context) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x22,3,0); // 0x2200C0
	cmdbuf[1] = select;
	cmdbuf[2] = effect;
	cmdbuf[3] = context;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetContrast(u32 select, CAMU_Contrast contrast) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x23,2,0); // 0x230080
	cmdbuf[1] = select;
	cmdbuf[2] = contrast;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetLensCorrection(u32 select, CAMU_LensCorrection lensCorrection) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x24,2,0); // 0x240080
	cmdbuf[1] = select;
	cmdbuf[2] = lensCorrection;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetOutputFormat(u32 select, CAMU_OutputFormat format, CAMU_Context context) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x25,2,0); // 0x2500C0
	cmdbuf[1] = select;
	cmdbuf[2] = format;
	cmdbuf[3] = context;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetAutoExposureWindow(u32 select, s16 x, s16 y, s16 width, s16 height) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x26,5,0); // 0x260140
	cmdbuf[1] = select;
	cmdbuf[2] = x;
	cmdbuf[3] = y;
	cmdbuf[4] = width;
	cmdbuf[5] = height;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetAutoWhiteBalanceWindow(u32 select, s16 x, s16 y, s16 width, s16 height) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x27,5,0); // 0x270140
	cmdbuf[1] = select;
	cmdbuf[2] = x;
	cmdbuf[3] = y;
	cmdbuf[4] = width;
	cmdbuf[5] = height;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetNoiseFilter(u32 select, bool noiseFilter) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x28,2,0); // 0x280080
	cmdbuf[1] = select;
	cmdbuf[2] = noiseFilter;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SynchronizeVsyncTiming(u32 select1, u32 select2) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x29,2,0); // 0x290080
	cmdbuf[1] = select1;
	cmdbuf[2] = select2;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_GetLatestVsyncTiming(s64* timing, u32 port, u32 past) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2A,2,0); // 0x2A0080
	cmdbuf[1] = port;
	cmdbuf[2] = past;
	cmdbuf[49] = (past << 17) | 2;
	cmdbuf[50] = (u32) timing;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_GetStereoCameraCalibrationData(CAMU_StereoCameraCalibrationData* data) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2B,0,0); // 0x2B0000

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*data = *(CAMU_StereoCameraCalibrationData*) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetStereoCameraCalibrationData(CAMU_StereoCameraCalibrationData data) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2C,16,0); // 0x2C0400
	*(CAMU_StereoCameraCalibrationData*) cmdbuf[1] = data;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_WriteRegisterI2c(u32 select, u16 addr, u16 data) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2D,3,0); // 0x2D00C0
	cmdbuf[1] = select;
	cmdbuf[2] = addr;
	cmdbuf[3] = data;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_WriteMcuVariableI2c(u32 select, u16 addr, u16 data) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2E,3,0); // 0x2E00C0
	cmdbuf[1] = select;
	cmdbuf[2] = addr;
	cmdbuf[3] = data;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_ReadRegisterI2cExclusive(u16* data, u32 select, u16 addr) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2F,2,0); // 0x2F0080
	cmdbuf[1] = select;
	cmdbuf[2] = addr;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*data = (u16) cmdbuf[2] & 0xFFFF;
	return cmdbuf[1];
}

Result CAMU_ReadMcuVariableI2cExclusive(u16* data, u32 select, u16 addr) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x30,2,0); // 0x300080
	cmdbuf[1] = select;
	cmdbuf[2] = addr;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*data = (u16) cmdbuf[2] & 0xFFFF;
	return cmdbuf[1];
}

Result CAMU_SetImageQualityCalibrationData(CAMU_ImageQualityCalibrationData data) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x31,6,0); // 0x310180
	*(CAMU_ImageQualityCalibrationData*) cmdbuf[1] = data;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_GetImageQualityCalibrationData(CAMU_ImageQualityCalibrationData* data) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x32,0,0); // 0x320000

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*data = *(CAMU_ImageQualityCalibrationData*) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetPackageParameterWithoutContext(CAMU_PackageParameterCameraSelect param) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x33,11,0); // 0x3302C0
	*(CAMU_PackageParameterCameraSelect*) cmdbuf[1] = param;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetPackageParameterWithContext(CAMU_PackageParameterContext param) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x34,5,0); // 0x340140
	*(CAMU_PackageParameterContext*) cmdbuf[1] = param;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetPackageParameterWithContextDetail(CAMU_PackageParameterContextDetail param) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x35,7,0); // 0x3501C0
	*(CAMU_PackageParameterContextDetail*) cmdbuf[1] = param;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_GetSuitableY2rStandardCoefficient(Y2RU_StandardCoefficient* coefficient) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x36,0,0); // 0x360000

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*coefficient = (Y2RU_StandardCoefficient) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_PlayShutterSound(CAMU_ShutterSoundType sound) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x38,1,0); // 0x380040
	cmdbuf[1] = sound;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_DriverInitialize(void) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x39,0,0); // 0x390000

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_DriverFinalize(void) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x3A,0,0); // 0x3A0000

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_GetActivatedCamera(u32* select) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x3B,0,0); // 0x3B0000

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*select = (u32) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_GetSleepCamera(u32* select) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x3C,0,0); // 0x3C0000

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	*select = (u32) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetSleepCamera(u32 select) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x3D,0,0); // 0x3D0040
	cmdbuf[1] = select;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}

Result CAMU_SetBrightnessSynchronization(bool brightnessSynchronization) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x3E,1,0); // 0x3E0040
	cmdbuf[1] = brightnessSynchronization;

	if (R_FAILED(ret = svcSendSyncRequest(camHandle))) return ret;
	return cmdbuf[1];
}


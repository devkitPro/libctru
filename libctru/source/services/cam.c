#include <3ds/services/cam.h>
#include <3ds/services/y2r.h>
#include <3ds/srv.h>
#include <3ds/svc.h>
#include <3ds/types.h>

Handle camHandle;
static bool initialized = false;

Result camInit() {
	Result ret = 0;

	if (initialized) return 0;

	if (camHandle == 0)
	{
		ret = srvGetServiceHandle(&camHandle, "cam:u");
		if (ret < 0) return ret;
	}

	ret = CAMU_DriverInitialize();
	if (ret < 0) return ret;
	initialized = true;

	return 0;
}

Result camExit() {
	Result ret = 0;

	if (initialized)
	{
		ret = CAMU_DriverFinalize();
		if (ret < 0) return ret;
	}

	if (camHandle != 0)
	{
		ret = svcCloseHandle(camHandle);
		if (ret < 0) return ret;
		camHandle = 0;
	}

	return 0;
}

Result CAMU_StartCapture(CAMU_Port port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00010040;
	cmdbuf[1] = port;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_StopCapture(CAMU_Port port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00020040;
	cmdbuf[1] = port;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_IsBusy(bool* busy, CAMU_Port port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00030040;
	cmdbuf[1] = port;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*busy = (bool) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_ClearBuffer(CAMU_Port port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00040040;
	cmdbuf[1] = port;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_GetVsyncInterruptEvent(Handle* event, CAMU_Port port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00050040;
	cmdbuf[1] = port;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*event = cmdbuf[3];
	return cmdbuf[1];
}

Result CAMU_GetBufferErrorInterruptEvent(Handle* event, CAMU_Port port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00060040;
	cmdbuf[1] = port;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*event = cmdbuf[3];
	return cmdbuf[1];
}

Result CAMU_SetReceiving(Handle* event, void* dst, CAMU_Port port, u32 imageSize, s16 transferUnit) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00070102;
	cmdbuf[1] = (u32) dst;
	cmdbuf[2] = port;
	cmdbuf[3] = imageSize;
	cmdbuf[4] = transferUnit;
	cmdbuf[5] = 0;
	cmdbuf[6] = 0xFFFF8001;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*event = cmdbuf[3];
	return cmdbuf[1];
}

Result CAMU_IsFinishedReceiving(bool* finishedReceiving, CAMU_Port port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00080040;
	cmdbuf[1] = port;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*finishedReceiving = (bool) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetTransferLines(CAMU_Port port, s16 lines, s16 width, s16 height) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00090100;
	cmdbuf[1] = port;
	cmdbuf[2] = lines;
	cmdbuf[3] = width;
	cmdbuf[4] = height;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_GetMaxLines(s16* maxLines, s16 width, s16 height) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x000A0080;
	cmdbuf[1] = width;
	cmdbuf[2] = height;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*maxLines = (s16) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetTransferBytes(CAMU_Port port, u32 bytes, s16 width, s16 height) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x000B0100;
	cmdbuf[1] = port;
	cmdbuf[2] = bytes;
	cmdbuf[3] = width;
	cmdbuf[4] = height;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_GetTransferBytes(u32* transferBytes, CAMU_Port port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x000C0040;
	cmdbuf[1] = port;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*transferBytes = cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_GetMaxBytes(u32* maxBytes, s16 width, s16 height) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x000D0080;
	cmdbuf[1] = width;
	cmdbuf[2] = height;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*maxBytes = cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetTrimming(CAMU_Port port, bool trimming) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x000E0080;
	cmdbuf[1] = port;
	cmdbuf[2] = trimming;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_IsTrimming(bool* trimming, CAMU_Port port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x000F0040;
	cmdbuf[1] = port;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*trimming = (bool) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetTrimmingParams(CAMU_Port port, s16 xStart, s16 yStart, s16 xEnd, s16 yEnd) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00100140;
	cmdbuf[1] = port;
	cmdbuf[2] = xStart;
	cmdbuf[3] = yStart;
	cmdbuf[4] = xEnd;
	cmdbuf[5] = yEnd;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_GetTrimmingParams(s16* xStart, s16* yStart, s16* xEnd, s16* yEnd, CAMU_Port port) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00110040;
	cmdbuf[1] = port;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*xStart = (s16) cmdbuf[2];
	*yStart = (s16) cmdbuf[3];
	*xEnd = (s16) cmdbuf[4];
	*yEnd = (s16) cmdbuf[5];
	return cmdbuf[1];
}

Result CAMU_SetTrimmingParamsCenter(CAMU_Port port, s16 trimWidth, s16 trimHeight, s16 camWidth, s16 camHeight) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00120140;
	cmdbuf[1] = port;
	cmdbuf[2] = trimWidth;
	cmdbuf[3] = trimHeight;
	cmdbuf[4] = camWidth;
	cmdbuf[5] = camHeight;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_Activate(CAMU_CameraSelect select) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00130040;
	cmdbuf[1] = select;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SwitchContext(CAMU_CameraSelect select, CAMU_Context context) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00140080;
	cmdbuf[1] = select;
	cmdbuf[2] = context;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetExposure(CAMU_CameraSelect select, s8 exposure) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00150080;
	cmdbuf[1] = select;
	cmdbuf[2] = exposure;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetWhiteBalance(CAMU_CameraSelect select, CAMU_WhiteBalance whiteBalance) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00160080;
	cmdbuf[1] = select;
	cmdbuf[2] = whiteBalance;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetWhiteBalanceWithoutBaseUp(CAMU_CameraSelect select, CAMU_WhiteBalance whiteBalance) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00170080;
	cmdbuf[1] = select;
	cmdbuf[2] = whiteBalance;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetSharpness(CAMU_CameraSelect select, s8 sharpness) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00180080;
	cmdbuf[1] = select;
	cmdbuf[2] = sharpness;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetAutoExposure(CAMU_CameraSelect select, bool autoExposure) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00190080;
	cmdbuf[1] = select;
	cmdbuf[2] = autoExposure;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_IsAutoExposure(bool* autoExposure, CAMU_CameraSelect select) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x001A0040;
	cmdbuf[1] = select;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*autoExposure = (bool) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetAutoWhiteBalance(CAMU_CameraSelect select, bool autoWhiteBalance) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x001B0080;
	cmdbuf[1] = select;
	cmdbuf[2] = autoWhiteBalance;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_IsAutoWhiteBalance(bool* autoWhiteBalance, CAMU_CameraSelect select) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x001C0040;
	cmdbuf[1] = select;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*autoWhiteBalance = (bool) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_FlipImage(CAMU_CameraSelect select, CAMU_Flip flip, CAMU_Context context) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x001D00C0;
	cmdbuf[1] = select;
	cmdbuf[2] = flip;
	cmdbuf[3] = context;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetDetailSize(CAMU_CameraSelect select, s16 width, s16 height, s16 cropX0, s16 cropY0, s16 cropX1, s16 cropY1, CAMU_Context context) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x001E0200;
	cmdbuf[1] = select;
	cmdbuf[2] = width;
	cmdbuf[3] = height;
	cmdbuf[4] = cropX0;
	cmdbuf[5] = cropY0;
	cmdbuf[6] = cropX1;
	cmdbuf[7] = cropY1;
	cmdbuf[8] = context;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetSize(CAMU_CameraSelect select, CAMU_Size size, CAMU_Context context) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x001F00C0;
	cmdbuf[1] = select;
	cmdbuf[2] = size;
	cmdbuf[3] = context;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetFrameRate(CAMU_CameraSelect select, CAMU_FrameRate frameRate) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00200080;
	cmdbuf[1] = select;
	cmdbuf[2] = frameRate;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetPhotoMode(CAMU_CameraSelect select, CAMU_PhotoMode photoMode) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00210080;
	cmdbuf[1] = select;
	cmdbuf[2] = photoMode;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetEffect(CAMU_CameraSelect select, CAMU_Effect effect, CAMU_Context context) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x002200C0;
	cmdbuf[1] = select;
	cmdbuf[2] = effect;
	cmdbuf[3] = context;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetContrast(CAMU_CameraSelect select, CAMU_Contrast contrast) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00230080;
	cmdbuf[1] = select;
	cmdbuf[2] = contrast;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetLensCorrection(CAMU_CameraSelect select, CAMU_LensCorrection lensCorrection) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00240080;
	cmdbuf[1] = select;
	cmdbuf[2] = lensCorrection;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetOutputFormat(CAMU_CameraSelect select, CAMU_OutputFormat format, CAMU_Context context) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x002500C0;
	cmdbuf[1] = select;
	cmdbuf[2] = format;
	cmdbuf[3] = context;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetAutoExposureWindow(CAMU_CameraSelect select, s16 x, s16 y, s16 width, s16 height) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00260140;
	cmdbuf[1] = select;
	cmdbuf[2] = x;
	cmdbuf[3] = y;
	cmdbuf[4] = width;
	cmdbuf[5] = height;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetAutoWhiteBalanceWindow(CAMU_CameraSelect select, s16 x, s16 y, s16 width, s16 height) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00270140;
	cmdbuf[1] = select;
	cmdbuf[2] = x;
	cmdbuf[3] = y;
	cmdbuf[4] = width;
	cmdbuf[5] = height;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetNoiseFilter(CAMU_CameraSelect select, bool noiseFilter) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00280080;
	cmdbuf[1] = select;
	cmdbuf[2] = noiseFilter;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SynchronizeVsyncTiming(CAMU_CameraSelect select1, CAMU_CameraSelect select2) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00290080;
	cmdbuf[1] = select1;
	cmdbuf[2] = select2;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_GetLatestVsyncTiming(s64* timing, CAMU_Port port, u32 past) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x002A0080;
	cmdbuf[1] = port;
	cmdbuf[2] = past;
	cmdbuf[49] = (past << 17) | 2;
	cmdbuf[50] = (u32) timing;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_GetStereoCameraCalibrationData(CAMU_StereoCameraCalibrationData* data) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x002B0000;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*data = *(CAMU_StereoCameraCalibrationData*) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetStereoCameraCalibrationData(CAMU_StereoCameraCalibrationData data) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x002C0400;
	*(CAMU_StereoCameraCalibrationData*) cmdbuf[1] = data;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_WriteRegisterI2c(CAMU_CameraSelect select, u16 addr, u16 data) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x002D00C0;
	cmdbuf[1] = select;
	cmdbuf[2] = addr;
	cmdbuf[3] = data;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_WriteMcuVariableI2c(CAMU_CameraSelect select, u16 addr, u16 data) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x002E00C0;
	cmdbuf[1] = select;
	cmdbuf[2] = addr;
	cmdbuf[3] = data;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_ReadRegisterI2cExclusive(u16* data, CAMU_CameraSelect select, u16 addr) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x002F0080;
	cmdbuf[1] = select;
	cmdbuf[2] = addr;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*data = (u16) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_ReadMcuVariableI2cExclusive(u16* data, CAMU_CameraSelect select, u16 addr) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00300080;
	cmdbuf[1] = select;
	cmdbuf[2] = addr;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*data = (u16) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetImageQualityCalibrationData(CAMU_ImageQualityCalibrationData data) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00310180;
	*(CAMU_ImageQualityCalibrationData*) cmdbuf[1] = data;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_GetImageQualityCalibrationData(CAMU_ImageQualityCalibrationData* data) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00320000;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*data = *(CAMU_ImageQualityCalibrationData*) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetPackageParameterWithoutContext(CAMU_PackageParameterCameraSelect param) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x003302C0;
	*(CAMU_PackageParameterCameraSelect*) cmdbuf[1] = param;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetPackageParameterWithContext(CAMU_PackageParameterContext param) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00340140;
	*(CAMU_PackageParameterContext*) cmdbuf[1] = param;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetPackageParameterWithContextDetail(CAMU_PackageParameterContextDetail param) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x003501C0;
	*(CAMU_PackageParameterContextDetail*) cmdbuf[1] = param;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_GetSuitableY2rStandardCoefficient(Y2R_StandardCoefficient* coefficient) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00360000;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*coefficient = (Y2R_StandardCoefficient) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_PlayShutterSound(CAMU_ShutterSoundType sound) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00380040;
	cmdbuf[1] = sound;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_DriverInitialize() {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x00390000;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_DriverFinalize() {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x003A0000;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_GetActivatedCamera(CAMU_CameraSelect* select) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x003B0000;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*select = (CAMU_CameraSelect) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_GetSleepCamera(CAMU_CameraSelect* select) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x003C0000;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	*select = (CAMU_CameraSelect) cmdbuf[2];
	return cmdbuf[1];
}

Result CAMU_SetSleepCamera(CAMU_CameraSelect select) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x003D0040;
	cmdbuf[1] = select;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result CAMU_SetBrightnessSynchronization(bool brightnessSynchronization) {
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = 0x003E0040;
	cmdbuf[1] = brightnessSynchronization;

	if ((ret = svcSendSyncRequest(camHandle)) != 0) return ret;
	return cmdbuf[1];
}


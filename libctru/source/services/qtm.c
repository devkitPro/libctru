#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/qtm.h>
#include <3ds/ipc.h>
#include <math.h>

Handle qtmHandle;
static int qtmRefCount;

bool qtmCheckServicesRegistered(void)
{
	bool registered = false;
	return R_SUCCEEDED(srvIsServiceRegistered(&registered, "qtm:u")) && registered;
}

Result qtmInit(QtmServiceName serviceName)
{
	const char *name = NULL;
	switch (serviceName)
	{
		default:
		case QTM_SERVICE_USER:
			name = "qtm:u";
			break;
		case QTM_SERVICE_SYSTEM:
			name = "qtm:s";
			break;
		case QTM_SERVICE_SYSTEM_PROCESS:
			name = "qtm:sp";
			break;
	}

	if (AtomicPostIncrement(&qtmRefCount)) return 0;
	Result res = srvGetServiceHandle(&qtmHandle, name);

	if (R_FAILED(res))
	{
		qtmHandle = 0;
		AtomicDecrement(&qtmRefCount);
	}

	return res;
}

void qtmExit(void)
{
	if (AtomicDecrement(&qtmRefCount)) return;
	svcCloseHandle(qtmHandle);
	qtmHandle = 0;
}

bool qtmIsInitialized(void)
{
	// Use qtmHandle instead of qtmRefCount in the event user
	// wants to bypass qtmInit and use *qtmGetSessionHandle() instead
	// (e.g. stolen session).
	return qtmHandle != 0;
}

Handle *qtmGetSessionHandle(void)
{
	return &qtmHandle;
}

Result QTMU_GetRawTrackingDataEx(QtmRawTrackingData *outData, s64 predictionTimePointOrZero)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1, 2, 0); // 0x10080
	memcpy(&cmdbuf[1], &predictionTimePointOrZero, 8);

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	memcpy(outData, &cmdbuf[2], sizeof(QtmRawTrackingData));

	return cmdbuf[1];
}

Result QTMU_GetTrackingDataEx(QtmTrackingData *outData, s64 predictionTimePointOrZero)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2, 2, 0); // 0x20080
	memcpy(&cmdbuf[1], &predictionTimePointOrZero, 8);

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	memcpy(outData, &cmdbuf[2], sizeof(QtmTrackingData));

	return cmdbuf[1];
}

float qtmComputeFovX(const QtmTrackingData *data)
{
	return 2.0f * atanf(data->eyeWorldCoordinates[QTM_EYE_LEFT][0] / data->eyeCameraCoordinates[QTM_EYE_LEFT][0]);
}

float qtmComputeFovY(const QtmTrackingData *data)
{
	return 2.0f * atanf(data->eyeWorldCoordinates[QTM_EYE_LEFT][1] / data->eyeCameraCoordinates[QTM_EYE_LEFT][1]);
}

float qtmComputeInverseAspectRatio(const QtmTrackingData *data)
{
	return
		(data->eyeWorldCoordinates[QTM_EYE_LEFT][1]  * data->eyeCameraCoordinates[QTM_EYE_LEFT][0]) /
		(data->eyeCameraCoordinates[QTM_EYE_LEFT][1] * data->eyeWorldCoordinates[QTM_EYE_LEFT][0]);
}

float qtmComputeHeadTiltAngle(const QtmTrackingData *data)
{
	return atan2f(
		data->eyeCameraCoordinates[QTM_EYE_RIGHT][1] - data->eyeCameraCoordinates[QTM_EYE_LEFT][1],
		data->eyeCameraCoordinates[QTM_EYE_RIGHT][0] - data->eyeCameraCoordinates[QTM_EYE_LEFT][0]
	);
}

float qtmEstimateEyeToCameraDistance(const QtmTrackingData *data)
{
	float fovX = qtmComputeFovX(data);
	float pxDistNorm = hypotf(
		data->eyeWorldCoordinates[QTM_EYE_RIGHT][0] - data->eyeWorldCoordinates[QTM_EYE_LEFT][0],
		data->eyeWorldCoordinates[QTM_EYE_RIGHT][1] - data->eyeWorldCoordinates[QTM_EYE_LEFT][1]
	);

	return 31.0f / tanf(0.5f * (pxDistNorm * fovX));
}

Result QTMU_EnableManualIrLedControl(void)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3, 0, 0); // 0x30000

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMU_DisableManualIrLedControl(void)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4, 0, 0); // 0x40000

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMU_SetIrLedStatus(bool on)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5, 1, 0); // 0x50040
	cmdbuf[1] = on;

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result qtmClearIrLedOverrides(void)
{
	QTMU_EnableManualIrLedControl();
	return QTMU_DisableManualIrLedControl();
}

Result QTMU_IsCurrentAppBlacklisted(bool *outBlacklisted)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6, 0, 0); // 0x60000

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	*outBlacklisted = (bool)(cmdbuf[2] & 1);

	return cmdbuf[1];
}

Result QTMS_SetCentralBarrierPosition(float position)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x401, 1, 0); // 0x4010040
	memcpy(&cmdbuf[1], &position, sizeof(float));

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMS_GetCameraLuminance(float *outLuminanceLux)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x402, 0, 0); // 0x4020000

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	memcpy(outLuminanceLux, &cmdbuf[2], sizeof(float));

	return cmdbuf[1];
}

Result QTMS_EnableAutoBarrierControl(void)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x403, 0, 0); // 0x4030000

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMS_DisableAutoBarrierControl(void)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x404, 0, 0); // 0x4040000

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMS_SetBarrierPosition(u8 position)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x405, 1, 0); // 0x4050040
	cmdbuf[1] = position;

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMS_GetCurrentBarrierPosition(u8 *outPosition)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x406, 0, 0); // 0x4060000

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	*outPosition = (u8)cmdbuf[2];

	return cmdbuf[1];
}

Result QTMS_SetIrLedStatusOverride(bool on)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x407, 1, 0); // 0x4070040
	cmdbuf[1] = on;

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMS_SetCalibrationData(const QtmCalibrationData *cal, bool saveCalToCfg)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x408, 7, 0); // 0x40801C0

	memcpy(&cmdbuf[1], cal, sizeof(QtmCalibrationData));
	cmdbuf[7] = saveCalToCfg;

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMS_GetQtmStatus(QtmStatus *outQtmStatus)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x409, 0, 0); // 0x4090000

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	*outQtmStatus = (QtmStatus)cmdbuf[2];

	return cmdbuf[1];
}

Result QTMS_SetQtmStatus(QtmStatus qtmStatus)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40A, 1, 0); // 0x40A0040
	cmdbuf[1] = qtmStatus;

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMSP_NotifyTopLcdModeChange(u8 newMode)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x801, 1, 0); // 0x8010040
	cmdbuf[1] = newMode;

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMSP_NotifyTopLcdPowerOn(void)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x802, 0, 0); // 0x8020000

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

Result QTMSP_IsExpanderInUse(bool *outInUse)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x803, 0, 0); // 0x8040000

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	*outInUse = (bool)(cmdbuf[2] & 1);

	return cmdbuf[1];
}

Result QTMSP_NotifyTopLcdPowerOff(void)
{
	Result res = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x804, 0, 0); // 0x8040000

	res = svcSendSyncRequest(qtmHandle);
	if (R_FAILED(res)) return res;

	return cmdbuf[1];
}

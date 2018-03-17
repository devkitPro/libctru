#include <math.h>
#include <3ds/types.h>
#include <3ds/ndsp/ndsp.h>
#include <3ds/ndsp/channel.h>

#define Fs NDSP_SAMPLE_RATE

bool ndspChnIirMonoSetParamsLowPassFilter(int id, float f0)
{
	const float w0 = 2.f * M_PI * f0 / Fs;

	const float a0 = 1.f;
	const float a1 = 1.f - expf(-w0);
	const float b0 = expf(-w0);

	return ndspChnIirMonoSetParamsCustomFilter(id, a0, a1, b0);
}

bool ndspChnIirMonoSetParamsHighPassFilter(int id, float f0)
{
	const float w0 = 2.f * M_PI * (0.5f - f0 / Fs);

	const float a0 = 1.f;
	const float a1 = 1.f - expf(-w0);
	const float b0 = -expf(-w0);

	return ndspChnIirMonoSetParamsCustomFilter(id, a0, a1, b0);
}

bool ndspChnIirBiquadSetParamsLowPassFilter(int id, float f0, float Q)
{
	const float w0 = 2.f * M_PI * f0 / Fs;
	const float a = sinf(w0) / (2.f * Q);

	const float a0 = 1.f + a;
	const float a1 = -2.f * cosf(w0);
	const float a2 = 1.f - a;
	const float b0 = 0.5f * (1.f - cosf(w0));
	const float b1 =        (1.f - cosf(w0));
	const float b2 = 0.5f * (1.f - cosf(w0));

	return ndspChnIirBiquadSetParamsCustomFilter(id, a0, a1, a2, b0, b1, b2);
}

bool ndspChnIirBiquadSetParamsHighPassFilter(int id, float f0, float Q)
{
	const float w0 = 2.f * M_PI * f0 / Fs;
	const float a = sinf(w0) / (2.f * Q);

	const float a0 = 1.f + a;
	const float a1 = -2.f * cosf(w0);
	const float a2 = 1.f - a;
	const float b0 = 0.5f * (1.f + cosf(w0));
	const float b1 =       -(1.f + cosf(w0));
	const float b2 = 0.5f * (1.f + cosf(w0));

	return ndspChnIirBiquadSetParamsCustomFilter(id, a0, a1, a2, b0, b1, b2);
}

bool ndspChnIirBiquadSetParamsBandPassFilter(int id, float f0, float Q)
{
	const float w0 = 2.f * M_PI * f0 / Fs;
	const float a = sinf(w0) / (2.f * Q);

	const float a0 = 1.f + a;
	const float a1 = -2.f * cosf(w0);
	const float a2 = 1.f - a;
	const float b0 = a;
	const float b1 = 0.f;
	const float b2 = a;

	return ndspChnIirBiquadSetParamsCustomFilter(id, a0, a1, a2, b0, b1, b2);
}

bool ndspChnIirBiquadSetParamsNotchFilter(int id, float f0, float Q)
{
	const float w0 = 2.f * M_PI * f0 / Fs;
	const float a = sinf(w0) / (2.f * Q);

	const float a0 = 1.f + a;
	const float a1 = -2.f * cosf(w0);
	const float a2 = 1.f - a;
	const float b0 = 1.f;
	const float b1 = -2.f * cosf(w0);
	const float b2 = 1.f;

	return ndspChnIirBiquadSetParamsCustomFilter(id, a0, a1, a2, b0, b1, b2);
}

bool ndspChnIirBiquadSetParamsPeakingEqualizer(int id, float f0, float Q, float gain)
{
	const float w0 = 2.f * M_PI * f0 / Fs;
	const float a = sinf(w0) / (2.f * Q);

	const float a0 = 1.f + a*gain;
	const float a1 = -2.f * cosf(w0);
	const float a2 = 1.f - a*gain;
	const float b0 = 1.f + a*gain;
	const float b1 = -2.f * cosf(w0);
	const float b2 = 1.f - a*gain;

	return ndspChnIirBiquadSetParamsCustomFilter(id, a0, a1, a2, b0, b1, b2);
}

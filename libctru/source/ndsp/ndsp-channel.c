#include "ndsp-internal.h"
#include <3ds/ndsp/channel.h>

enum
{
	CFLAG_INITPARAMS    = BIT(0),
	CFLAG_SYNCCOUNT     = BIT(1),
	CFLAG_PLAYSTATUS    = BIT(2),
	CFLAG_INTERPTYPE    = BIT(3),
	CFLAG_IIRFILTERTYPE = BIT(4),
	CFLAG_RATE          = BIT(5),
	CFLAG_MIX           = BIT(6),
	CFLAG_ADPCMCOEFS    = BIT(7),
};

typedef struct
{
	u32 flags;

	LightLock lock;
	u16 syncCount, waveBufSeqPos;
	u32 samplePos;

	ndspWaveBuf* waveBuf;
	u16 wavBufCount, wavBufIdNext;

	bool playing;
	u8 interpType, iirFilterType;

	u16 format;
	u16 wavBufSeq;

	float rate;
	float mix[12];

	u16 adpcmCoefs[16];

} ndspChnSt;

static ndspChnSt ndspChn[24];

void ndspChnReset(int id)
{
	ndspChnSt* chn = &ndspChn[id];
	LightLock_Lock(&chn->lock);
	chn->flags = ~0;
	chn->syncCount = 1;
	chn->waveBufSeqPos = 0;
	chn->samplePos = 0;
	chn->waveBuf = NULL;
	chn->wavBufCount = 0;
	chn->wavBufIdNext = 0;
	chn->wavBufSeq = 0;
	chn->playing = false;
	chn->interpType = 0;
	chn->iirFilterType = 0;
	chn->format = NDSP_FORMAT_PCM16;
	chn->rate = 1.0f;
	chn->mix[0] = chn->mix[1] = 1.0f;
	memset(&chn->mix[2], 0, 14*sizeof(float));
	LightLock_Unlock(&chn->lock);
}

void ndspChnInitParams(int id)
{
	ndspChnSt* chn = &ndspChn[id];
	LightLock_Lock(&chn->lock);
	chn->flags |= CFLAG_INITPARAMS;
	LightLock_Unlock(&chn->lock);
}

bool ndspChnIsPlaying(int id)
{
	return ndspChn[id].playing;
}

u32 ndspChnGetSamplePos(int id)
{
	return ndspChn[id].samplePos;
}

u16 ndspChnGetWaveBufSeq(int id)
{
	return ndspChn[id].waveBufSeqPos;
}

void ndspChnSetFormat(int id, u16 format)
{
	ndspChn[id].format = format;
}

void ndspChnSetInterp(int id, ndspInterpType type)
{
	ndspChnSt* chn = &ndspChn[id];
	LightLock_Lock(&chn->lock);
	chn->interpType = type;
	chn->flags |= CFLAG_INTERPTYPE;
	LightLock_Unlock(&chn->lock);
}

void ndspChnSetRate(int id, float rate)
{
	ndspChnSt* chn = &ndspChn[id];
	LightLock_Lock(&chn->lock);
	chn->rate = rate/32728.0f;
	chn->flags |= CFLAG_RATE;
	LightLock_Unlock(&chn->lock);
}

void ndspChnSetMix(int id, float mix[12])
{
	ndspChnSt* chn = &ndspChn[id];
	LightLock_Lock(&chn->lock);
	memcpy(&chn->mix, mix, sizeof(ndspChn[id].mix));
	chn->flags |= CFLAG_MIX;
	LightLock_Unlock(&chn->lock);
}

void ndspChnSetAdpcmCoefs(int id, u16 coefs[16])
{
	ndspChnSt* chn = &ndspChn[id];
	LightLock_Lock(&chn->lock);
	memcpy(&chn->adpcmCoefs, coefs, sizeof(ndspChn[id].adpcmCoefs));
	chn->flags |= CFLAG_ADPCMCOEFS;
	LightLock_Unlock(&chn->lock);
}

void ndspChnWaveBufClear(int id)
{
	ndspChnSt* chn = &ndspChn[id];
	LightLock_Lock(&chn->lock);
	chn->waveBuf = NULL;
	chn->waveBufSeqPos = 0;
	chn->wavBufCount = 0;
	chn->wavBufIdNext = 0;
	chn->wavBufSeq = 0;
	chn->playing = false;
	chn->syncCount ++;
	chn->flags |= CFLAG_SYNCCOUNT | CFLAG_PLAYSTATUS;
	LightLock_Unlock(&chn->lock);
}

void ndspChnWaveBufAdd(int id, ndspWaveBuf* buf)
{
	ndspChnSt* chn = &ndspChn[id];
	if (!buf->nsamples) return;

	LightLock_Lock(&chn->lock);
	buf->next = NULL;
	buf->status = NDSP_WBUF_QUEUED;
	ndspWaveBuf* cb = chn->waveBuf;

	if (cb)
	{
		while (cb->next) cb = cb->next;
		cb->next = buf;
	} else
		chn->waveBuf = buf;

	u16 seq = chn->wavBufSeq;
	if (!seq) seq = 1;
	buf->sequence_id = seq;
	chn->wavBufSeq = seq + 1;

	LightLock_Unlock(&chn->lock);
}

void ndspChnIirMonoSetEnable(int id, bool enable)
{
	ndspChnSt* chn = &ndspChn[id];
	LightLock_Lock(&chn->lock);
	u16 f = chn->iirFilterType &~ BIT(0);
	if (enable) f |= BIT(0);
	chn->iirFilterType = f;
	chn->flags |= CFLAG_IIRFILTERTYPE;
	LightLock_Unlock(&chn->lock);
}

void ndspChnIirBiquadSetEnable(int id, bool enable)
{
	ndspChnSt* chn = &ndspChn[id];
	LightLock_Lock(&chn->lock);
	u16 f = chn->iirFilterType &~ BIT(1);
	if (enable) f |= BIT(1);
	chn->iirFilterType = f;
	chn->flags |= CFLAG_IIRFILTERTYPE;
	LightLock_Unlock(&chn->lock);
}

void ndspiInitChn(void)
{
	int i;
	for (i = 0; i < 24; i ++)
	{
		LightLock_Init(&ndspChn[i].lock);
		ndspChnReset(i);
	}
}

void ndspiDirtyChn(void)
{
	int i;
	for (i = 0; i < 24; i ++)
		ndspChn[i].flags |= ~CFLAG_INITPARAMS;
}

void ndspiUpdateChn(void)
{
	int i;
	for (i = 0; i < 24; i ++)
	{
		ndspChnSt* chn = &ndspChn[i];
		DspChnStruct* st = ndspiGetChnStruct(i);
		LightLock_Lock(&chn->lock);

		u32 flags = chn->flags;
		u32 stflags = st->flags;

		if (flags & CFLAG_INITPARAMS)
			stflags |= 0x20000000;

		if (flags & CFLAG_MIX)
		{
			memcpy(st->mix, chn->mix, sizeof(st->mix));
			stflags |= 0xE000000;
		}

		if (flags & CFLAG_RATE)
		{
			st->rate = chn->rate;
			stflags |= 0x40000;
			if (chn->interpType == 0)
				flags |= CFLAG_INTERPTYPE;
		}

		if (flags & CFLAG_IIRFILTERTYPE)
		{
			st->iirFilterType = chn->iirFilterType;
			stflags |= 0x400000;
		}

		// TODO: IIR filter coefficent update

		if (flags & CFLAG_INTERPTYPE)
		{
			st->rim[0] = chn->interpType;
			if (chn->interpType == NDSP_INTERP_POLYPHASE)
			{
				if (chn->rate <= 1.0f)
					st->rim[1] = NDSP_INTERP_NONE;
				else if (chn->rate <= (4.0f/3))
					st->rim[1] = NDSP_INTERP_LINEAR;
				else
					st->rim[1] = NDSP_INTERP_POLYPHASE;
			} else
				st->rim[1] = NDSP_INTERP_LINEAR;
			stflags |= 0x20000;
		}

		if (flags & CFLAG_ADPCMCOEFS)
		{
			memcpy(ndspiGetChnAdpcmCoefs(i), chn->adpcmCoefs, sizeof(chn->adpcmCoefs));
			stflags |= 4;
		}

		// Do wavebuf stuff
		int wvcount = chn->wavBufCount;
		ndspWaveBuf* wb = chn->waveBuf;
		if (wb && !chn->playing)
		{
			chn->playing = true;
			flags |= CFLAG_PLAYSTATUS;
		}
		while (wvcount && wb)
		{
			wb = wb->next;
			wvcount--;
		}

		int j;
		for (j = chn->wavBufCount; wb && j < 5; j ++)
		{
			if (chn->wavBufCount == 0)
			{
				// This is the first buffer - set it up
				wb->status = NDSP_WBUF_PLAYING;
				chn->wavBufIdNext = 0;
				st->seqId = wb->sequence_id;
				st->sampleCount = ndspiRotateVal(wb->nsamples);
				st->paddr = ndspiRotateVal(osConvertVirtToPhys(wb->data_vaddr));
				st->cntFlags = chn->format;
				st->moreFlags = (st->moreFlags &~ BIT(1)) | (wb->looping ? BIT(1) : 0);
				st->unknown = 0;
				if ((chn->format & NDSP_ENCODING(3)) == NDSP_ENCODING(NDSP_ENCODING_ADPCM))
				{
					if (wb->adpcm_data)
					{
						st->adpcmData.index = wb->adpcm_data->index;
						st->adpcmData.history0 = wb->adpcm_data->history0;
						st->adpcmData.history1 = wb->adpcm_data->history1;
						st->moreFlags |= BIT(0);
					} else
						st->moreFlags &= ~BIT(0);
				}
				stflags |= 0x10 | 0x40200000;
			} else
			{
				// Queue the next buffer
				DspChnBuf* cbuf = &st->buffers[chn->wavBufIdNext];
				cbuf->seqId = wb->sequence_id;
				cbuf->paddr = ndspiRotateVal(osConvertVirtToPhys(wb->data_vaddr));
				cbuf->sampleCount = ndspiRotateVal(wb->nsamples);
				if (wb->adpcm_data)
				{
					cbuf->adpcmData.index = wb->adpcm_data->index;
					cbuf->adpcmData.history0 = wb->adpcm_data->history0;
					cbuf->adpcmData.history1 = wb->adpcm_data->history1;
					cbuf->hasAdpcmData = 1;
				} else
					cbuf->hasAdpcmData = 0;
				cbuf->looping = wb->looping ? 1 : 0;
				st->activeBuffers |= BIT(chn->wavBufIdNext);
				chn->wavBufIdNext = (chn->wavBufIdNext+1) & 3;
				stflags |= 0x80000;
			}
			wb = wb->next;
			chn->wavBufCount++;
		}

		if (flags & CFLAG_SYNCCOUNT)
		{
			st->syncCount = chn->syncCount;
			stflags |= 0x10000000;
		}

		if (flags & CFLAG_PLAYSTATUS)
		{
			u16 playStatus = st->playStatus &~ 0xFF;
			if (chn->playing)
				playStatus |= 1;
			st->playStatus = playStatus;
			stflags |= 0x10000;
		}

		chn->flags = 0;
		st->flags = stflags;

		LightLock_Unlock(&chn->lock);
	}
}

void ndspiReadChnState(void)
{
	int i;
	for (i = 0; i < 24; i ++)
	{
		ndspChnSt* chn   = &ndspChn[i];
		DspChnStatus* st = ndspiGetChnStatus(i);

		if (chn->syncCount == st->syncCount)
		{
			u16 seqId = st->curSeqId;
			chn->samplePos = ndspiRotateVal(st->samplePos);
			chn->waveBufSeqPos = seqId;

			if (st->flags & 0xFF00)
			{
				LightLock_Lock(&chn->lock);
				ndspWaveBuf* wb = chn->waveBuf;
				if (wb)
				{
					ndspWaveBuf* doneList = NULL;
					if (chn->wavBufCount)
					{
						while (wb->sequence_id != seqId)
						{
							chn->wavBufCount--;
							bool shouldBreak = seqId == 0 && (wb->sequence_id == st->lastSeqId || st->lastSeqId == 0);
							ndspWaveBuf* next = wb->next;
							wb->next = doneList;
							doneList = wb;
							wb = next;
							if (!wb || shouldBreak || chn->wavBufCount == 0)
								break;
						}
						if (wb)
							wb->status = NDSP_WBUF_PLAYING;
					}
					if (seqId == 0)
						chn->wavBufCount = 0;
					chn->waveBuf = wb;
					for (; doneList; doneList = doneList->next)
						doneList->status = NDSP_WBUF_DONE;
				}
				LightLock_Unlock(&chn->lock);
			}
			chn->playing = (st->flags & 0xFF) ? true : false;
		}
	}
}

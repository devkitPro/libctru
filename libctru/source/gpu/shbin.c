/*
  shdr.c _ Shader loader.
*/

#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/gpu/gpu.h>
#include <3ds/gpu/shbin.h>

//please don't feed this an invalid SHBIN
DVLB_s* DVLB_ParseFile(u32* shbinData, u32 shbinSize)
{
	if(!shbinData)return NULL;
	DVLB_s* ret=malloc(sizeof(DVLB_s));
	if(!ret)goto exit;

	//parse DVLB
	ret->numDVLE=shbinData[1];
	ret->DVLE=malloc(sizeof(DVLE_s)*ret->numDVLE);
	if(!ret->DVLE)goto clean1;

	//parse DVLP
	u32* dvlpData=&shbinData[2+ret->numDVLE];
	ret->DVLP.codeSize=dvlpData[3];
	ret->DVLP.codeData=&dvlpData[dvlpData[2]/4];
	ret->DVLP.opdescSize=dvlpData[5];
	ret->DVLP.opcdescData=(u32*)malloc(sizeof(u32)*ret->DVLP.opdescSize);
	if(!ret->DVLP.opcdescData)goto clean2;
	int i; for(i=0;i<ret->DVLP.opdescSize;i++)ret->DVLP.opcdescData[i]=dvlpData[dvlpData[4]/4+i*2];

	//parse DVLE
	for(i=0;i<ret->numDVLE;i++)
	{
		DVLE_s* dvle=&ret->DVLE[i];
		u32* dvleData=&shbinData[shbinData[2+i]/4];

		dvle->dvlp=&ret->DVLP;

		dvle->type=(dvleData[1]>>16)&0xFF;
		dvle->mergeOutmaps=(dvleData[1]>>24)&1;
		dvle->mainOffset=dvleData[2];
		dvle->endmainOffset=dvleData[3];

		if(dvle->type==GEOMETRY_SHDR)
		{
			dvle->gshMode=dvleData[5]&0xFF;
			dvle->gshFixedVtxStart=(dvleData[5]>>8)&0xFF;
			dvle->gshVariableVtxNum=(dvleData[5]>>16)&0xFF;
			dvle->gshFixedVtxNum=(dvleData[5]>>24)&0xFF;
		}

		dvle->constTableSize=dvleData[7];
		dvle->constTableData=(DVLE_constEntry_s*)&dvleData[dvleData[6]/4];

		dvle->outTableSize=dvleData[11];
		dvle->outTableData=(DVLE_outEntry_s*)&dvleData[dvleData[10]/4];

		dvle->uniformTableSize=dvleData[13];
		dvle->uniformTableData=(DVLE_uniformEntry_s*)&dvleData[dvleData[12]/4];

		dvle->symbolTableData=(char*)&dvleData[dvleData[14]/4];

		DVLE_GenerateOutmap(dvle);
	}

	goto exit;
	clean2:
		free(ret->DVLE);
	clean1:
		free(ret);
		ret=NULL;
	exit:
		return ret;
}

//TODO
void DVLB_Free(DVLB_s* dvlb)
{
	if(!dvlb)return;
	if(dvlb->DVLP.opcdescData)free(dvlb->DVLP.opcdescData);
	if(dvlb->DVLE)free(dvlb->DVLE);
	free(dvlb);
}

s8 DVLE_GetUniformRegister(DVLE_s* dvle, const char* name)
{
	if(!dvle || !name)return -1;

	int i;	DVLE_uniformEntry_s* u=dvle->uniformTableData;
	for(i=0;i<dvle->uniformTableSize;i++)
	{
		if(!strcmp(&dvle->symbolTableData[u->symbolOffset],name))return (s8)u->startReg-0x10;
		u++;
	}
	return -1;
}

void DVLE_GenerateOutmap(DVLE_s* dvle)
{
	if (!dvle) return;

	// Initialize outmap data
	memset(dvle->outmapData, 0x1F, sizeof(dvle->outmapData));
	dvle->outmapData[0] = 0;
	dvle->outmapMask    = 0;
	dvle->outmapMode    = 0;
	dvle->outmapClock   = 0;

	int i, j, k;
	for (i = 0; i < dvle->outTableSize; i ++)
	{
		int type = dvle->outTableData[i].type;
		int mask = dvle->outTableData[i].mask;
		int regID = dvle->outTableData[i].regID;
		u32* out = &dvle->outmapData[regID+1];

		if (!(dvle->outmapMask & BIT(regID)))
		{
			dvle->outmapMask |= BIT(regID);
			dvle->outmapData[0] ++;
		}

		int sem = 0x1F, num = 0;
		switch (type)
		{
			case RESULT_POSITION:   sem = 0x00; num = 4;                                                     break;
			case RESULT_NORMALQUAT: sem = 0x04; num = 4; dvle->outmapClock |= BIT(24);                       break;
			case RESULT_COLOR:      sem = 0x08; num = 4; dvle->outmapClock |= BIT(1);                        break;
			case RESULT_TEXCOORD0:  sem = 0x0C; num = 2; dvle->outmapClock |= BIT(8);  dvle->outmapMode = 1; break;
			case RESULT_TEXCOORD0W: sem = 0x10; num = 1; dvle->outmapClock |= BIT(16); dvle->outmapMode = 1; break;
			case RESULT_TEXCOORD1:  sem = 0x0E; num = 2; dvle->outmapClock |= BIT(9);  dvle->outmapMode = 1; break;
			case RESULT_TEXCOORD2:  sem = 0x16; num = 2; dvle->outmapClock |= BIT(10); dvle->outmapMode = 1; break;
			case RESULT_VIEW:       sem = 0x12; num = 3; dvle->outmapClock |= BIT(24);                       break;
			default: continue;
		}

		for (j = 0, k = 0; j < 4 && k < num; j ++)
		{
			if (mask & BIT(j))
			{
				*out &= ~(0xFF << (j*8));
				*out |= (sem++) << (j*8);
				k ++;
				if (type==RESULT_POSITION && k==3)
					dvle->outmapClock |= BIT(0);
			}
		}
	}
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/GSP.h>
#include <ctr/GX.h>
#include <ctr/GPU.h>
#include <ctr/SHDR.h>
#include <ctr/svc.h>

//please don't feed this an invalid SHBIN
DVLB_s* SHDR_ParseSHBIN(u32* shbinData, u32 shbinSize)
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
	ret->DVLP.opcdescData=&dvlpData[dvlpData[4]/4];

	//parse DVLE
	int i;
	for(i=0;i<ret->numDVLE;i++)
	{
		DVLE_s* dvle=&ret->DVLE[i];
		u32* dvleData=&shbinData[shbinData[2+i]/4];

		dvle->type=(dvleData[1]>>16)&0xFF;
		dvle->mainOffset=dvleData[2];
		dvle->endmainOffset=dvleData[3];

		dvle->constTableSize=dvleData[7];
		dvle->constTableData=(DVLE_constEntry_s*)&dvleData[dvleData[6]/4];

		dvle->outTableSize=dvleData[11];
		dvle->outTableData=(DVLE_outEntry_s*)&dvleData[dvleData[10]/4];
	}

	goto exit;
	clean1:
		free(ret);
	exit:
		return ret;
}

//hm
static inline minu8(u8 a, u8 b)
{
	if(a<b)return a;
	return b;
}
static inline maxu8(u8 a, u8 b)
{
	if(a<b)return b;
	return a;
}

void DVLP_SendCode(DVLP_s* dvlp)
{
	if(!dvlp)return;

	GPUCMD_AddSingleParam(0x000F02CB, 0x00000000);

	int i;
	// for(i=0;i<dvlp->codeSize;i+=0x80)GPUCMD_Add(0x000F02CC, &dvlp->codeData[i], ((dvlp->codeSize-i)<0x80)?(dvlp->codeSize-i):0x80);
	for(i=0;i<dvlp->codeSize;i+=0x80)GPUCMD_Add(0x000F02CC, &dvlp->codeData[i], maxu8(minu8(dvlp->codeSize-i,0x80),0x37)); //not sure why, but anything smaller than 0x37 seems to break stuff atm...

	GPUCMD_AddSingleParam(0x000F02BF, 0x00000001);
}

void DVLP_SendOpDesc(DVLP_s* dvlp)
{
	if(!dvlp)return;

	GPUCMD_AddSingleParam(0x000F02D5, 0x00000000);

	u32 param[0x20];

	int i;
	//TODO : should probably preprocess this
	for(i=0;i<dvlp->opdescSize;i++)param[i]=dvlp->opcdescData[i*2];

	GPUCMD_Add(0x000F02D6, param, dvlp->opdescSize);
}

void DVLE_SendOutmap(DVLE_s* dvle)
{
	if(!dvle)return;

	u32 param[0x7]={0x1F1F1F1F,0x1F1F1F1F,0x1F1F1F1F,0x1F1F1F1F,
					0x1F1F1F1F,0x1F1F1F1F,0x1F1F1F1F};

	int i;
	//TODO : should probably preprocess this
	for(i=0;i<dvle->outTableSize;i++)
	{
		u32* out=&param[dvle->outTableData[i].regID];

		//desc could include masking/swizzling info not currently taken into account
		//also TODO : map out other output register values
		switch(dvle->outTableData[i].type)
		{
			case RESULT_POSITION: *out=0x03020100; break;
			case RESULT_COLOR: *out=0x0B0A0908; break;
		}
	}

	GPUCMD_Add(0x800F0050, param, 0x00000007);
}

//TODO
void SHDR_FreeDVLB(DVLB_s* dvlb)
{
	if(!dvlb)return;

}

/*
  shdr.c _ Shader loader.
*/

#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/gpu/gpu.h>
#include <3ds/gpu/shbin.h>

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
		dvle->mainOffset=dvleData[2];
		dvle->endmainOffset=dvleData[3];

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
void SHDR_FreeDVLB(DVLB_s* dvlb)
{
	if(!dvlb)return;

}

s8 SHDR_GetUniformRegister(DVLB_s* dvlb, const char* name, u8 programID)
{
	if(!dvlb || !name)return -1;

	DVLE_s* dvle=&dvlb->DVLE[programID];

	int i;	DVLE_uniformEntry_s* u=dvle->uniformTableData;
	for(i=0;i<dvle->uniformTableSize;i++)
	{
		if(!strcmp(&dvle->symbolTableData[u->symbolOffset],name))return (s8)u->startReg-0x10;
		u++;
	}
	return -1;
}

void DVLP_SendCode(DVLP_s* dvlp, DVLE_type type)
{
	if(!dvlp)return;
	
	GPU_SendShaderCode(type, dvlp->codeData, 0, dvlp->codeSize);
}

void DVLP_SendOpDesc(DVLP_s* dvlp, DVLE_type type)
{
	if(!dvlp)return;

	GPU_SendOperandDescriptors(type, dvlp->opcdescData, 0, dvlp->opdescSize);
}

void DVLE_GenerateOutmap(DVLE_s* dvle)
{
	if(!dvle)return;

	memset(dvle->outmapData, 0x1F, sizeof(dvle->outmapData));

	int i;
	u8 numAttr=0;
	u8 maxAttr=0;
	u8 attrMask=0;

	for(i=0;i<dvle->outTableSize;i++)
	{
		u32* out=&dvle->outmapData[dvle->outTableData[i].regID+1];
		u32 mask=0x00000000;
		u8 tmpmask=dvle->outTableData[i].mask;
		mask=(mask<<8)|((tmpmask&8)?0xFF:0x00);tmpmask<<=1;
		mask=(mask<<8)|((tmpmask&8)?0xFF:0x00);tmpmask<<=1;
		mask=(mask<<8)|((tmpmask&8)?0xFF:0x00);tmpmask<<=1;
		mask=(mask<<8)|((tmpmask&8)?0xFF:0x00);tmpmask<<=1;
		
		if(*out==0x1F1F1F1F)numAttr++;

		u32 val=0x1F1F1F1F;
		switch(dvle->outTableData[i].type)
		{
			case RESULT_POSITION: val=0x03020100; break;
			case RESULT_NORMALQUAT: val=0x07060504; break;
			case RESULT_COLOR: val=0x0B0A0908; break;
			case RESULT_TEXCOORD0: val=0x1F1F0D0C; break;
			case RESULT_TEXCOORD0W: val=0x10101010; break;
			case RESULT_TEXCOORD1: val=0x1F1F0F0E; break;
			case RESULT_TEXCOORD2: val=0x1F1F1716; break;
			case RESULT_VIEW: val=0x1F141312; break;
		}
		*out=((*out)&~mask)|(val&mask);

		attrMask|=1<<dvle->outTableData[i].regID;
		if(dvle->outTableData[i].regID+1>maxAttr)maxAttr=dvle->outTableData[i].regID+1;
	}

	dvle->outmapData[0]=numAttr;
	dvle->outmapMask=attrMask;
}

void DVLE_SendOutmap(DVLE_s* dvle)
{
	if(!dvle)return;

	u32 regOffset=(dvle->type==GEOMETRY_SHDR)?(-0x30):(0x0);

	if(dvle->type==VERTEX_SHDR)
	{
		GPUCMD_AddWrite(GPUREG_024A, dvle->outmapData[0]-1); //?
		GPUCMD_AddWrite(GPUREG_0251, dvle->outmapData[0]-1); //?
	}
	
	GPUCMD_AddWrite(GPUREG_VSH_OUTMAP_MASK+regOffset, dvle->outmapMask);
	GPU_SetShaderOutmap(dvle->outmapData);
}

void DVLE_SendConstants(DVLE_s* dvle)
{
	if(!dvle)return;

	u32 regOffset=(dvle->type==GEOMETRY_SHDR)?(-0x30):(0x0);

	u32 param[4];
	u32 rev[3];
	u8* rev8=(u8*)rev;

	int i;
	DVLE_constEntry_s* cnst=dvle->constTableData;
	for(i=0;i<dvle->constTableSize;i++,cnst++)
	{
		memcpy(&rev8[0], &cnst->data[0], 3);
		memcpy(&rev8[3], &cnst->data[1], 3);
		memcpy(&rev8[6], &cnst->data[2], 3);
		memcpy(&rev8[9], &cnst->data[3], 3);

		param[0x0]=(cnst->id)&0xFF;
		param[0x1]=rev[2];
		param[0x2]=rev[1];
		param[0x3]=rev[0];

		GPUCMD_AddIncrementalWrites(GPUREG_VSH_FLOATUNIFORM_CONFIG+regOffset, param, 4);
	}
}

void SHDR_UseProgram(DVLB_s* dvlb, u8 id)
{
	if(!dvlb || id>dvlb->numDVLE)return;
	DVLE_s* dvle=&dvlb->DVLE[id];

	u32 regOffset=(dvlb->DVLE[id].type==GEOMETRY_SHDR)?(-0x30):(0x0);

	GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG, 0x1, 0x00000000);
	GPUCMD_AddMaskedWrite(GPUREG_0244, 0x1, (dvlb->DVLE[id].type==GEOMETRY_SHDR)?0x1:0x0);

	DVLP_SendCode(&dvlb->DVLP, dvlb->DVLE[id].type);
	DVLP_SendOpDesc(&dvlb->DVLP, dvlb->DVLE[id].type);
	DVLE_SendConstants(dvle);

	GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG, 0x8, 0x00000000);
	GPUCMD_AddWrite(GPUREG_VSH_ENTRYPOINT-regOffset, 0x7FFF0000|(dvle->mainOffset&0xFFFF)); //set entrypoint

	GPUCMD_AddWrite(GPUREG_0252, 0x00000000); // gsh related ?

	DVLE_SendOutmap(dvle);

	//?
		GPUCMD_AddWrite(GPUREG_0064, 0x00000001);
		GPUCMD_AddWrite(GPUREG_006F, 0x00000703);
}

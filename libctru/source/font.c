#include <stdlib.h>
#include <stdio.h>
#include <3ds/font.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#include <3ds/result.h>
#include <3ds/services/apt.h>

CFNT_s* g_sharedFont;
static u32 sharedFontAddr;

Result fontEnsureMapped(void)
{
	if (g_sharedFont) return 0;
	Result res = 0;
	Handle hSharedFont = 0;

	res = APT_GetSharedFont(&hSharedFont, &sharedFontAddr);
	if (R_FAILED(res)) return res;

	// Map the shared font if it's not already mapped.
	res = svcMapMemoryBlock(hSharedFont, 0, MEMPERM_READ, MEMPERM_DONTCARE);
	svcCloseHandle(hSharedFont);
	if (R_FAILED(res) && res != 0xE0A01BF5)
		return res;

	g_sharedFont = (CFNT_s*)(sharedFontAddr+0x80);
	return 0;
}

void fontFixPointers(CFNT_s* font)
{
	font->finf.tglp = (TGLP_s*)((u32)(font->finf.tglp) + (u32) font);
	font->finf.tglp->sheetData = (u8*)((u32)(font->finf.tglp->sheetData) + (u32) font);

	font->finf.cmap = (CMAP_s*)((u32)(font->finf.cmap) + (u32) font);
	for (CMAP_s* cmap = font->finf.cmap; cmap->next; cmap = cmap->next)
		cmap->next = (CMAP_s*)((u32)(cmap->next) + (u32) font);

	font->finf.cwdh = (CWDH_s*)((u32)(font->finf.cwdh) + (u32) font);
	for (CWDH_s* cwdh = font->finf.cwdh; cwdh->next; cwdh = cwdh->next)
		cwdh->next = (CWDH_s*)((u32)(cwdh->next) + (u32) font);
}

int fontGlyphIndexFromCodePoint(CFNT_s* font, u32 codePoint)
{
	if (!font)
		font = g_sharedFont;
	if (!font)
		return -1;
	int ret = 0xFFFF;
	if (codePoint < 0x10000)
	{
		for (CMAP_s* cmap = font->finf.cmap; cmap; cmap = cmap->next)
		{
			if (codePoint < cmap->codeBegin || codePoint > cmap->codeEnd)
				continue;

			if (cmap->mappingMethod == CMAP_TYPE_DIRECT)
			{
				ret = cmap->indexOffset + (codePoint - cmap->codeBegin);
				break;
			}

			if (cmap->mappingMethod == CMAP_TYPE_TABLE)
			{
				ret = cmap->indexTable[codePoint - cmap->codeBegin];
				break;
			}

			int j;
			for (j = 0; j < cmap->nScanEntries; j ++)
				if (cmap->scanEntries[j].code == codePoint)
					break;
			if (j < cmap->nScanEntries)
			{
				ret = cmap->scanEntries[j].glyphIndex;
				break;
			}
		}
	}
	if (ret == 0xFFFF) // Bogus CMAP entry. Probably exist to save space by using TABLE mappings?
	{
		if (font->finf.alterCharIndex == 0xFFFF)
			return -1;
		else
			return font->finf.alterCharIndex;
	}
	return ret;
}

charWidthInfo_s* fontGetCharWidthInfo(CFNT_s* font, int glyphIndex)
{
	if (!font)
		font = g_sharedFont;
	if (!font)
		return NULL;
	charWidthInfo_s* info = NULL;
	for (CWDH_s* cwdh = font->finf.cwdh; cwdh && !info; cwdh = cwdh->next)
	{
		if (glyphIndex < cwdh->startIndex || glyphIndex > cwdh->endIndex)
			continue;
		info = &cwdh->widths[glyphIndex - cwdh->startIndex];
	}
	if (!info)
		info = &font->finf.defaultWidth;
	return info;
}

void fontCalcGlyphPos(fontGlyphPos_s* out, CFNT_s* font, int glyphIndex, u32 flags, float scaleX, float scaleY)
{
	if (!font)
		font = g_sharedFont;
	if (!font)
		return;
	FINF_s* finf = &font->finf;
	TGLP_s* tglp = finf->tglp;
	charWidthInfo_s* cwi = fontGetCharWidthInfo(font, glyphIndex);
	int charPerSheet = font->finf.tglp->nRows * font->finf.tglp->nLines;

	int sheetId = glyphIndex / charPerSheet;
	int glInSheet = glyphIndex % charPerSheet;
	out->sheetIndex = sheetId;
	out->xOffset = scaleX*cwi->left;
	out->xAdvance = scaleX*cwi->charWidth;
	out->width = scaleX*cwi->glyphWidth;

	int lineId = glInSheet / tglp->nRows;
	int rowId = glInSheet % tglp->nRows;

	float tx = (float)(rowId*(tglp->cellWidth+1)+1) / tglp->sheetWidth;
	float ty = 1.0f - (float)(lineId*(tglp->cellHeight+1)+1) / tglp->sheetHeight;
	float tw = (float)cwi->glyphWidth / tglp->sheetWidth;
	float th = (float)tglp->cellHeight / tglp->sheetHeight;
	out->texcoord.left = tx;
	out->texcoord.top = ty;
	out->texcoord.right = tx+tw;
	out->texcoord.bottom = ty-th;

	if (flags & GLYPH_POS_CALC_VTXCOORD)
	{
		float vx = out->xOffset;
		float vy = (flags & GLYPH_POS_AT_BASELINE) ? (scaleY*tglp->baselinePos) : 0;
		float vw = out->width;
		float vh = scaleY*tglp->cellHeight;
		if (flags & GLYPH_POS_Y_POINTS_UP)
		{
			vy = -(vh-vy);
			out->vtxcoord.left = vx;
			out->vtxcoord.top = vy+vh;
			out->vtxcoord.right = vx+vw;
			out->vtxcoord.bottom = vy;
		} else
		{
			vy = -vy;
			out->vtxcoord.left = vx;
			out->vtxcoord.top = vy;
			out->vtxcoord.right = vx+vw;
			out->vtxcoord.bottom = vy+vh;
		}
	}
}

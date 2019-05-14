/**
 * @file font.h
 * @brief Shared font support.
 */
#pragma once
#include <3ds/types.h>

///@name Data types
///@{

/// Character width information structure.
typedef struct
{
	s8 left;       ///< Horizontal offset to draw the glyph with.
	u8 glyphWidth; ///< Width of the glyph.
	u8 charWidth;  ///< Width of the character, that is, horizontal distance to advance.
} charWidthInfo_s;

/// Font texture sheet information.
typedef struct
{
	u8 cellWidth;    ///< Width of a glyph cell.
	u8 cellHeight;   ///< Height of a glyph cell.
	u8 baselinePos;  ///< Vertical position of the baseline.
	u8 maxCharWidth; ///< Maximum character width.

	u32 sheetSize; ///< Size in bytes of a texture sheet.
	u16 nSheets;   ///< Number of texture sheets.
	u16 sheetFmt;  ///< GPU texture format (GPU_TEXCOLOR).

	u16 nRows;  ///< Number of glyphs per row per sheet.
	u16 nLines; ///< Number of glyph rows per sheet.

	u16 sheetWidth;  ///< Texture sheet width.
	u16 sheetHeight; ///< Texture sheet height.
	u8* sheetData;   ///< Pointer to texture sheet data.
} TGLP_s;

/// Font character width information block type.
typedef struct tag_CWDH_s CWDH_s;

/// Font character width information block structure.
struct tag_CWDH_s
{
	u16 startIndex; ///< First Unicode codepoint the block applies to.
	u16 endIndex;   ///< Last Unicode codepoint the block applies to.
	CWDH_s* next;   ///< Pointer to the next block.

	charWidthInfo_s widths[0]; ///< Table of character width information structures.
};

/// Font character map methods.
enum
{
	CMAP_TYPE_DIRECT = 0, ///< Identity mapping.
	CMAP_TYPE_TABLE = 1,  ///< Mapping using a table.
	CMAP_TYPE_SCAN = 2,   ///< Mapping using a list of mapped characters.
};

/// Font character map type.
typedef struct tag_CMAP_s CMAP_s;

/// Font character map structure.
struct tag_CMAP_s
{
	u16 codeBegin;     ///< First Unicode codepoint the block applies to.
	u16 codeEnd;       ///< Last Unicode codepoint the block applies to.
	u16 mappingMethod; ///< Mapping method.
	u16 reserved;
	CMAP_s* next;      ///< Pointer to the next map.

	union
	{
		u16 indexOffset;   ///< For CMAP_TYPE_DIRECT: index of the first glyph.
		u16 indexTable[0]; ///< For CMAP_TYPE_TABLE: table of glyph indices.
		/// For CMAP_TYPE_SCAN: Mapping data.
		struct
		{
			u16 nScanEntries; ///< Number of pairs.
			/// Mapping pairs.
			struct
			{
				u16 code;       ///< Unicode codepoint.
				u16 glyphIndex; ///< Mapped glyph index.
			} scanEntries[0];
		};
	};
};

/// Font information structure.
typedef struct
{
	u32 signature;   ///< Signature (FINF).
	u32 sectionSize; ///< Section size.

	u8 fontType;                  ///< Font type
	u8 lineFeed;                  ///< Line feed vertical distance.
	u16 alterCharIndex;           ///< Glyph index of the replacement character.
	charWidthInfo_s defaultWidth; ///< Default character width information.
	u8 encoding;                  ///< Font encoding (?)

	TGLP_s* tglp; ///< Pointer to texture sheet information.
	CWDH_s* cwdh; ///< Pointer to the first character width information block.
	CMAP_s* cmap; ///< Pointer to the first character map.

	u8 height;  ///< Font height.
	u8 width;   ///< Font width.
	u8 ascent;  ///< Font ascent.
	u8 padding;
} FINF_s;

/// Font structure.
typedef struct
{
	u32 signature;  ///< Signature (CFNU).
	u16 endianness; ///< Endianness constant (0xFEFF).
	u16 headerSize; ///< Header size.
	u32 version;    ///< Format version.
	u32 fileSize;   ///< File size.
	u32 nBlocks;    ///< Number of blocks.

	FINF_s finf; ///< Font information.
} CFNT_s;

/// Font glyph position structure.
typedef struct
{
	int sheetIndex; ///< Texture sheet index to use to render the glyph.
	float xOffset;  ///< Horizontal offset to draw the glyph width.
	float xAdvance; ///< Horizontal distance to advance after drawing the glyph.
	float width;    ///< Glyph width.
	/// Texture coordinates to use to render the glyph.
	struct
	{
		float left, top, right, bottom;
	} texcoord;
	/// Vertex coordinates to use to render the glyph.
	struct
	{
		float left, top, right, bottom;
	} vtxcoord;
} fontGlyphPos_s;

/// Flags for use with fontCalcGlyphPos.
enum
{
	GLYPH_POS_CALC_VTXCOORD = BIT(0), ///< Calculates vertex coordinates in addition to texture coordinates.
	GLYPH_POS_AT_BASELINE   = BIT(1), ///< Position the glyph at the baseline instead of at the top-left corner.
	GLYPH_POS_Y_POINTS_UP   = BIT(2), ///< Indicates that the Y axis points up instead of down.
};

///@}

///@name Initialization and basic operations
///@{

/// Ensures the shared system font is mapped.
Result fontEnsureMapped(void);

/**
 * @brief Fixes the pointers internal to a just-loaded font
 * @param font Font to fix
 * @remark Should never be run on the system font, and only once on any other font.
 */
void fontFixPointers(CFNT_s* font);

/// Gets the currently loaded system font
static inline CFNT_s* fontGetSystemFont(void)
{
	extern CFNT_s* g_sharedFont;
	if (!g_sharedFont)
		fontEnsureMapped();
	return g_sharedFont;
}

/**
 * @brief Retrieves the font information structure of a font.
 * @param font Pointer to font structure. If NULL, the shared system font is used.
 */
static inline FINF_s* fontGetInfo(CFNT_s* font)
{
	if (!font)
		font = fontGetSystemFont();
	return &font->finf;
}

/**
 * @brief Retrieves the texture sheet information of a font.
 * @param font Pointer to font structure. If NULL, the shared system font is used.
 */
static inline TGLP_s* fontGetGlyphInfo(CFNT_s* font)
{
	if (!font)
		font = fontGetSystemFont();
	return fontGetInfo(font)->tglp;
}

/**
 * @brief Retrieves the pointer to texture data for the specified texture sheet.
 * @param font Pointer to font structure. If NULL, the shared system font is used.
 * @param sheetIndex Index of the texture sheet.
 */
static inline void* fontGetGlyphSheetTex(CFNT_s* font, int sheetIndex)
{
	if (!font)
		font = fontGetSystemFont();
	TGLP_s* tglp = fontGetGlyphInfo(font);
	return &tglp->sheetData[sheetIndex*tglp->sheetSize];
}

/**
 * @brief Retrieves the glyph index of the specified Unicode codepoint.
 * @param font Pointer to font structure. If NULL, the shared system font is used.
 * @param codePoint Unicode codepoint.
 */
int fontGlyphIndexFromCodePoint(CFNT_s* font, u32 codePoint);

/**
 * @brief Retrieves character width information of the specified glyph.
 * @param font Pointer to font structure. If NULL, the shared system font is used.
 * @param glyphIndex Index of the glyph.
 */
charWidthInfo_s* fontGetCharWidthInfo(CFNT_s* font, int glyphIndex);

/**
 * @brief Calculates position information for the specified glyph.
 * @param out Output structure in which to write the information.
 * @param font Pointer to font structure. If NULL, the shared system font is used.
 * @param glyphIndex Index of the glyph.
 * @param flags Calculation flags (see GLYPH_POS_* flags).
 * @param scaleX Scale factor to apply horizontally.
 * @param scaleY Scale factor to apply vertically.
 */
void fontCalcGlyphPos(fontGlyphPos_s* out, CFNT_s* font, int glyphIndex, u32 flags, float scaleX, float scaleY);

///@}

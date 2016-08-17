/**
 * @file shbin.h
 * @brief Shader binary support.
 */
#pragma once

#include <3ds/gpu/gpu.h>

/// DVLE type.
typedef enum{
	VERTEX_SHDR=GPU_VERTEX_SHADER,    ///< Vertex shader.
	GEOMETRY_SHDR=GPU_GEOMETRY_SHADER ///< Geometry shader.
}DVLE_type;

/// Constant type.
typedef enum{
	DVLE_CONST_BOOL=0x0,    ///< Bool.
	DVLE_CONST_u8=0x1,      ///< Unsigned 8-bit integer.
	DVLE_CONST_FLOAT24=0x2, ///< 24-bit float.
}DVLE_constantType;

/// Output attribute.
typedef enum{
	RESULT_POSITION = 0x0,   ///< Position.
	RESULT_NORMALQUAT = 0x1, ///< Normal Quaternion.
	RESULT_COLOR = 0x2,      ///< Color.
	RESULT_TEXCOORD0 = 0x3,  ///< Texture coordinate 0.
	RESULT_TEXCOORD0W = 0x4, ///< Texture coordinate 0 W.
	RESULT_TEXCOORD1 = 0x5,  ///< Texture coordinate 1.
	RESULT_TEXCOORD2 = 0x6,  ///< Texture coordinate 2.
	RESULT_VIEW = 0x8,       ///< View.
	RESULT_DUMMY = 0x9,      ///< Dummy attribute (used as passthrough for geometry shader input).
}DVLE_outputAttribute_t;

/// Geometry shader operation modes.
typedef enum
{
	GSH_POINT         = 0, ///< Point processing mode.
	GSH_VARIABLE_PRIM = 1, ///< Variable-size primitive processing mode.
	GSH_FIXED_PRIM    = 2, ///< Fixed-size primitive processing mode.
} DVLE_geoShaderMode;

/// DVLP data.
typedef struct{
	u32 codeSize;     ///< Code size.
	u32* codeData;    ///< Code data.
	u32 opdescSize;   ///< Operand description size.
	u32* opcdescData; ///< Operand description data.
}DVLP_s;

/// DVLE constant entry data.
typedef struct{
	u16 type;    ///< Constant type. See @ref DVLE_constantType
	u16 id;      ///< Constant ID.
	u32 data[4]; ///< Constant data.
}DVLE_constEntry_s;

/// DVLE output entry data.
typedef struct{
	u16 type;  ///< Output type. See @ref DVLE_outputAttribute_t
	u16 regID; ///< Output register ID.
	u8 mask;   ///< Output mask.
	u8 unk[3]; ///< Unknown.
}DVLE_outEntry_s;

/// DVLE uniform entry data.
typedef struct{
	u32 symbolOffset; ///< Symbol offset.
	u16 startReg;     ///< Start register.
	u16 endReg;       ///< End register.
}DVLE_uniformEntry_s;

/// DVLE data.
typedef struct{
	DVLE_type type;                        ///< DVLE type.
	bool mergeOutmaps;                     ///< true = merge vertex/geometry shader outmaps ('dummy' output attribute is present).
	DVLE_geoShaderMode gshMode;            ///< Geometry shader operation mode.
	u8 gshFixedVtxStart;                   ///< Starting float uniform register number for storing the fixed-size primitive vertex array.
	u8 gshVariableVtxNum;                  ///< Number of fully-defined vertices in the variable-size primitive vertex array.
	u8 gshFixedVtxNum;                     ///< Number of vertices in the fixed-size primitive vertex array.
	DVLP_s* dvlp;                          ///< Contained DVLPs.
	u32 mainOffset;                        ///< Offset of the start of the main function.
	u32 endmainOffset;                     ///< Offset of the end of the main function.
	u32 constTableSize;                    ///< Constant table size.
	DVLE_constEntry_s* constTableData;     ///< Constant table data.
	u32 outTableSize;                      ///< Output table size.
	DVLE_outEntry_s* outTableData;         ///< Output table data.
	u32 uniformTableSize;                  ///< Uniform table size.
	DVLE_uniformEntry_s* uniformTableData; ///< Uniform table data.
	char* symbolTableData;                 ///< Symbol table data.
	u8 outmapMask;                         ///< Output map mask.
	u32 outmapData[8];                     ///< Output map data.
	u32 outmapMode;                        ///< Output map mode.
	u32 outmapClock;                       ///< Output map attribute clock.
}DVLE_s;

/// DVLB data.
typedef struct{
	u32 numDVLE;  ///< DVLE count.
	DVLP_s DVLP;  ///< Primary DVLP.
	DVLE_s* DVLE; ///< Contained DVLE.
}DVLB_s;

/**
 * @brief Parses a shader binary.
 * @param shbinData Shader binary data.
 * @param shbinSize Shader binary size.
 * @return The parsed shader binary.
 */
DVLB_s* DVLB_ParseFile(u32* shbinData, u32 shbinSize);

/**
 * @brief Frees shader binary data.
 * @param dvlb DVLB to free.
 */
void DVLB_Free(DVLB_s* dvlb);

/**
 * @brief Gets a uniform register index from a shader.
 * @param dvle Shader to get the register from.
 * @param name Name of the register.
 * @return The uniform register index.
 */
s8 DVLE_GetUniformRegister(DVLE_s* dvle, const char* name);

/**
 * @brief Generates a shader output map.
 * @param dvle Shader to generate an output map for.
 */
void DVLE_GenerateOutmap(DVLE_s* dvle);

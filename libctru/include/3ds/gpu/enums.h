/**
 * @file enums.h
 * @brief GPU enumeration values.
 */
#pragma once

/// Creates a texture magnification filter parameter from a @ref GPU_TEXTURE_FILTER_PARAM
#define GPU_TEXTURE_MAG_FILTER(v) (((v)&0x1)<<1)
/// Creates a texture minification filter parameter from a @ref GPU_TEXTURE_FILTER_PARAM
#define GPU_TEXTURE_MIN_FILTER(v) (((v)&0x1)<<2)
/// Creates a texture mipmap filter parameter from a @ref GPU_TEXTURE_FILTER_PARAM
#define GPU_TEXTURE_MIP_FILTER(v) (((v)&0x1)<<24)
/// Creates a texture wrap S parameter from a @ref GPU_TEXTURE_WRAP_PARAM
#define GPU_TEXTURE_WRAP_S(v)     (((v)&0x3)<<12)
/// Creates a texture wrap T parameter from a @ref GPU_TEXTURE_WRAP_PARAM
#define GPU_TEXTURE_WRAP_T(v)     (((v)&0x3)<<8)
/// Creates a texture mode parameter from a @ref GPU_TEXTURE_MODE_PARAM
#define GPU_TEXTURE_MODE(v)       (((v)&0x7)<<28)
/// Texture parameter indicating ETC1 texture.
#define GPU_TEXTURE_ETC1_PARAM    BIT(5)
/// Texture parameter indicating shadow texture.
#define GPU_TEXTURE_SHADOW_PARAM  BIT(20)

/// Creates a combiner buffer write configuration.
#define GPU_TEV_BUFFER_WRITE_CONFIG(stage0, stage1, stage2, stage3) ((stage0) | ((stage1) << 1) | ((stage2) << 2) | ((stage3) << 3))

/// Texture filters.
typedef enum
{
	GPU_NEAREST = 0x0, ///< Nearest-neighbor interpolation.
	GPU_LINEAR  = 0x1, ///< Linear interpolation.
} GPU_TEXTURE_FILTER_PARAM;

/// Texture wrap modes.
typedef enum
{
	GPU_CLAMP_TO_EDGE   = 0x0, ///< Clamps to edge.
	GPU_CLAMP_TO_BORDER = 0x1, ///< Clamps to border.
	GPU_REPEAT          = 0x2, ///< Repeats texture.
	GPU_MIRRORED_REPEAT = 0x3, ///< Repeats with mirrored texture.
} GPU_TEXTURE_WRAP_PARAM;

/// Texture modes.
typedef enum
{
	GPU_TEX_2D          = 0x0, ///< 2D texture
	GPU_TEX_CUBE_MAP    = 0x1, ///< Cube map
	GPU_TEX_SHADOW_2D   = 0x2, ///< 2D Shadow texture
	GPU_TEX_PROJECTION  = 0x3, ///< Projection texture
	GPU_TEX_SHADOW_CUBE = 0x4, ///< Shadow cube map
	GPU_TEX_DISABLED    = 0x5, ///< Disabled
} GPU_TEXTURE_MODE_PARAM;

/// Supported texture units.
typedef enum
{
	GPU_TEXUNIT0 = 0x1, ///< Texture unit 0.
	GPU_TEXUNIT1 = 0x2, ///< Texture unit 1.
	GPU_TEXUNIT2 = 0x4, ///< Texture unit 2.
} GPU_TEXUNIT;

/// Supported texture formats.
typedef enum
{
	GPU_RGBA8    = 0x0, ///< 8-bit Red + 8-bit Green + 8-bit Blue + 8-bit Alpha
	GPU_RGB8     = 0x1, ///< 8-bit Red + 8-bit Green + 8-bit Blue
	GPU_RGBA5551 = 0x2, ///< 5-bit Red + 5-bit Green + 5-bit Blue + 1-bit Alpha
	GPU_RGB565   = 0x3, ///< 5-bit Red + 6-bit Green + 5-bit Blue
	GPU_RGBA4    = 0x4, ///< 4-bit Red + 4-bit Green + 4-bit Blue + 4-bit Alpha
	GPU_LA8      = 0x5, ///< 8-bit Luminance + 8-bit Alpha
	GPU_HILO8    = 0x6, ///< 8-bit Hi + 8-bit Lo
	GPU_L8       = 0x7, ///< 8-bit Luminance
	GPU_A8       = 0x8, ///< 8-bit Alpha
	GPU_LA4      = 0x9, ///< 4-bit Luminance + 4-bit Alpha
	GPU_L4       = 0xA, ///< 4-bit Luminance
	GPU_A4       = 0xB, ///< 4-bit Alpha
	GPU_ETC1     = 0xC, ///< ETC1 texture compression
	GPU_ETC1A4   = 0xD, ///< ETC1 texture compression + 4-bit Alpha
} GPU_TEXCOLOR;

// Texture faces.
typedef enum
{
	GPU_TEXFACE_2D = 0, // 2D face
	GPU_POSITIVE_X = 0, // +X face
	GPU_NEGATIVE_X = 1, // -X face
	GPU_POSITIVE_Y = 2, // +Y face
	GPU_NEGATIVE_Y = 3, // -Y face
	GPU_POSITIVE_Z = 4, // +Z face
	GPU_NEGATIVE_Z = 5, // -Z face
} GPU_TEXFACE;

/// Procedural texture clamp modes.
typedef enum
{
	GPU_PT_CLAMP_TO_ZERO   = 0, ///< Clamp to zero.
	GPU_PT_CLAMP_TO_EDGE   = 1, ///< Clamp to edge.
	GPU_PT_REPEAT          = 2, ///< Symmetrical repeat.
	GPU_PT_MIRRORED_REPEAT = 3, ///< Mirrored repeat.
	GPU_PT_PULSE           = 4, ///< Pulse.
} GPU_PROCTEX_CLAMP;

/// Procedural texture mapping functions.
typedef enum
{
	GPU_PT_U     = 0, ///< U
	GPU_PT_U2    = 1, ///< U2
	GPU_PT_V     = 2, ///< V
	GPU_PT_V2    = 3, ///< V2
	GPU_PT_ADD   = 4, ///< U+V
	GPU_PT_ADD2  = 5, ///< U2+V2
	GPU_PT_SQRT2 = 6, ///< sqrt(U2+V2)
	GPU_PT_MIN   = 7, ///< min
	GPU_PT_MAX   = 8, ///< max
	GPU_PT_RMAX  = 9, ///< rmax
} GPU_PROCTEX_MAPFUNC;

/// Procedural texture shift values.
typedef enum
{
	GPU_PT_NONE = 0, ///< No shift.
	GPU_PT_ODD  = 1, ///< Odd shift.
	GPU_PT_EVEN = 2, ///< Even shift.
} GPU_PROCTEX_SHIFT;

/// Procedural texture filter values.
typedef enum
{
	GPU_PT_NEAREST             = 0, ///< Nearest-neighbor
	GPU_PT_LINEAR              = 1, ///< Linear interpolation
	GPU_PT_NEAREST_MIP_NEAREST = 2, ///< Nearest-neighbor with mipmap using nearest-neighbor
	GPU_PT_LINEAR_MIP_NEAREST  = 3, ///< Linear interpolation with mipmap using nearest-neighbor
	GPU_PT_NEAREST_MIP_LINEAR  = 4, ///< Nearest-neighbor with mipmap using linear interpolation
	GPU_PT_LINEAR_MIP_LINEAR   = 5, ///< Linear interpolation with mipmap using linear interpolation
} GPU_PROCTEX_FILTER;

/// Procedural texture LUT IDs.
typedef enum
{
	GPU_LUT_NOISE    = 0, ///< Noise table
	GPU_LUT_RGBMAP   = 2, ///< RGB mapping function table
	GPU_LUT_ALPHAMAP = 3, ///< Alpha mapping function table
	GPU_LUT_COLOR    = 4, ///< Color table
	GPU_LUT_COLORDIF = 5, ///< Color difference table
} GPU_PROCTEX_LUTID;

/// Supported color buffer formats.
typedef enum
{
	GPU_RB_RGBA8    = 0, ///< 8-bit Red + 8-bit Green + 8-bit Blue + 8-bit Alpha
	GPU_RB_RGB8     = 1, ///< 8-bit Red + 8-bit Green + 8-bit Blue
	GPU_RB_RGBA5551 = 2, ///< 5-bit Red + 5-bit Green + 5-bit Blue + 1-bit Alpha
	GPU_RB_RGB565   = 3, ///< 5-bit Red + 6-bit Green + 5-bit Blue
	GPU_RB_RGBA4    = 4, ///< 4-bit Red + 4-bit Green + 4-bit Blue + 4-bit Alpha
} GPU_COLORBUF;

/// Supported depth buffer formats.
typedef enum
{
	GPU_RB_DEPTH16          = 0, ///< 16-bit Depth
	GPU_RB_DEPTH24          = 2, ///< 24-bit Depth
	GPU_RB_DEPTH24_STENCIL8 = 3, ///< 24-bit Depth + 8-bit Stencil
} GPU_DEPTHBUF;

/// Test functions.
typedef enum
{
	GPU_NEVER    = 0, ///< Never pass.
	GPU_ALWAYS   = 1, ///< Always pass.
	GPU_EQUAL    = 2, ///< Pass if equal.
	GPU_NOTEQUAL = 3, ///< Pass if not equal.
	GPU_LESS     = 4, ///< Pass if less than.
	GPU_LEQUAL   = 5, ///< Pass if less than or equal.
	GPU_GREATER  = 6, ///< Pass if greater than.
	GPU_GEQUAL   = 7, ///< Pass if greater than or equal.
} GPU_TESTFUNC;

/// Early depth test functions.
typedef enum
{
	GPU_EARLYDEPTH_GEQUAL  = 0, ///< Pass if greater than or equal.
	GPU_EARLYDEPTH_GREATER = 1, ///< Pass if greater than.
	GPU_EARLYDEPTH_LEQUAL  = 2, ///< Pass if less than or equal.
	GPU_EARLYDEPTH_LESS    = 3, ///< Pass if less than.
} GPU_EARLYDEPTHFUNC;

/// Gas depth functions.
typedef enum
{
	GPU_GAS_NEVER    = 0, ///< Never pass (0).
	GPU_GAS_ALWAYS   = 1, ///< Always pass (1).
	GPU_GAS_GREATER  = 2, ///< Pass if greater than (1-X).
	GPU_GAS_LESS     = 3, ///< Pass if less than (X).
} GPU_GASDEPTHFUNC;

/// Converts \ref GPU_TESTFUNC into \ref GPU_GASDEPTHFUNC.
#define GPU_MAKEGASDEPTHFUNC(n) (GPU_GASDEPTHFUNC)((0xAF02>>((int)(n)<<1))&3)

/// Scissor test modes.
typedef enum
{
	GPU_SCISSOR_DISABLE = 0, ///< Disable.
	GPU_SCISSOR_INVERT  = 1, ///< Exclude pixels inside the scissor box.
	// 2 is the same as 0
	GPU_SCISSOR_NORMAL  = 3, ///< Exclude pixels outside of the scissor box.
} GPU_SCISSORMODE;

/// Stencil operations.
typedef enum
{
	GPU_STENCIL_KEEP      = 0, ///< Keep old value. (old_stencil)
	GPU_STENCIL_ZERO      = 1, ///< Zero. (0)
	GPU_STENCIL_REPLACE   = 2, ///< Replace value. (ref)
	GPU_STENCIL_INCR      = 3, ///< Increment value. (old_stencil + 1 saturated to [0, 255])
	GPU_STENCIL_DECR      = 4, ///< Decrement value. (old_stencil - 1 saturated to [0, 255])
	GPU_STENCIL_INVERT    = 5, ///< Invert value. (~old_stencil)
	GPU_STENCIL_INCR_WRAP = 6, ///< Increment value. (old_stencil + 1)
	GPU_STENCIL_DECR_WRAP = 7, ///< Decrement value. (old_stencil - 1)
} GPU_STENCILOP;

/// Pixel write mask.
typedef enum
{
	GPU_WRITE_RED   = 0x01, ///< Write red.
	GPU_WRITE_GREEN = 0x02, ///< Write green.
	GPU_WRITE_BLUE  = 0x04, ///< Write blue.
	GPU_WRITE_ALPHA = 0x08, ///< Write alpha.
	GPU_WRITE_DEPTH = 0x10, ///< Write depth.

	GPU_WRITE_COLOR = 0x0F, ///< Write all color components.
	GPU_WRITE_ALL   = 0x1F, ///< Write all components.
} GPU_WRITEMASK;

/// Blend modes.
typedef enum
{
	GPU_BLEND_ADD              = 0, ///< Add colors.
	GPU_BLEND_SUBTRACT         = 1, ///< Subtract colors.
	GPU_BLEND_REVERSE_SUBTRACT = 2, ///< Reverse-subtract colors.
	GPU_BLEND_MIN              = 3, ///< Use the minimum color.
	GPU_BLEND_MAX              = 4, ///< Use the maximum color.
} GPU_BLENDEQUATION;

/// Blend factors.
typedef enum
{
	GPU_ZERO                     = 0,  ///< Zero.
	GPU_ONE                      = 1,  ///< One.
	GPU_SRC_COLOR                = 2,  ///< Source color.
	GPU_ONE_MINUS_SRC_COLOR      = 3,  ///< Source color - 1.
	GPU_DST_COLOR                = 4,  ///< Destination color.
	GPU_ONE_MINUS_DST_COLOR      = 5,  ///< Destination color - 1.
	GPU_SRC_ALPHA                = 6,  ///< Source alpha.
	GPU_ONE_MINUS_SRC_ALPHA      = 7,  ///< Source alpha - 1.
	GPU_DST_ALPHA                = 8,  ///< Destination alpha.
	GPU_ONE_MINUS_DST_ALPHA      = 9,  ///< Destination alpha - 1.
	GPU_CONSTANT_COLOR           = 10, ///< Constant color.
	GPU_ONE_MINUS_CONSTANT_COLOR = 11, ///< Constant color - 1.
	GPU_CONSTANT_ALPHA           = 12, ///< Constant alpha.
	GPU_ONE_MINUS_CONSTANT_ALPHA = 13, ///< Constant alpha - 1.
	GPU_SRC_ALPHA_SATURATE       = 14, ///< Saturated alpha.
} GPU_BLENDFACTOR;

/// Logical operations.
typedef enum
{
	GPU_LOGICOP_CLEAR         = 0,  ///< Clear.
	GPU_LOGICOP_AND           = 1,  ///< Bitwise AND.
	GPU_LOGICOP_AND_REVERSE   = 2,  ///< Reverse bitwise AND.
	GPU_LOGICOP_COPY          = 3,  ///< Copy.
	GPU_LOGICOP_SET           = 4,  ///< Set.
	GPU_LOGICOP_COPY_INVERTED = 5,  ///< Inverted copy.
	GPU_LOGICOP_NOOP          = 6,  ///< No operation.
	GPU_LOGICOP_INVERT        = 7,  ///< Invert.
	GPU_LOGICOP_NAND          = 8,  ///< Bitwise NAND.
	GPU_LOGICOP_OR            = 9,  ///< Bitwise OR.
	GPU_LOGICOP_NOR           = 10, ///< Bitwise NOR.
	GPU_LOGICOP_XOR           = 11, ///< Bitwise XOR.
	GPU_LOGICOP_EQUIV         = 12, ///< Equivalent.
	GPU_LOGICOP_AND_INVERTED  = 13, ///< Inverted bitwise AND.
	GPU_LOGICOP_OR_REVERSE    = 14, ///< Reverse bitwise OR.
	GPU_LOGICOP_OR_INVERTED   = 15, ///< Inverted bitwize OR.
} GPU_LOGICOP;

/// Fragment operation modes.
typedef enum
{
	GPU_FRAGOPMODE_GL      = 0, ///< OpenGL mode.
	GPU_FRAGOPMODE_GAS_ACC = 1, ///< Gas mode (?).
	GPU_FRAGOPMODE_SHADOW  = 3, ///< Shadow mode (?).
} GPU_FRAGOPMODE;

/// Supported component formats.
typedef enum
{
	GPU_BYTE          = 0, ///< 8-bit byte.
	GPU_UNSIGNED_BYTE = 1, ///< 8-bit unsigned byte.
	GPU_SHORT         = 2, ///< 16-bit short.
	GPU_FLOAT         = 3, ///< 32-bit float.
} GPU_FORMATS;

/// Cull modes.
typedef enum
{
	GPU_CULL_NONE      = 0, ///< Disabled.
	GPU_CULL_FRONT_CCW = 1, ///< Front, counter-clockwise.
	GPU_CULL_BACK_CCW  = 2, ///< Back, counter-clockwise.
} GPU_CULLMODE;

/// Creates a VBO attribute parameter from its index, size, and format.
#define GPU_ATTRIBFMT(i, n, f) (((((n)-1)<<2)|((f)&3))<<((i)*4))

/// Texture combiner sources.
typedef enum
{
	GPU_PRIMARY_COLOR            = 0x00, ///< Primary color.
	GPU_FRAGMENT_PRIMARY_COLOR   = 0x01, ///< Primary fragment color.
	GPU_FRAGMENT_SECONDARY_COLOR = 0x02, ///< Secondary fragment color.
	GPU_TEXTURE0                 = 0x03, ///< Texture unit 0.
	GPU_TEXTURE1                 = 0x04, ///< Texture unit 1.
	GPU_TEXTURE2                 = 0x05, ///< Texture unit 2.
	GPU_TEXTURE3                 = 0x06, ///< Texture unit 3.
	GPU_PREVIOUS_BUFFER          = 0x0D, ///< Previous buffer.
	GPU_CONSTANT                 = 0x0E, ///< Constant value.
	GPU_PREVIOUS                 = 0x0F, ///< Previous value.
} GPU_TEVSRC;

/// Texture RGB combiner operands.
typedef enum
{
	GPU_TEVOP_RGB_SRC_COLOR           = 0x00, ///< Source color.
	GPU_TEVOP_RGB_ONE_MINUS_SRC_COLOR = 0x01, ///< Source color - 1.
	GPU_TEVOP_RGB_SRC_ALPHA           = 0x02, ///< Source alpha.
	GPU_TEVOP_RGB_ONE_MINUS_SRC_ALPHA = 0x03, ///< Source alpha - 1.
	GPU_TEVOP_RGB_SRC_R               = 0x04, ///< Source red.
	GPU_TEVOP_RGB_ONE_MINUS_SRC_R     = 0x05, ///< Source red - 1.
	GPU_TEVOP_RGB_0x06                = 0x06, ///< Unknown.
	GPU_TEVOP_RGB_0x07                = 0x07, ///< Unknown.
	GPU_TEVOP_RGB_SRC_G               = 0x08, ///< Source green.
	GPU_TEVOP_RGB_ONE_MINUS_SRC_G     = 0x09, ///< Source green - 1.
	GPU_TEVOP_RGB_0x0A                = 0x0A, ///< Unknown.
	GPU_TEVOP_RGB_0x0B                = 0x0B, ///< Unknown.
	GPU_TEVOP_RGB_SRC_B               = 0x0C, ///< Source blue.
	GPU_TEVOP_RGB_ONE_MINUS_SRC_B     = 0x0D, ///< Source blue - 1.
	GPU_TEVOP_RGB_0x0E                = 0x0E, ///< Unknown.
	GPU_TEVOP_RGB_0x0F                = 0x0F, ///< Unknown.
} GPU_TEVOP_RGB;

/// Texture Alpha combiner operands.
typedef enum
{
	GPU_TEVOP_A_SRC_ALPHA           = 0x00, ///< Source alpha.
	GPU_TEVOP_A_ONE_MINUS_SRC_ALPHA = 0x01, ///< Source alpha - 1.
	GPU_TEVOP_A_SRC_R               = 0x02, ///< Source red.
	GPU_TEVOP_A_ONE_MINUS_SRC_R     = 0x03, ///< Source red - 1.
	GPU_TEVOP_A_SRC_G               = 0x04, ///< Source green.
	GPU_TEVOP_A_ONE_MINUS_SRC_G     = 0x05, ///< Source green - 1.
	GPU_TEVOP_A_SRC_B               = 0x06, ///< Source blue.
	GPU_TEVOP_A_ONE_MINUS_SRC_B     = 0x07, ///< Source blue - 1.
} GPU_TEVOP_A;

/// Texture combiner functions.
typedef enum
{
	GPU_REPLACE      = 0x00, ///< Replace.
	GPU_MODULATE     = 0x01, ///< Modulate.
	GPU_ADD          = 0x02, ///< Add.
	GPU_ADD_SIGNED   = 0x03, ///< Signed add.
	GPU_INTERPOLATE  = 0x04, ///< Interpolate.
	GPU_SUBTRACT     = 0x05, ///< Subtract.
	GPU_DOT3_RGB     = 0x06, ///< Dot3. RGB only.
	GPU_MULTIPLY_ADD = 0x08, ///< Multiply then add.
	GPU_ADD_MULTIPLY = 0x09, ///< Add then multiply.
} GPU_COMBINEFUNC;

/// Texture scale factors.
typedef enum
{
	GPU_TEVSCALE_1 = 0x0, ///< 1x
	GPU_TEVSCALE_2 = 0x1, ///< 2x
	GPU_TEVSCALE_4 = 0x2, ///< 4x
} GPU_TEVSCALE;

/// Creates a texture combiner source parameter from three sources.
#define GPU_TEVSOURCES(a,b,c)  (((a))|((b)<<4)|((c)<<8))
/// Creates a texture combiner operand parameter from three operands.
#define GPU_TEVOPERANDS(a,b,c) (((a))|((b)<<4)|((c)<<8))

/// Creates a light environment layer configuration parameter.
#define GPU_LIGHT_ENV_LAYER_CONFIG(n) ((n)+((n)==7))
/// Light shadow disable bits in GPUREG_LIGHT_CONFIG1.
#define GPU_LC1_SHADOWBIT(n)   BIT(n)
/// Light spot disable bits in GPUREG_LIGHT_CONFIG1.
#define GPU_LC1_SPOTBIT(n)     BIT((n)+8)
/// LUT disable bits in GPUREG_LIGHT_CONFIG1.
#define GPU_LC1_LUTBIT(n)      BIT((n)+16)
/// Light distance attenuation disable bits in GPUREG_LIGHT_CONFIG1.
#define GPU_LC1_ATTNBIT(n)     BIT((n)+24)
/// Creates a light permutation parameter.
#define GPU_LIGHTPERM(i,n)     ((n) << ((i)*4))
/// Creates a light LUT input parameter.
#define GPU_LIGHTLUTINPUT(i,n) ((n) << ((i)*4))
/// Creates a light LUT index parameter.
#define GPU_LIGHTLUTIDX(c,i,o) ((o) | ((i) << 8) | ((c) << 11))
/// Creates a light color parameter from red, green, and blue components.
#define GPU_LIGHTCOLOR(r,g,b)  (((b) & 0xFF) | (((g) << 10) & 0xFF) | (((r) << 20) & 0xFF))

/// Fresnel options.
typedef enum
{
	GPU_NO_FRESNEL            = 0, ///< None.
	GPU_PRI_ALPHA_FRESNEL     = 1, ///< Primary alpha.
	GPU_SEC_ALPHA_FRESNEL     = 2, ///< Secondary alpha.
	GPU_PRI_SEC_ALPHA_FRESNEL = 3, ///< Primary and secondary alpha.
} GPU_FRESNELSEL;

/// Bump map modes.
typedef enum
{
	GPU_BUMP_NOT_USED = 0, ///< Disabled.
	GPU_BUMP_AS_BUMP  = 1, ///< Bump as bump mapping.
	GPU_BUMP_AS_TANG  = 2, ///< Bump as tangent/normal mapping.
} GPU_BUMPMODE;

/// LUT IDs.
typedef enum
{
	GPU_LUT_D0 = 0, ///< D0 LUT.
	GPU_LUT_D1 = 1, ///< D1 LUT.
	GPU_LUT_SP = 2, ///< Spotlight LUT.
	GPU_LUT_FR = 3, ///< Fresnel LUT.
	GPU_LUT_RB = 4, ///< Reflection-Blue LUT.
	GPU_LUT_RG = 5, ///< Reflection-Green LUT.
	GPU_LUT_RR = 6, ///< Reflection-Red LUT.
	GPU_LUT_DA = 7, ///< Distance attenuation LUT.
} GPU_LIGHTLUTID;

/// LUT inputs.
typedef enum
{
	GPU_LUTINPUT_NH = 0, ///< Normal*HalfVector
	GPU_LUTINPUT_VH = 1, ///< View*HalfVector
	GPU_LUTINPUT_NV = 2, ///< Normal*View
	GPU_LUTINPUT_LN = 3, ///< LightVector*Normal
	GPU_LUTINPUT_SP = 4, ///< -LightVector*SpotlightVector
	GPU_LUTINPUT_CP = 5, ///< cosine of phi
} GPU_LIGHTLUTINPUT;

/// LUT scalers.
typedef enum
{
	GPU_LUTSCALER_1x    = 0, ///< 1x scale.
	GPU_LUTSCALER_2x    = 1, ///< 2x scale.
	GPU_LUTSCALER_4x    = 2, ///< 4x scale.
	GPU_LUTSCALER_8x    = 3, ///< 8x scale.
	GPU_LUTSCALER_0_25x = 6, ///< 0.25x scale.
	GPU_LUTSCALER_0_5x  = 7, ///< 0.5x scale.
} GPU_LIGHTLUTSCALER;

/// LUT selection.
typedef enum
{
	GPU_LUTSELECT_COMMON = 0, ///< LUTs that are common to all lights.
	GPU_LUTSELECT_SP     = 1, ///< Spotlight LUT.
	GPU_LUTSELECT_DA     = 2, ///< Distance attenuation LUT.
} GPU_LIGHTLUTSELECT;

/// Fog modes.
typedef enum
{
	GPU_NO_FOG = 0, ///< Fog/Gas unit disabled.
	GPU_FOG    = 5, ///< Fog/Gas unit configured in Fog mode.
	GPU_GAS    = 7, ///< Fog/Gas unit configured in Gas mode.
} GPU_FOGMODE;

/// Gas shading density source values.
typedef enum
{
	GPU_PLAIN_DENSITY = 0, ///< Plain density.
	GPU_DEPTH_DENSITY = 1, ///< Depth density.
} GPU_GASMODE;

/// Gas color LUT inputs.
typedef enum
{
	GPU_GAS_DENSITY      = 0, ///< Gas density used as input.
	GPU_GAS_LIGHT_FACTOR = 1, ///< Light factor used as input.
} GPU_GASLUTINPUT;

/// Supported primitives.
typedef enum
{
	GPU_TRIANGLES      = 0x0000, ///< Triangles.
	GPU_TRIANGLE_STRIP = 0x0100, ///< Triangle strip.
	GPU_TRIANGLE_FAN   = 0x0200, ///< Triangle fan.
	GPU_GEOMETRY_PRIM  = 0x0300, ///< Geometry shader primitive.
} GPU_Primitive_t;

/// Shader types.
typedef enum
{
	GPU_VERTEX_SHADER   = 0x0, ///< Vertex shader.
	GPU_GEOMETRY_SHADER = 0x1, ///< Geometry shader.
} GPU_SHADER_TYPE;

/**
 * @file shaderProgram.h
 * @brief Functions for working with shaders.
 */
#pragma once

#include <3ds/types.h>
#include <3ds/gpu/shbin.h>

/// 24-bit float uniforms.
typedef struct
{
	u32 id;      ///< Uniform ID.
	u32 data[3]; ///< Uniform data.
}float24Uniform_s;

/// Describes an instance of either a vertex or geometry shader.
typedef struct
{
	DVLE_s* dvle;                      ///< Shader DVLE.
	u16 boolUniforms;                  ///< Boolean uniforms.
	u16 boolUniformMask;               ///< Used boolean uniform mask.
	u32 intUniforms[4];                ///< Integer uniforms.
	float24Uniform_s* float24Uniforms; ///< 24-bit float uniforms.
	u8 intUniformMask;                 ///< Used integer uniform mask.
	u8 numFloat24Uniforms;             ///< Float uniform count.
}shaderInstance_s;

/// Describes an instance of a full shader program.
typedef struct
{
	shaderInstance_s* vertexShader;   ///< Vertex shader.
	shaderInstance_s* geometryShader; ///< Geometry shader.
	u32 geoShaderInputPermutation[2]; ///< Geometry shader input permutation.
	u8 geoShaderInputStride;          ///< Geometry shader input stride.
}shaderProgram_s;

/**
 * @brief Initializes a shader instance.
 * @param si Shader instance to initialize.
 * @param dvle DVLE to initialize the shader instance with.
 */
Result shaderInstanceInit(shaderInstance_s* si, DVLE_s* dvle);

/**
 * @brief Frees a shader instance.
 * @param si Shader instance to free.
 */
Result shaderInstanceFree(shaderInstance_s* si);

/**
 * @brief Sets a bool uniform of a shader.
 * @param si Shader instance to use.
 * @param id ID of the bool uniform.
 * @param value Value to set.
 */
Result shaderInstanceSetBool(shaderInstance_s* si, int id, bool value);

/**
 * @brief Gets a bool uniform of a shader.
 * @param si Shader instance to use.
 * @param id ID of the bool uniform.
 * @param value Pointer to output the value to.
 */
Result shaderInstanceGetBool(shaderInstance_s* si, int id, bool* value);

/**
 * @brief Gets the location of a shader's uniform.
 * @param si Shader instance to use.
 * @param name Name of the uniform.
 */
s8 shaderInstanceGetUniformLocation(shaderInstance_s* si, const char* name);

/**
 * @brief Initializes a shader program.
 * @param sp Shader program to initialize.
 */
Result shaderProgramInit(shaderProgram_s* sp);

/**
 * @brief Frees a shader program.
 * @param sp Shader program to free.
 */
Result shaderProgramFree(shaderProgram_s* sp);

/**
 * @brief Sets the vertex shader of a shader program.
 * @param sp Shader program to use.
 * @param dvle Vertex shader to set.
 */
Result shaderProgramSetVsh(shaderProgram_s* sp, DVLE_s* dvle);

/**
 * @brief Sets the geometry shader of a shader program.
 * @param sp Shader program to use.
 * @param dvle Geometry shader to set.
 * @param stride Input stride of the shader (pass 0 to match the number of outputs of the vertex shader).
 */
Result shaderProgramSetGsh(shaderProgram_s* sp, DVLE_s* dvle, u8 stride);

/**
 * @brief Configures the permutation of the input attributes of the geometry shader of a shader program.
 * @param sp Shader program to use.
 * @param permutation Attribute permutation to use.
 */
Result shaderProgramSetGshInputPermutation(shaderProgram_s* sp, u64 permutation);

/**
 * @brief Configures the shader units to use the specified shader program.
 * @param sp Shader program to use.
 * @param sendVshCode When true, the vertex shader's code and operand descriptors are uploaded.
 * @param sendGshCode When true, the geometry shader's code and operand descriptors are uploaded.
 */
Result shaderProgramConfigure(shaderProgram_s* sp, bool sendVshCode, bool sendGshCode);

/**
 * @brief Same as shaderProgramConfigure, but always loading code/operand descriptors and uploading DVLE constants afterwards.
 * @param sp Shader program to use.
 */
Result shaderProgramUse(shaderProgram_s* sp);

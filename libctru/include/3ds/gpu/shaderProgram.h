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
	u32 intUniforms[4];                ///< Integer uniforms.
	float24Uniform_s* float24Uniforms; ///< 24-bit float uniforms.
	u8 numFloat24Uniforms;             ///< Float uniform count.
}shaderInstance_s;

/// Describes an instance of a full shader program.
typedef struct
{
	shaderInstance_s* vertexShader;   ///< Vertex shader.
	shaderInstance_s* geometryShader; ///< Geometry shader.
	u8 geometryShaderInputStride;     ///< Geometry shader input stride.
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
Result shaderInstanceGetUniformLocation(shaderInstance_s* si, const char* name);

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
 * @param stride Stride of the geometry shader.
 */
Result shaderProgramSetGsh(shaderProgram_s* sp, DVLE_s* dvle, u8 stride);

/**
 * @brief Sets the active shader program.
 * @param sp Shader program to use.
 */
Result shaderProgramUse(shaderProgram_s* sp);

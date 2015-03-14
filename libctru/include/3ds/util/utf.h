#pragma once

#include <stdint.h>
#include <sys/types.h>

/*! Convert a UTF-8 sequence into a UTF-32 codepoint
 *
 *  @param[out] out Output codepoint
 *  @param[in]  in  Input sequence
 *
 *  @returns number of input code units consumed
 *  @returns -1 for error
 */
ssize_t decode_utf8 (uint32_t *out, const uint8_t *in);

/*! Convert a UTF-16 sequence into a UTF-32 codepoint
 *
 *  @param[out] out Output codepoint
 *  @param[in]  in  Input sequence
 *
 *  @returns number of input code units consumed
 *  @returns -1 for error
 */
ssize_t decode_utf16(uint32_t *out, const uint16_t *in);

/*! Convert a UTF-32 codepoint into a UTF-8 sequence
 *
 *  @param[out] out Output sequence
 *  @param[in]  in  Input codepoint
 *
 *  @returns number of output code units produced
 *  @returns -1 for error
 *
 *  @note \a out must be able to store 4 code units
 */
ssize_t encode_utf8 (uint8_t *out, uint32_t in);

/*! Convert a UTF-32 codepoint into a UTF-16 sequence
 *
 *  @param[out] out Output sequence
 *  @param[in]  in  Input codepoint
 *
 *  @returns number of output code units produced
 *  @returns -1 for error
 *
 *  @note \a out must be able to store 2 code units
 */
ssize_t encode_utf16(uint16_t *out, uint32_t in);

/*! Convert a UTF-8 sequence into a UTF-16 sequence
 *
 *  @param[out] out Output sequence
 *  @param[in]  in  Input sequence
 *
 *  @returns number of output code units produced
 *  @returns -1 for error
 */
size_t utf8_to_utf16(uint16_t *out, const uint8_t  *in, size_t len);

/*! Convert a UTF-8 sequence into a UTF-32 sequence
 *
 *  @param[out] out Output sequence
 *  @param[in]  in  Input sequence
 *
 *  @returns number of output code units produced
 *  @returns -1 for error
 */
size_t utf8_to_utf32(uint32_t *out, const uint8_t  *in, size_t len);

/*! Convert a UTF-16 sequence into a UTF-8 sequence
 *
 *  @param[out] out Output sequence
 *  @param[in]  in  Input sequence
 *
 *  @returns number of output code units produced
 *  @returns -1 for error
 */
size_t utf16_to_utf8(uint8_t  *out, const uint16_t *in, size_t len);

/*! Convert a UTF-16 sequence into a UTF-32 sequence
 *
 *  @param[out] out Output sequence
 *  @param[in]  in  Input sequence
 *
 *  @returns number of output code units produced
 *  @returns -1 for error
 */
size_t utf16_to_utf32(uint32_t *out, const uint16_t *in, size_t len);

/*! Convert a UTF-32 sequence into a UTF-8 sequence
 *
 *  @param[out] out Output sequence
 *  @param[in]  in  Input sequence
 *
 *  @returns number of output code units produced
 *  @returns -1 for error
 */
size_t utf32_to_utf8(uint8_t  *out, const uint32_t *in, size_t len);

/*! Convert a UTF-32 sequence into a UTF-16 sequence
 *
 *  @param[out] out Output sequence
 *  @param[in]  in  Input sequence
 *
 *  @returns number of output code units produced
 *  @returns -1 for error
 */
size_t utf32_to_utf16(uint16_t *out, const uint32_t *in, size_t len);

/**
 * @file decompress.h
 * @brief Decompression functions.
 */
#pragma once

#include <stdbool.h>
#include <sys/types.h>

/** @brief I/O vector */
typedef struct
{
  void   *data; ///< I/O buffer
  size_t size;  ///< Buffer size
} decompressIOVec;

/** @brief Data callback */
typedef ssize_t (*decompressCallback)(void *userdata, void *buffer,
                                      size_t size);

#ifdef __cplusplus
extern "C"
{
#endif

/** @brief Decompression callback for file descriptors
 *  @param[in] userdata Address of file descriptor
 *  @param[in] buffer   Buffer to write into
 *  @param[in] size     Size to read from file descriptor
 *  @returns Number of bytes read
 */
ssize_t decompressCallback_FD(void *userdata, void *buffer, size_t size);

/** @brief Decompression callback for stdio FILE*
 *  @param[in] userdata FILE*
 *  @param[in] buffer   Buffer to write into
 *  @param[in] size     Size to read from file descriptor
 *  @returns Number of bytes read
 */
ssize_t decompressCallback_Stdio(void *userdata, void *buffer, size_t size);

/** @brief Decompress data
 *  @param[in] iov      Output vector
 *  @param[in] iovcnt   Number of buffers
 *  @param[in] callback Data callback (see note)
 *  @param[in] userdata User data passed to callback (see note)
 *  @param[in] insize   Size of userdata (see note)
 *  @returns Whether succeeded
 *
 *  @note If callback is null, userdata is a pointer to memory to read from,
 *        and insize is the size of that data. If callback is not null,
 *        userdata is passed to callback to fetch more data, and insize is
 *        unused.
 */
bool decompressV(const decompressIOVec *iov, size_t iovcnt,
                 decompressCallback callback, void *userdata, size_t insize);

/** @brief Decompress data
 *  @param[in] output   Output buffer
 *  @param[in] size     Output size limit
 *  @param[in] callback Data callback (see decompressV())
 *  @param[in] userdata User data passed to callback (see decompressV())
 *  @param[in] insize   Size of userdata (see decompressV())
 *  @returns Whether succeeded
 */
static inline bool
decompress(void *output, size_t size, decompressCallback callback,
           void *userdata, size_t insize)
{
  decompressIOVec iov;
  iov.data = output;
  iov.size = size;

  return decompressV(&iov, 1, callback, userdata, insize);
}

/** @brief Decompress LZSS/LZ10
 *  @param[in] iov      Output vector
 *  @param[in] iovcnt   Number of buffers
 *  @param[in] callback Data callback (see decompressV())
 *  @param[in] userdata User data passed to callback (see decompressV())
 *  @param[in] insize   Size of userdata (see decompressV())
 *  @returns Whether succeeded
 */
bool decompressV_LZSS(const decompressIOVec *iov, size_t iovcnt,
                      decompressCallback callback, void *userdata,
                      size_t insize);

/** @brief Decompress LZSS/LZ10
 *  @param[in] output   Output buffer
 *  @param[in] size     Output size limit
 *  @param[in] callback Data callback (see decompressV())
 *  @param[in] userdata User data passed to callback (see decompressV())
 *  @param[in] insize   Size of userdata (see decompressV())
 *  @returns Whether succeeded
 */
static inline bool
decompress_LZSS(void *output, size_t size, decompressCallback callback,
                void *userdata, size_t insize)
{
  decompressIOVec iov;
  iov.data = output;
  iov.size = size;

  return decompressV_LZSS(&iov, 1, callback, userdata, insize);
}

/** @brief Decompress LZ11
 *  @param[in] iov      Output vector
 *  @param[in] iovcnt   Number of buffers
 *  @param[in] callback Data callback (see decompressV())
 *  @param[in] userdata User data passed to callback (see decompressV())
 *  @param[in] insize   Size of userdata (see decompressV())
 *  @returns Whether succeeded
 */
bool decompressV_LZ11(const decompressIOVec *iov, size_t iovcnt,
                      decompressCallback callback, void *userdata,
                      size_t insize);

/** @brief Decompress LZ11
 *  @param[in] output   Output buffer
 *  @param[in] size     Output size limit
 *  @param[in] callback Data callback (see decompressV())
 *  @param[in] userdata User data passed to callback (see decompressV())
 *  @param[in] insize   Size of userdata (see decompressV())
 *  @returns Whether succeeded
 */
static inline bool
decompress_LZ11(void *output, size_t size, decompressCallback callback,
                void *userdata, size_t insize)
{
  decompressIOVec iov;
  iov.data = output;
  iov.size = size;

  return decompressV_LZ11(&iov, 1, callback, userdata, insize);
}

/** @brief Decompress Huffman
 *  @param[in] iov      Output vector
 *  @param[in] iovcnt   Number of buffers
 *  @param[in] callback Data callback (see decompressV())
 *  @param[in] userdata User data passed to callback (see decompressV())
 *  @param[in] insize   Size of userdata (see decompressV())
 *  @returns Whether succeeded
 */
bool decompressV_Huff(const decompressIOVec *iov, size_t iovcnt,
                      decompressCallback callback, void *userdata,
                      size_t insize);

/** @brief Decompress Huffman
 *  @param[in] output   Output buffer
 *  @param[in] size     Output size limit
 *  @param[in] callback Data callback (see decompressV())
 *  @param[in] userdata User data passed to callback (see decompressV())
 *  @param[in] insize   Size of userdata (see decompressV())
 *  @returns Whether succeeded
 */
static inline bool
decompress_Huff(void *output, size_t size, decompressCallback callback,
                void *userdata, size_t insize)
{
  decompressIOVec iov;
  iov.data = output;
  iov.size = size;

  return decompressV_Huff(&iov, 1, callback, userdata, insize);
}

/** @brief Decompress run-length encoding
 *  @param[in] iov      Output vector
 *  @param[in] iovcnt   Number of buffers
 *  @param[in] callback Data callback (see decompressV())
 *  @param[in] userdata User data passed to callback (see decompressV())
 *  @param[in] insize   Size of userdata (see decompressV())
 *  @returns Whether succeeded
 */
bool decompressV_RLE(const decompressIOVec *iov, size_t iovcnt,
                     decompressCallback callback, void *userdata,
                     size_t insize);

/** @brief Decompress run-length encoding
 *  @param[in] output   Output buffer
 *  @param[in] size     Output size limit
 *  @param[in] callback Data callback (see decompressV())
 *  @param[in] userdata User data passed to callback (see decompressV())
 *  @param[in] insize   Size of userdata (see decompressV())
 *  @returns Whether succeeded
 */
static inline bool
decompress_RLE(void *output, size_t size, decompressCallback callback,
               void *userdata, size_t insize)
{
  decompressIOVec iov;
  iov.data = output;
  iov.size = size;

  return decompressV_RLE(&iov, 1, callback, userdata, insize);
}

#ifdef __cplusplus
}
#endif

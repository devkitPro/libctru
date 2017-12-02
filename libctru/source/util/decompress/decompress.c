/** @file decompress.c
 *  @brief Decompression routines
 */
#include <3ds/util/decompress.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFERSIZE 4096

/** @brief Buffer */
typedef struct buffer_t
{
  uint8_t *data; ///< Pointer to buffer
  size_t  limit; ///< Max buffer size
  size_t  size;  ///< Current buffer size
  size_t  pos;   ///< Buffer position
} buffer_t;

/*! I/O vector iterator */
typedef struct
{
  const decompressIOVec *iov; //!< I/O vector
  size_t                cnt;  //!< Number of buffers
  size_t                num;  //!< Current buffer number
  size_t                pos;  //!< Current buffer position
} iov_iter;

/** @brief Initialize buffer object from memory
 *  @param[in] buffer Decompression buffer object
 *  @param[in] data   Data to emulate buffering
 *  @param[in] size   Data size
 */
static inline void
buffer_memory(buffer_t *buffer, const void *data, size_t size)
{
  buffer->data  = (void*)data;
  buffer->limit = size;
  buffer->size  = size;
  buffer->pos   = 0;
}

/** @brief Initialize buffer object with static memory
 *  @param[in] buffer Decompression buffer object
 *  @param[in] data   Data buffer
 *  @param[in] size   Data size
 */
static inline void
buffer_static(buffer_t *buffer, const void *data, size_t size)
{
  buffer->data  = (void*)data;
  buffer->limit = size;
  buffer->size  = 0;
  buffer->pos   = 0;
}

/** @brief Initialize buffer object with dynamic memory
 *  @param[in] buffer Decompression buffer object
 *  @param[in] size   Buffer size limit
 *  @returns Whether succeeded
 */
static inline bool
buffer_dynamic(buffer_t *buffer, size_t size)
{
  buffer->data = (uint8_t*)malloc(size);
  if(!buffer->data)
    return false;

  buffer->limit = size;
  buffer->size  = 0;
  buffer->pos   = 0;

  return true;
}

/** @brief Destroy buffer object
 *  @param[in] buffer Decompression buffer object
 */
static inline void
buffer_destroy(buffer_t *buffer)
{
  free(buffer->data);
}

/** @brief Read from buffer object
 *  @param[in]  buffer   Decompression buffer object
 *  @param[out] dest     Output buffer
 *  @param[in]  size     Amount to read
 *  @param[in]  callback Data callback
 *  @param[in]  userdata User data passed to callback
 *  @returns Whether succeeded
 */
static bool
buffer_read(buffer_t *buffer, void *dest, size_t size,
            decompressCallback callback, void *userdata)
{
  while(size > 0)
  {
    // entire request is in our buffer
    if(size <= buffer->size - buffer->pos)
    {
      memcpy(dest, buffer->data + buffer->pos, size);
      buffer->pos += size;
      return true;
    }

    // copy partial data
    if(buffer->pos != buffer->size)
    {
      memcpy(dest, buffer->data + buffer->pos, buffer->size - buffer->pos);
      dest  = (uint8_t*)dest + (buffer->size - buffer->pos);
      size -= buffer->size - buffer->pos;

      assert(size != 0);
    }

    if(!callback)
      return false;

    // fetch some more data
    buffer->size = buffer->pos = 0;
    ssize_t rc = callback(userdata, buffer->data, buffer->limit);
    if(rc <= 0)
      return false;

    buffer->size = rc;
  }

  return true;
}

/** @brief Read a byte from a buffer object
 *  @param[in]  buffer   Decompression buffer object
 *  @param[out] dest     Output buffer
 *  @param[in]  callback Data callback
 *  @param[in]  userdata User data passed to callback
 *  @returns Whether succeeded
 */
static inline bool
buffer_get(buffer_t *buffer, uint8_t *dest, decompressCallback callback,
           void *userdata)
{
  // fast-path; just read the byte if we have it
  if(buffer->pos < buffer->size)
  {
    *dest = buffer->data[buffer->pos++];
    return true;
  }

  // grab the byte with read-ahead
  return buffer_read(buffer, dest, sizeof(*dest), callback, userdata);
}

/*! Create I/O vector iterator
 *  @param[in] iov I/O vector
 *  @param[in] iovcnt Number of buffers
 *  @returns I/O vector iterator
 */
static inline iov_iter
iov_begin(const decompressIOVec *iov, size_t iovcnt)
{
  iov_iter it;
  it.iov = iov;
  it.cnt = iovcnt;
  it.num = 0;
  it.pos = 0;

  return it;
}

/*! Get I/O vector total size
 *  @param[in] iov I/O vector
 *  @param[in] iovcnt Number of buffers
 *  @returns Total buffer size
 */
static inline size_t
iov_size(const decompressIOVec *iov, size_t iovcnt)
{
  size_t size = 0;

  for(size_t i = 0; i < iovcnt; ++i)
  {
    assert(SIZE_MAX - size >= iov[i].size);
    size += iov[i].size;
  }

  return size;
}

/*! Get address for current iterator position
 *  @param[in] it I/O vector iterator
 *  @returns address for current iterator position
 */
static inline uint8_t*
iov_addr(const iov_iter *it)
{
  assert(it->num < it->cnt);
  assert(it->pos < it->iov[it->num].size);

  return (uint8_t*)it->iov[it->num].data + it->pos;
}

/*! Increment iterator by one position
 *  @param[in] it Iterator to increment
 */
static inline void
iov_increment(iov_iter *it)
{
  assert(it->num < it->cnt);
  assert(it->pos < it->iov[it->num].size);

  ++it->pos;

  if(it->pos == it->iov[it->num].size)
  {
    // advance to next buffer
    it->pos = 0;
    ++it->num;
  }
}

/*! Increment iterator by size
 *  @param[in] it   Iterator to increment
 *  @param[in] size Size to increment
 */
static inline void
iov_add(iov_iter *it, size_t size)
{
  while(true)
  {
    assert(it->num <= it->cnt);
    assert(it->iov[it->num].size > it->pos);

    if(it->iov[it->num].size - it->pos > size)
    {
      // position is within current buffer
      it->pos += size;
      return;
    }

    // advance to next buffer
    size -= it->iov[it->num].size - it->pos;
    ++it->num;
    it->pos = 0;
  }
}

/*! Decrement iterator by size
 *  @param[in] it   Iterator to decrement
 *  @param[in] size Size to decrement
 */
static inline void
iov_sub(iov_iter *it, size_t size)
{
  while(true)
  {
    if(it->pos >= size)
    {
      // position is within current buffer
      it->pos -= size;
      return;
    }

    // move to previous buffer
    size -= it->pos;
    assert(it->num > 0);
    --it->num;
    it->pos = it->iov[it->num].size;
  }
}

static bool
iov_read(buffer_t *buffer, iov_iter *out, size_t size,
         decompressCallback callback, void *userdata)
{
  while(size > 0)
  {
    assert(out->num < out->cnt);
    assert(out->pos < out->iov[out->num].size);

    size_t bytes = out->iov[out->num].size - out->pos;
    if(size < bytes)
      bytes = size;

    if(!buffer_read(buffer, iov_addr(out), bytes, callback, userdata))
      return false;

    size -= bytes;
    iov_add(out, bytes);
  }

  return true;
}

static void
iov_memmove(iov_iter *out, iov_iter *in, size_t size)
{
  while(size > 0)
  {
    assert(out->num < out->cnt);
    assert(out->pos < out->iov[out->num].size);
    assert(in->num < in->cnt);
    assert(in->pos < in->iov[in->num].size);

    size_t bytes;
    uint8_t *outbuf = iov_addr(out);
    uint8_t *inbuf  = iov_addr(in);

    if(out->iov[out->num].size - out->pos < in->iov[in->num].size - in->pos)
      bytes = out->iov[out->num].size - out->pos;
    else
      bytes = in->iov[in->num].size - in->pos;

    if(size < bytes)
      bytes = size;

    size -= bytes;

    iov_add(out, bytes);
    iov_add(in, bytes);

    while(bytes-- > 0)
      *outbuf++ = *inbuf++;
  }
}

static void
iov_memset(iov_iter *out, char val, size_t size)
{
  while(size > 0)
  {
    assert(out->num < out->cnt);
    assert(out->pos < out->iov[out->num].size);

    size_t bytes = out->iov[out->num].size - out->pos;
    if(size < bytes)
      bytes = size;

    memset(iov_addr(out), val, bytes);

    size -= bytes;
    iov_add(out, bytes);
  }
}

/** @brief Decompress LZSS/LZ10
 *  @param[in] buffer   Decompression buffer object
 *  @param[in] iov      Output vector
 *  @param[in] iovcnt   Number of buffers
 *  @param[in] size     Output size limit
 *  @param[in] callback Data callback
 *  @param[in] userdata User data passed to callback
 *  @returns Whether succeeded
 */
static bool
decompress_lzss(buffer_t *buffer, const decompressIOVec *iov, size_t iovcnt,
                size_t size, decompressCallback callback, void *userdata)
{
  iov_iter     out = iov_begin(iov, iovcnt);
  uint8_t      flags = 0;
  uint8_t      mask  = 0;
  unsigned int len;
  unsigned int disp;

  while(size > 0)
  {
    if(mask == 0)
    {
      // read in the flags data
      // from bit 7 to bit 0:
      //     0: raw byte
      //     1: compressed block
      if(!buffer_get(buffer, &flags, callback, userdata))
        return false;
      mask = 0x80;
    }

    if(flags & mask) // compressed block
    {
      uint8_t displen[2];
      if(!buffer_get(buffer, &displen[0], callback, userdata)
      || !buffer_get(buffer, &displen[1], callback, userdata))
        return false;

      // disp: displacement
      // len:  length
      len  = ((displen[0] & 0xF0) >> 4) + 3;
      disp = displen[0] & 0x0F;
      disp = disp << 8 | displen[1];

      if(len > size)
        len = size;

      size -= len;

      iov_iter in = out;
      iov_sub(&in, disp+1);
      iov_memmove(&out, &in, len);
    }
    else // uncompressed block
    {
      // copy a raw byte from the input to the output
      if(!buffer_get(buffer, iov_addr(&out), callback, userdata))
        return false;

      --size;
      iov_increment(&out);
    }

    mask >>= 1;
  }

  return true;
}

/** @brief Decompress LZ11
 *  @param[in] buffer   Decompression buffer object
 *  @param[in] iov      Output vector
 *  @param[in] iovcnt   Number of buffers
 *  @param[in] size     Output size limit
 *  @param[in] callback Data callback
 *  @param[in] userdata User data passed to callback
 *  @returns Whether succeeded
 */
static bool
decompress_lz11(buffer_t *buffer, const decompressIOVec *iov, size_t iovcnt,
                size_t size, decompressCallback callback, void *userdata)
{
  iov_iter out = iov_begin(iov, iovcnt);
  int      i;
  uint8_t  flags;

  while(size > 0)
  {
    // read in the flags data
    // from bit 7 to bit 0, following blocks:
    //     0: raw byte
    //     1: compressed block
    if(!buffer_get(buffer, &flags, callback, userdata))
      return false;

    for(i = 0; i < 8 && size > 0; i++, flags <<= 1)
    {
      if(flags & 0x80) // compressed block
      {
        uint8_t displen[4];
        if(!buffer_get(buffer, &displen[0], callback, userdata))
          return false;

        size_t len;     // length
        size_t disp;    // displacement
        size_t pos = 0; // displen position

        switch(displen[pos] >> 4)
        {
          case 0: // extended block
            if(!buffer_get(buffer, &displen[1], callback, userdata)
            || !buffer_get(buffer, &displen[2], callback, userdata))
              return false;

            len   = displen[pos++] << 4;
            len  |= displen[pos] >> 4;
            len  += 0x11;
            break;

          case 1: // extra extended block
            if(!buffer_get(buffer, &displen[1], callback, userdata)
            || !buffer_get(buffer, &displen[2], callback, userdata)
            || !buffer_get(buffer, &displen[3], callback, userdata))
              return false;

            len   = (displen[pos++] & 0x0F) << 12;
            len  |= (displen[pos++]) << 4;
            len  |= displen[pos] >> 4;
            len  += 0x111;
            break;

          default: // normal block
            if(!buffer_get(buffer, &displen[1], callback, userdata))
              return false;
            len   = (displen[pos] >> 4) + 1;
            break;
        }

        disp  = (displen[pos++] & 0x0F) << 8;
        disp |= displen[pos];

        if(len > size)
          len = size;

        size -= len;

        iov_iter in = out;
        iov_sub(&in, disp+1);
        iov_memmove(&out, &in, len);
      }
      else // uncompressed block
      {
        // copy a raw byte from the input to the output
        if(!buffer_get(buffer, iov_addr(&out), callback, userdata))
          return false;

        --size;
        iov_increment(&out);
      }
    }
  }

  return true;
}

/** @brief Decompress Huffman
 *  @param[in] bits     Data size in bits (usually 4 or 8)
 *  @param[in] buffer   Decompression buffer object
 *  @param[in] iov      Output vector
 *  @param[in] iovcnt   Number of buffers
 *  @param[in] size     Output size limit
 *  @param[in] callback Data callback
 *  @param[in] userdata User data passed to callback
 *  @returns Whether succeeded
 */
static bool
decompress_huff(const size_t bits, buffer_t *buffer, const decompressIOVec *iov,
                size_t iovcnt, size_t size, decompressCallback callback,
                void *userdata)
{
  if(bits < 1 || bits > 8)
    return false;

  uint8_t *tree = (uint8_t*)malloc(512);
  if(!tree)
    return false;

  // get tree size
  if(!buffer_read(buffer, &tree[0], 1, callback, userdata))
  {
    free(tree);
    return false;
  }

  // read tree
  if(!buffer_read(buffer, &tree[1], (((size_t)tree[0])+1)*2-1, callback, userdata))
  {
    free(tree);
    return false;
  }

  iov_iter out = iov_begin(iov, iovcnt);
  uint32_t word = 0;               // 32-bits of input bitstream
  uint32_t mask = 0;               // which bit we are reading
  uint8_t  dataMask = (1<<bits)-1; // mask to apply to data
  size_t   node;                   // node in the huffman tree
  size_t   child;                  // child of a node
  uint32_t offset;                 // offset from node to child

  // point to the root of the huffman tree
  node = 1;

  while(size > 0)
  {
    if(mask == 0) // we exhausted 32 bits
    {
      // reset the mask
      mask = 0x80000000;

      // read the next 32 bits
      uint8_t wordbuf[4];
      if(!buffer_get(buffer, &wordbuf[0], callback, userdata)
      || !buffer_get(buffer, &wordbuf[1], callback, userdata)
      || !buffer_get(buffer, &wordbuf[2], callback, userdata)
      || !buffer_get(buffer, &wordbuf[3], callback, userdata))
      {
        free(tree);
        return false;
      }

      word = (wordbuf[0] <<  0)
           | (wordbuf[1] <<  8)
           | (wordbuf[2] << 16)
           | (wordbuf[3] << 24);
    }

    // read the current node's offset value
    offset = tree[node] & 0x1F;

    child = (node & ~1) + offset*2 + 2;

    if(word & mask) // we read a 1
    {
      // point to the "right" child
      ++child;

      if(tree[node] & 0x40) // "right" child is a data node
      {
        // copy the child node into the output buffer and apply mask
        *iov_addr(&out) = tree[child] & dataMask;
        iov_increment(&out);
        --size;

        // start over at the root node
        node = 1;
      }
      else // traverse to the "right" child
        node = child;
    }
    else // we read a 0
    {
      // pointed to the "left" child

      if(tree[node] & 0x80) // "left" child is a data node
      {
        // copy the child node into the output buffer and apply mask
        *iov_addr(&out) = tree[child] & dataMask;
        iov_increment(&out);
        --size;

        // start over at the root node
        node = 1;
      }
      else // traverse to the "left" child
        node = child;
    }

    // shift to read next bit (read bit 31 to bit 0)
    mask >>= 1;
  }

  free(tree);
  return true;
}

/** @brief Decompress Run-length encoding
 *  @param[in] buffer   Decompression buffer object
 *  @param[in] iov      Output vector
 *  @param[in] iovcnt   Number of buffers
 *  @param[in] size     Output size limit
 *  @param[in] callback Data callback
 *  @param[in] userdata User data passed to callback
 *  @returns Whether succeeded
 */
static bool
decompress_rle(buffer_t *buffer, const decompressIOVec *iov, size_t iovcnt,
               size_t size, decompressCallback callback, void *userdata)
{
  iov_iter out = iov_begin(iov, iovcnt);
  uint8_t  byte;
  size_t   len;

  while(size > 0)
  {
    // read in the data header
    if(!buffer_get(buffer, &byte, callback, userdata))
      return false;

    if(byte & 0x80) // compressed block
    {
      // read the length of the run
      len = (byte & 0x7F) + 3;

      if(len > size)
        len = size;

      size -= len;

      // read in the byte used for the run
      if(!buffer_get(buffer, &byte, callback, userdata))
        return false;

      // for len, copy byte into output
      iov_memset(&out, byte, len);
    }
    else // uncompressed block
    {
      // read the length of uncompressed bytes
      len = (byte & 0x7F) + 1;

      if(len > size)
        len = size;

      size -= len;

      // for len, copy from input to output
      if(!iov_read(buffer, &out, len, callback, userdata))
        return false;
    }
  }

  return true;
}

ssize_t
decompressCallback_FD(void *userdata, void *buffer, size_t size)
{
  int fd = *(int*)userdata;
  return read(fd, buffer, size);
}

ssize_t
decompressCallback_Stdio(void *userdata, void *buffer, size_t size)
{
  FILE *fp = (FILE*)userdata;
  return fread(buffer, 1, size, fp);
}

ssize_t
decompressHeader(decompressType *type, size_t *size,
                 decompressCallback callback, void *userdata, size_t insize)
{
  buffer_t buffer;
  uint8_t bufferdata[4];
  if(!callback)
    buffer_memory(&buffer, userdata, insize);
  else
    buffer_static(&buffer, bufferdata, sizeof(bufferdata));

  uint8_t header[4];
  if(!buffer_read(&buffer, header, 4, callback, userdata))
    return -1;

  size_t bytes = 4;

  decompressType outtype = header[0] & ~0x80;
  size_t outsize = (header[1] <<  0)
                 | (header[2] <<  8)
                 | (header[3] << 16);

  if(header[0] & 0x80)
  {
    if(!buffer_read(&buffer, header, 4, callback, userdata))
      return -1;

    bytes += 4;
    outsize |= header[0] << 24;
  }

  if(type)
    *type = outtype;
  if(size)
    *size = outsize;

  return bytes;
}

bool
decompressV(const decompressIOVec *iov, size_t iovcnt,
            decompressCallback callback, void *userdata, size_t insize)
{
  if(iovcnt == 0)
    return false;

  decompressType type;
  size_t size;
  ssize_t bytes = decompressHeader(&type, &size, callback, userdata, insize);
  if(bytes < 0)
    return false;

  buffer_t buffer;
  if(!callback)
  {
    userdata = (uint8_t*)userdata + bytes;
    insize -= bytes;
    buffer_memory(&buffer, userdata, insize);
  }
  else if(!buffer_dynamic(&buffer, BUFFERSIZE))
    return false;

  size_t iovsize = iov_size(iov, iovcnt);
  if(iovsize < size)
    size = iovsize;

  bool result = false;
  switch(type)
  {
    case DECOMPRESS_DUMMY:
    {
      iov_iter out = iov_begin(iov, iovcnt);
      result = iov_read(&buffer, &out, size, callback, userdata);
      break;
    }

    case DECOMPRESS_LZSS:
      result = decompress_lzss(&buffer, iov, iovcnt, size, callback, userdata);
      break;

    case DECOMPRESS_LZ11:
      result = decompress_lz11(&buffer, iov, iovcnt, size, callback, userdata);
      break;

    case DECOMPRESS_HUFF1:
    case DECOMPRESS_HUFF2:
    case DECOMPRESS_HUFF3:
    case DECOMPRESS_HUFF4:
    case DECOMPRESS_HUFF5:
    case DECOMPRESS_HUFF6:
    case DECOMPRESS_HUFF7:
    case DECOMPRESS_HUFF8:
      result = decompress_huff(type & 0xF, &buffer, iov, iovcnt, size,
                               callback, userdata);
      break;

    case DECOMPRESS_RLE:
      result = decompress_rle(&buffer, iov, iovcnt, size, callback, userdata);
      break;
  }

  if(callback)
    buffer_destroy(&buffer);
  return result;
}

bool
decompressV_LZSS(const decompressIOVec *iov, size_t iovcnt,
                 decompressCallback callback, void *userdata, size_t insize)
{
  buffer_t buffer;
  if(!callback)
    buffer_memory(&buffer, userdata, insize);
  else if(!buffer_dynamic(&buffer, BUFFERSIZE))
    return false;

  size_t size = iov_size(iov, iovcnt);
  bool result = decompress_lzss(&buffer, iov, iovcnt, size, callback, userdata);

  if(callback)
    buffer_destroy(&buffer);
  return result;
}

bool
decompressV_LZ11(const decompressIOVec *iov, size_t iovcnt,
                 decompressCallback callback, void *userdata, size_t insize)
{
  buffer_t buffer;
  if(!callback)
    buffer_memory(&buffer, userdata, insize);
  else if(!buffer_dynamic(&buffer, BUFFERSIZE))
    return false;

  size_t size = iov_size(iov, iovcnt);
  bool result = decompress_lz11(&buffer, iov, iovcnt, size, callback, userdata);

  if(callback)
    buffer_destroy(&buffer);
  return result;
}

bool
decompressV_Huff(size_t bits, const decompressIOVec *iov, size_t iovcnt,
                 decompressCallback callback, void *userdata, size_t insize)
{
  buffer_t buffer;
  if(!callback)
    buffer_memory(&buffer, userdata, insize);
  else if(!buffer_dynamic(&buffer, BUFFERSIZE))
    return false;

  size_t size = iov_size(iov, iovcnt);
  bool result = decompress_huff(bits, &buffer, iov, iovcnt, size, callback, userdata);

  if(callback)
    buffer_destroy(&buffer);
  return result;
}

bool
decompressV_RLE(const decompressIOVec *iov, size_t iovcnt,
                decompressCallback callback, void *userdata, size_t insize)
{
  buffer_t buffer;
  if(!callback)
    buffer_memory(&buffer, userdata, insize);
  else if(!buffer_dynamic(&buffer, BUFFERSIZE))
    return false;

  size_t size = iov_size(iov, iovcnt);
  bool result = decompress_rle(&buffer, iov, iovcnt, size, callback, userdata);

  if(callback)
    buffer_destroy(&buffer);
  return result;
}

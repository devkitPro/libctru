#include "3ds/util/utf.h"

size_t
utf32_to_utf8(uint8_t        *out,
              const uint32_t *in,
              size_t         len)
{
  size_t   rc = 0;
  ssize_t  units;
  uint8_t  encoded[4];

  while(*in > 0)
  {
    units = encode_utf8(encoded, *in++);
    if(units == -1)
      return (size_t)-1;

    if(out != NULL)
    {
      if(rc + units <= len)
      {
        *out++ = encoded[0];
        if(units > 1)
          *out++ = encoded[1];
        if(units > 2)
          *out++ = encoded[2];
        if(units > 3)
          *out++ = encoded[3];
      }
      else
        return rc;
    }

    rc += units;
  }

  return rc;
}

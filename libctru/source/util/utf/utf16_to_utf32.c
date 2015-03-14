#include "3ds/util/utf.h"

size_t
utf16_to_utf32(uint32_t       *out,
               const uint16_t *in,
               size_t         len)
{
  size_t   rc = 0;
  ssize_t  units;
  uint32_t code;

  do
  {
    units = decode_utf16(&code, in);
    if(units == -1)
      return (size_t)-1;

    if(code > 0)
    {
      in += units;

      if(out != NULL)
      {
        if(rc < len)
          *out++ = code;
        else
          return rc;
      }

      ++rc;
    }
  } while(code > 0);

  return rc;
}

#include "path_buf.h"

char     __thread __ctru_dev_path_buf[PATH_MAX+1];
uint16_t __thread __ctru_dev_utf16_buf[PATH_MAX+1];

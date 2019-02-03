#pragma once

#include <stddef.h>
#include <stdint.h>

uint32_t crc32c(const void *buffer, size_t size, uint32_t crc);

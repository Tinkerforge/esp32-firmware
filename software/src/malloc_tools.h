#pragma once

#include <stddef.h>

// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/mem_alloc.html#bit-accessible-memory
void *malloc_32bit_addressed(size_t s);
void *malloc_psram(size_t s);

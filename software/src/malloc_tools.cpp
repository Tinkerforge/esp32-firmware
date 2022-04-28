#include "malloc_tools.h"

#include "esp_heap_caps.h"

void *malloc_32bit_addressed(size_t s)
{
    return heap_caps_malloc(s, MALLOC_CAP_32BIT);
}

void *malloc_psram(size_t s)
{
    return heap_caps_malloc(s, MALLOC_CAP_SPIRAM);
}

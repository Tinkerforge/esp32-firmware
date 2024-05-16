#include "stddef.h"
#include "stdint.h"

// Unchecked snprintf that returns size_t
[[gnu::format(__printf__, 3, 4)]]
size_t snprintf_u(char *buf, size_t len, const char *format, ...);

bool a_after_b(uint32_t a, uint32_t b);
bool deadline_elapsed(uint32_t deadline_ms);

uint32_t millis();

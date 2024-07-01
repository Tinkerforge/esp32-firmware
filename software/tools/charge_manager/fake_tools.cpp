#include "tools.h"

#include "stdarg.h"
#include "stdint.h"
#include "stdio.h"
#include "time.h"

size_t snprintf_u(char *buf, size_t len, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vsnprintf(buf, len, format, args);
    va_end(args);

    return res < 0 ? 0 : static_cast<size_t>(res);
}

bool a_after_b(uint32_t a, uint32_t b)
{
    return ((uint32_t)(a - b)) < (UINT32_MAX / 2);
}

bool deadline_elapsed(uint32_t deadline_ms)
{
    return a_after_b(millis(), deadline_ms);
}

uint32_t millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts );
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000L);
}

micros_t now_us()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts );
    return (micros_t)(ts.tv_sec * 1000000 + ts.tv_nsec / 1000L);
}

bool deadline_elapsed(micros_t deadline_us)
{
    return deadline_us < now_us();
}

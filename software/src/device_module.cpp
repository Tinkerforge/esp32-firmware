#include "device_module.h"

#include "event_log.h"

int device_module_printfln(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = logger.printfln(fmt, args);
    va_end(args);

    return result;
}

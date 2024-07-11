#include "event_log_prefix.h"

#include "stdio.h"
#include "stdarg.h"
#include "string.h"

EventLog logger;

void EventLog::trace_timestamp() {
    printf("timestamp\n");
}

void EventLog::write(const char *buf, size_t len) {
    printf("%.*s\n", (int)len, buf);
}

void EventLog::printfln(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int res = vprintf(fmt, args);
    va_end(args);

    if (fmt[strlen(fmt) - 1] != '\n')
        putchar('\n');
}

void EventLog::trace_write(const char *buf) {
    printf("%s\n", buf);
}

void EventLog::tracefln(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int res = vprintf(fmt, args);
    va_end(args);

    if (fmt[strlen(fmt) - 1] != '\n')
        putchar('\n');
}

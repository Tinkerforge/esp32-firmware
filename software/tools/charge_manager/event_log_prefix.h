#pragma once

#include <stddef.h>

struct EventLog {
    void trace_timestamp();
    void write(const char *buf, size_t len);
    void printfln(const char *fmt, ...);
    void trace_write(const char *buf);
    void tracefln(const char *fmt, ...);
};

extern EventLog logger;

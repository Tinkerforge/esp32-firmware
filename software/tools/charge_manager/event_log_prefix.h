#pragma once

#include <stddef.h>

struct EventLog {
    void write(const char *buf, size_t len);
    void printfln(const char *fmt, ...);
};

extern EventLog logger;

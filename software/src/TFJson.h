#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

struct TFJsonSerializer {
    char * const buf;
    const size_t buf_size;

    char *head;

    size_t buf_required;
    bool in_empty_container = true;

    // To get the required buffer size, construct with buf = nullptr and buf_size = 0 and construct your JSON payload.
    // TFJsonSerializer::end() will return the required buffer size WITHOUT NULL TERMINATOR!
    TFJsonSerializer(char *buf, size_t buf_size);

    // Object
    void add(const char *key, uint32_t u);
    void add(const char *key, int32_t i);
    void add(const char *key, float f);
    void add(const char *key, bool b);
    void addNull(const char *key);
    void add(const char *key, const char *c);
    void addArray(const char *key);
    void addObject(const char *key);

    // Array
    void add(uint32_t u, bool enquote = false);
    void add(int32_t i);
    void add(float f);
    void add(bool b);
    void addNull();
    void add(const char *c);
    void addArray();
    void addObject();

    // Both
    void endArray();
    void endObject();
    size_t end();

private:
    void addKey(const char *key);
    void write(const char *c);
    void write(char c);
    void writeUnescaped(const char *c, size_t len);
    void writeFmt(const char *fmt, ...) __attribute__((__format__(__printf__, 2, 3)));
    void back(size_t n);
};

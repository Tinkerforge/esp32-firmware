#ifndef TFJSON_H
#define TFJSON_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <inttypes.h>
#include <limits>
#include <functional>

#define TFJSON_USE_STRLEN std::numeric_limits<size_t>::max()

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
    void add(const char *key, uint64_t u);
    void add(const char *key, int64_t i);
    void add(const char *key, double f);
    void add(const char *key, bool b);
    void addNull(const char *key);
    void add(const char *key, const char *c);
    void addArray(const char *key);
    void addObject(const char *key);

    // Array or top level
    void add(uint64_t u, bool enquote = false);
    void add(int64_t i);
    void add(double f);
    void add(bool b);
    void addNull();
    void add(const char *c, size_t len = TFJSON_USE_STRLEN, bool enquote = true);
    void addArray();
    void addObject();

    // Both
    void endArray();
    void endObject();
    size_t end();

private:
    void addKey(const char *key);
    void write(const char *c, size_t len = TFJSON_USE_STRLEN);
    void write(char c);
    void writeUnescaped(const char *c, size_t len);
    void writeFmt(const char *fmt, ...) __attribute__((__format__(__printf__, 2, 3)));
};

struct TFJsonDeserializer {
    enum class Error {
        Aborted,
        ExpectingEndOfInput,
        ExpectingValue,
        ExpectingOpeningCurlyBracket,
        ExpectingClosingCurlyBracket,
        ExpectingColon,
        ExpectingOpeningSquareBracket,
        ExpectingClosingSquareBracket,
        ExpectingOpeningQuote,
        ExpectingClosingQuote,
        ExpectingNumber,
        ExpectingFractionDigits,
        ExpectingExponentDigits,
        ExpectingNull,
        ExpectingTrue,
        ExpectingFalse,
        InvalidEscapeSequence,
        UnescapedControlCharacter,
        NestingTooDeep,
        InlineNullByte,
        InvalidUTF8StartByte,
        InvalidUTF8ContinuationByte,
    };

    const size_t nesting_depth_max;
    size_t nesting_depth;
    size_t utf8_count;
    char *buf;
    size_t buf_len;
    ssize_t idx_cur;  // current character
    ssize_t idx_okay; // no parsing error until here [inclusive]
    ssize_t idx_done; // data is not needed anymore until here [inclusive]
    char cur;
    std::function<void(Error, size_t)> error_handler;
    std::function<bool(void)> begin_handler;
    std::function<bool(void)> end_handler;
    std::function<bool(void)> object_begin_handler;
    std::function<bool(void)> object_end_handler;
    std::function<bool(void)> array_begin_handler;
    std::function<bool(void)> array_end_handler;
    std::function<bool(char *, size_t)> member_handler;
    std::function<bool(char *, size_t)> string_handler;
    std::function<bool(double)> double_handler;
    std::function<bool(int64_t)> int64_handler;
    std::function<bool(uint64_t)> uint64_handler;
    std::function<bool(char *, size_t)> number_handler;
    std::function<bool(bool)> bool_handler;
    std::function<bool(void)> null_handler;

    TFJsonDeserializer(size_t nesting_depth_max);

    static const char *getErrorName(Error error);

    void setErrorHandler(std::function<void(Error, size_t)> error_handler);
    void setBeginHandler(std::function<bool(void)> begin_handler);
    void setEndHandler(std::function<bool(void)> end_handler);
    void setObjectBeginHandler(std::function<bool(void)> object_begin_handler);
    void setObjectEndHandler(std::function<bool(void)> object_end_handler);
    void setArrayBeginHandler(std::function<bool(void)> array_begin_handler);
    void setArrayEndHandler(std::function<bool(void)> array_end_handler);
    void setMemberHandler(std::function<bool(char *, size_t)> member_handler);
    void setStringHandler(std::function<bool(char *, size_t)> string_handler);
    void setDoubleHandler(std::function<bool(double)> double_handler);
    void setInt64Handler(std::function<bool(int64_t)> int64_handler);
    void setUInt64Handler(std::function<bool(uint64_t)> uint64_handler);
    void setNumberHandler(std::function<bool(char *, size_t)> number_handler);
    void setBoolHandler(std::function<bool(bool)> bool_handler);
    void setNullHandler(std::function<bool(void)> null_handler);

    bool parse(char *buf, size_t len = TFJSON_USE_STRLEN);

private:
    void reportError(Error error);
    bool next();
    void okay(ssize_t offset = 0);
    void done();
    bool enterNesting();
    void leaveNesting();
    bool isWhitespace();
    bool isDigit();
    bool isHexDigit();
    bool isControl();
    bool skipWhitespace();
    bool parseElements();
    bool parseElement();
    bool parseValue();
    bool parseObject();
    bool parseMembers();
    bool parseMember();
    bool parseArray();
    bool parseString(bool report_as_member_name = false);
    bool parseNumber();
    bool parseNull();
    bool parseTrue();
    bool parseFalse();
};

#endif

#ifdef TFJSON_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <inttypes.h>
#include <assert.h>

#if 0
#define debugf(...) printf("TFJsonDeserializer: " __VA_ARGS__)
#else
#define debugf(...) (void)0
#endif

// Use this macro and pass length to writeUnescaped so that the compiler can see (and create constants of) the string literal lengths.
#define WRITE_LITERAL(x) this->writeUnescaped((x), strlen((x)))

TFJsonSerializer::TFJsonSerializer(char *buf, size_t buf_size) : buf(buf), buf_size(buf_size), head(buf), buf_required(0) {}

void TFJsonSerializer::add(const char *key, uint64_t u) {
    this->addKey(key);
    this->add(u);
}

void TFJsonSerializer::add(const char *key, int64_t i) {
    this->addKey(key);
    this->add(i);
}

void TFJsonSerializer::add(const char *key, double f) {
    this->addKey(key);
    this->add(f);
}

void TFJsonSerializer::add(const char *key, bool b) {
    this->addKey(key);
    this->add(b);
}

void TFJsonSerializer::addNull(const char *key) {
    this->addKey(key);
    this->addNull();
}

void TFJsonSerializer::add(const char *key, const char *c) {
    this->addKey(key);
    this->add(c);
}

void TFJsonSerializer::addArray(const char *key) {
    this->addKey(key);
    this->write('[');
}

void TFJsonSerializer::addObject(const char *key) {
    this->addKey(key);
    this->write('{');
}

void TFJsonSerializer::add(uint64_t u, bool enquote) {
    if (!in_empty_container)
        this->write(',');

    in_empty_container = false;
    if (enquote)
        this->write('"');

    this->writeFmt("%" PRIu64, u);

    if (enquote)
        this->write('"');
}

void TFJsonSerializer::add(int64_t i) {
    if (!in_empty_container)
        this->write(',');

    in_empty_container = false;

    this->writeFmt("%" PRIi64, i);
}

void TFJsonSerializer::add(double f) {
    if (!in_empty_container)
        this->write(',');

    in_empty_container = false;

    if (isfinite(f))
        this->writeFmt("%f", f);
    else
        WRITE_LITERAL("null");
}

void TFJsonSerializer::add(bool b) {
    if (!in_empty_container)
        this->write(',');

    in_empty_container = false;

    if (b)
        WRITE_LITERAL("true");
    else
        WRITE_LITERAL("false");
}

void TFJsonSerializer::addNull() {
    if (!in_empty_container)
        this->write(',');

    in_empty_container = false;

    WRITE_LITERAL("null");
}

void TFJsonSerializer::add(const char *c, size_t len, bool enquote) {
    if (!in_empty_container)
        this->write(',');

    in_empty_container = false;

    if (enquote)
        this->write('\"');

    this->write(c, len);

    if (enquote)
        this->write('\"');
}

void TFJsonSerializer::addArray() {
    if (!in_empty_container)
        this->write(',');

    in_empty_container = true;

    WRITE_LITERAL("[");
}

void TFJsonSerializer::addObject() {
    if (!in_empty_container)
        this->write(',');

    in_empty_container = true;

    WRITE_LITERAL("{");
}

void TFJsonSerializer::endArray() {
    in_empty_container = false;

    WRITE_LITERAL("]");
}

void TFJsonSerializer::endObject() {
    in_empty_container = false;

    WRITE_LITERAL("}");
}

size_t TFJsonSerializer::end() {
    // Return required buffer size _without_ the null terminator.
    // This mirrors the behaviour of snprintf.
    size_t result = buf_required;
    this->write('\0');
    if (buf_size > 0 && result >= buf_size)
        buf[buf_size - 1] = '\0';

    return result;
}

void TFJsonSerializer::addKey(const char *key) {
    if (!in_empty_container)
        this->write(",");

    in_empty_container = true;

    this->write('\"');
    this->write(key);
    WRITE_LITERAL("\":");
}

/*
    All code points may
    be placed within the quotation marks except for the code points that must be escaped: quotation mark
    (U+0022), reverse solidus (U+005C), and the control characters U+0000 to U+001F.
*/
void TFJsonSerializer::write(const char *c, size_t len) {
    const char *end = c + (len == TFJSON_USE_STRLEN ? strlen(c) : len);

    while(c != end) {
        switch (*c) {
            case '\\':
                write('\\');
                write('\\');
                break;
            case '"':
                write('\\');
                write('"');
                break;
            case '\b':
                write('\\');
                write('b');
                break;
            case '\f':
                write('\\');
                write('f');
                break;
            case '\n':
                write('\\');
                write('n');
                break;
            case '\r':
                write('\\');
                write('r');
                break;
            case '\t':
                write('\\');
                write('t');
                break;
            default:
#if CHAR_MIN == 0
                if (*c <= 0x1F) {
#else
                if (*c <= 0x1F && *c >= 0/*UTF-8 compatibility*/) {
#endif
                    char x = *c;

                    write('\\');
                    write('u');
                    write('0');
                    write('0');
                    write(x & 0x10 ? '1' : '0');

                    x &= 0x0F;

                    if (x >= 10)
                        write('A' + (x - 10));
                    else
                        write('0' + (x));
                }
                else
                    write(*c);
                break;
        }
        ++c;
    }
}

void TFJsonSerializer::write(char c) {
    ++buf_required;

    if (buf_size == 0 || (size_t)(head - buf) > (buf_size - 1))
        return;

    *head = c;
    ++head;
}

void TFJsonSerializer::writeUnescaped(const char *c, size_t len) {
    buf_required += len;

    if (len > buf_size || (size_t)(head - buf) > (buf_size - len))
        return;

    memcpy(head, c, len);
    head += len;
}

void TFJsonSerializer::writeFmt(const char *fmt, ...) {
    size_t buf_left = (head >= buf + buf_size) ? 0 : buf_size - (size_t)(head - buf);

    va_list args;
    va_start(args, fmt);
    int w = vsnprintf(head, buf_left, fmt, args);
    va_end(args);

    if (w < 0) {
        // don't move head if vsnprintf fails completely.
        return;
    }

    buf_required += (size_t)w;

    if (buf_size == 0)
        return;

    if ((size_t)w >= buf_left) {
        head = buf + buf_size;

        buf[buf_size - 1] = '\0';
        return;
    }

    head += (size_t)w;
    return;
}

TFJsonDeserializer::TFJsonDeserializer(size_t nesting_depth_max) : nesting_depth_max(nesting_depth_max) {}

const char *TFJsonDeserializer::getErrorName(Error error) {
    switch (error) {
        case Error::Aborted: return "Aborted";
        case Error::ExpectingEndOfInput: return "ExpectingEndOfInput";
        case Error::ExpectingValue: return "ExpectingValue";
        case Error::ExpectingOpeningCurlyBracket: return "ExpectingOpeningCurlyBracket";
        case Error::ExpectingClosingCurlyBracket: return "ExpectingClosingCurlyBracket";
        case Error::ExpectingColon: return "ExpectingColon";
        case Error::ExpectingOpeningSquareBracket: return "ExpectingOpeningSquareBracket";
        case Error::ExpectingClosingSquareBracket: return "ExpectingClosingSquareBracket";
        case Error::ExpectingOpeningQuote: return "ExpectingOpeningQuote";
        case Error::ExpectingClosingQuote: return "ExpectingClosingQuote";
        case Error::ExpectingNumber: return "ExpectingNumber";
        case Error::ExpectingFractionDigits: return "ExpectingFractionDigits";
        case Error::ExpectingExponentDigits: return "ExpectingExponentDigits";
        case Error::ExpectingNull: return "ExpectingNull";
        case Error::ExpectingTrue: return "ExpectingTrue";
        case Error::ExpectingFalse: return "ExpectingFalse";
        case Error::InvalidEscapeSequence: return "InvalidEscapeSequence";
        case Error::UnescapedControlCharacter: return "UnescapedControlCharacter";
        case Error::NestingTooDeep: return "NestingTooDeep";
        case Error::InlineNullByte: return "InlineNullByte";
        case Error::InvalidUTF8StartByte: return "InvalidUTF8StartByte";
        case Error::InvalidUTF8ContinuationByte: return "InvalidUTF8ContinuationByte";
        default: return "Unknown";
    }
}

void TFJsonDeserializer::setErrorHandler(std::function<void(Error, size_t)> error_handler_) { error_handler = error_handler_; }

void TFJsonDeserializer::setBeginHandler(std::function<bool(void)> begin_handler_) { begin_handler = begin_handler_; }

void TFJsonDeserializer::setEndHandler(std::function<bool(void)> end_handler_) { end_handler = end_handler_; }

void TFJsonDeserializer::setObjectBeginHandler(std::function<bool(void)> object_begin_handler_) { object_begin_handler = object_begin_handler_; }

void TFJsonDeserializer::setObjectEndHandler(std::function<bool(void)> object_end_handler_) { object_end_handler = object_end_handler_; }

void TFJsonDeserializer::setArrayBeginHandler(std::function<bool(void)> array_begin_handler_) { array_begin_handler = array_begin_handler_; }

void TFJsonDeserializer::setArrayEndHandler(std::function<bool(void)> array_end_handler_) { array_end_handler = array_end_handler_; }

void TFJsonDeserializer::setMemberHandler(std::function<bool(char *, size_t)> member_handler_) { member_handler = member_handler_; }

void TFJsonDeserializer::setStringHandler(std::function<bool(char *, size_t)> string_handler_) { string_handler = string_handler_; }

void TFJsonDeserializer::setDoubleHandler(std::function<bool(double)> double_handler_) { double_handler = double_handler_; }

void TFJsonDeserializer::setInt64Handler(std::function<bool(int64_t)> int64_handler_) { int64_handler = int64_handler_; }

void TFJsonDeserializer::setUInt64Handler(std::function<bool(uint64_t)> uint64_handler_) { uint64_handler = uint64_handler_; }

void TFJsonDeserializer::setNumberHandler(std::function<bool(char *, size_t)> number_handler_) { number_handler = number_handler_; }

void TFJsonDeserializer::setBoolHandler(std::function<bool(bool)> bool_handler_) { bool_handler = bool_handler_; }

void TFJsonDeserializer::setNullHandler(std::function<bool(void)> null_handler_) { null_handler = null_handler_; }

bool TFJsonDeserializer::parse(char *buf_, size_t buf_len_) {
    nesting_depth = 0;
    utf8_count = 0;
    buf = buf_;
    buf_len = buf_len_ == TFJSON_USE_STRLEN ? strlen(buf) : buf_len_;
    idx_cur = -1;
    idx_okay = -1;
    idx_done = -1;

    debugf("parse(%p, %zu) -> \"%.*s\"\n", buf, buf_len, (int)buf_len, buf);

    if (begin_handler && !begin_handler()) {
        reportError(Error::Aborted);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (!parseElement()) {
        return false;
    }

    if (idx_done + 1 < (ssize_t)buf_len) {
        reportError(Error::ExpectingEndOfInput);
        return false;
    }

    if (end_handler && !end_handler()) {
        reportError(Error::Aborted);
        return false;
    }

    debugf("parse(...) -> buf_len: %zu, idx_cur: %zd, idx_okay: %zd, idx_done: %zd\n", buf_len, idx_cur, idx_okay, idx_done);

    return true;
}

void TFJsonDeserializer::reportError(Error error) {
    debugf("reportError(%s, idx_cur: %zd, idx_okay: %zd) -> \"%.*s\"\n", getErrorName(error), idx_cur, idx_okay, (int)(buf_len - idx_okay), buf + idx_okay + 1);

    if (error_handler) {
        error_handler(error, idx_okay + 1);
    }
}

static int count_leading_ones_intrinsic(char value) {
    uint8_t bits = ~(uint8_t)value;

    if (bits == 0) {
        return 8;
    }

    return __builtin_clz(bits) - 24;
}

bool TFJsonDeserializer::next() {
    if (idx_cur + 1 >= (ssize_t)buf_len) {
        idx_cur = (ssize_t)buf_len;
        cur = '\0';
    } else {
        ++idx_cur;
        cur = buf[idx_cur];

        if (cur == '\0') {
            okay(-1);

            reportError(Error::InlineNullByte);
            return false;
        }
    }

    debugf("next() -> idx_cur: %zd, utf8_count: %zu, cur: '%c' [0x%02x]\n", idx_cur, utf8_count, cur, (uint8_t)cur);

    if (utf8_count > 0) {
        if (((uint8_t)cur & 0b11000000) != 0b10000000) {
            okay(-1);

            reportError(Error::InvalidUTF8ContinuationByte);
            return false;
        }

        --utf8_count;
    }
    else {
        utf8_count = count_leading_ones_intrinsic(cur);

        if (utf8_count != 0 && (utf8_count < 2 || utf8_count > 4)) {
            okay(-1);

            reportError(Error::InvalidUTF8StartByte);
            return false;
        }

        if (utf8_count > 0) {
            --utf8_count;
        }
    }

    return true;
}

void TFJsonDeserializer::okay(ssize_t offset) {
    idx_okay = idx_cur + offset;

    debugf("okay(offset: %zd) -> idx_okay: %zd\n", offset, idx_okay);
}

void TFJsonDeserializer::done() {
    idx_done = idx_okay;

    debugf("done() -> idx_done: %zd\n", idx_done);
}

bool TFJsonDeserializer::enterNesting() {
    if (nesting_depth >= nesting_depth_max) {
        reportError(Error::NestingTooDeep);
        return false;
    }

    ++nesting_depth;

    return true;
}

void TFJsonDeserializer::leaveNesting() {
    assert(nesting_depth > 0);

    --nesting_depth;
}

bool TFJsonDeserializer::isWhitespace() {
    switch (cur) {
        case ' ':
        case '\r':
        case '\n':
        case '\t':
            return true;

        default:
            return false;
    }
}

bool TFJsonDeserializer::isDigit() {
    return cur >= '0' && cur <= '9';
}

bool TFJsonDeserializer::isHexDigit() {
    return isDigit() || (cur >= 'a' && cur <= 'f') || (cur >= 'A' && cur <= 'F');
}

bool TFJsonDeserializer::isControl() {
    // JSON allows 0x7F unescaped
#if CHAR_MIN == 0
    return cur <= 0x1F;
#else
    return cur >= 0x00 && cur <= 0x1F;
#endif
}

bool TFJsonDeserializer::skipWhitespace() {
    while (isWhitespace()) {
        debugf("skipWhitespace(cur: '%c' [0x%02x])\n", cur, (uint8_t)cur);

        okay();
        done();

        if (!next()) {
            return false;
        }
    }

    return true;
}

bool TFJsonDeserializer::parseElements() {
    if (!parseElement()) {
        return false;
    }

    while (cur == ',') {
        okay();
        done();

        if (!next()) {
            return false;
        }

        if (!parseElement()) {
            return false;
        }
    }

    return true;
}

bool TFJsonDeserializer::parseElement() {
    if (!skipWhitespace()) {
        return false;
    }

    if (!parseValue()) {
        return false;
    }

    if (!skipWhitespace()) {
        return false;
    }

    return true;
}

bool TFJsonDeserializer::parseValue() {
    switch (cur) {
        case '{':
            return parseObject();

        case '[':
            return parseArray();

        case '"':
            return parseString(false);

        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return parseNumber();

        case 'n':
            return parseNull();

        case 't':
            return parseTrue();

        case 'f':
            return parseFalse();

        default:
            reportError(Error::ExpectingValue);
            return false;
    }
}

bool TFJsonDeserializer::parseObject() {
    if (cur != '{') {
        reportError(Error::ExpectingOpeningCurlyBracket);
        return false;
    }

    okay();
    done();

    if (!enterNesting()) {
        return false;
    }

    if (object_begin_handler && !object_begin_handler()) {
        reportError(Error::Aborted);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (!skipWhitespace()) {
        return false;
    }

    if (cur == '}') {
        okay();
        done();

        if (object_end_handler && !object_end_handler()) {
            reportError(Error::Aborted);
            return false;
        }

        leaveNesting();

        if (!next()) {
            return false;
        }

        return true;
    }

    if (!parseMembers()) {
        return false;
    }

    if (cur != '}') {
        reportError(Error::ExpectingClosingCurlyBracket);
        return false;
    }

    okay();
    done();

    if (object_end_handler && !object_end_handler()) {
        reportError(Error::Aborted);
        return false;
    }

    leaveNesting();

    if (!next()) {
        return false;
    }

    return true;
}

bool TFJsonDeserializer::parseMembers() {
    if (!parseMember()) {
        return false;
    }

    while (cur == ',') {
        okay();
        done();

        if (!next()) {
            return false;
        }

        if (!parseMember()) {
            return false;
        }
    }

    return true;
}

bool TFJsonDeserializer::parseMember() {
    if (!skipWhitespace()) {
        return false;
    }

    if (!parseString(true)) {
        return false;
    }

    if (!skipWhitespace()) {
        return false;
    }

    if (cur != ':') {
        reportError(Error::ExpectingColon);
        return false;
    }

    okay();
    done();

    if (!next()) {
        return false;
    }

    return parseElement();
}

bool TFJsonDeserializer::parseArray() {
    if (cur != '[') {
        reportError(Error::ExpectingOpeningSquareBracket);
        return false;
    }

    okay();
    done();

    if (!enterNesting()) {
        return false;
    }

    if (array_begin_handler && !array_begin_handler()) {
        reportError(Error::Aborted);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (!skipWhitespace()) {
        return false;
    }

    if (cur == ']') {
        okay();
        done();

        if (array_end_handler && !array_end_handler()) {
            reportError(Error::Aborted);
            return false;
        }

        leaveNesting();

        if (!next()) {
            return false;
        }

        return true;
    }

    if (!parseElements()) {
        return false;
    }

    if (cur != ']') {
        reportError(Error::ExpectingClosingSquareBracket);
        return false;
    }

    okay();
    done();

    if (array_end_handler && !array_end_handler()) {
        reportError(Error::Aborted);
        return false;
    }

    leaveNesting();

    if (!next()) {
        return false;
    }

    return true;
}

bool TFJsonDeserializer::parseString(bool report_as_member) {
    if (cur != '"') {
        reportError(Error::ExpectingOpeningQuote);
        return false;
    }

    okay();
    done();

    if (!next()) {
        return false;
    }

    char *str = buf + idx_cur;
    char *end = str;

    while (cur != '"') {
        if (cur == '\0') {
            reportError(Error::ExpectingClosingQuote);
            return false;
        }

        if (cur != '\\') {
            if (isControl()) {
                reportError(Error::UnescapedControlCharacter);
                return false;
            }

            *end++ = cur;

            okay();

            if (!next()) {
                return false;
            }

            continue;
        }

        if (!next()) {
            return false;
        }

        char unescaped = '\0';

        switch (cur) {
            case '"':
                unescaped = '"';
                break;

            case '\\':
                unescaped = '\\';
                break;

            case '/':
                unescaped = '/';
                break;

            case 'b':
                unescaped = '\b';
                break;

            case 'f':
                unescaped = '\f';
                break;

            case 'n':
                unescaped = '\n';
                break;

            case 'r':
                unescaped = '\r';
                break;

            case 't':
                unescaped = '\t';
                break;
        }

        if (unescaped != '\0') {
            *end++ = unescaped;

            okay();

            if (!next()) {
                return false;
            }

            continue;
        }

        if (cur == 'u') {
            if (!next()) {
                return false;
            }

            char hex[5] = {0};

            for (int i = 0; i < 4; ++i) {
                if (!isHexDigit()) {
                    reportError(Error::InvalidEscapeSequence);
                    return false;
                }

                hex[i] = cur;

                if (!next()) {
                    return false;
                }
            }

            okay();

            uint32_t code_point = strtoul(hex, nullptr, 16);

            if (code_point <= 0x7F) {
                *end++ = (char)code_point;
            }
            else if (code_point <= 0x07FF) {
                *end++ = (char)(((code_point >> 6) & 0x1F) | 0xC0);
                *end++ = (char)(((code_point >> 0) & 0x3F) | 0x80);
            }
            else if (code_point <= 0xFFFF) {
                *end++ = (char)(((code_point >> 12) & 0x0F) | 0xE0);
                *end++ = (char)(((code_point >>  6) & 0x3F) | 0x80);
                *end++ = (char)(((code_point >>  0) & 0x3F) | 0x80);
            }
            else if (code_point <= 0x10FFFF) {
                *end++ = (char)(((code_point >> 18) & 0x07) | 0xF0);
                *end++ = (char)(((code_point >> 12) & 0x3F) | 0x80);
                *end++ = (char)(((code_point >>  6) & 0x3F) | 0x80);
                *end++ = (char)(((code_point >>  0) & 0x3F) | 0x80);
            }
            else {
                reportError(Error::InvalidEscapeSequence);
                return false;
            }

            continue;
        }

        reportError(Error::InvalidEscapeSequence);
        return false;
    }

    okay();

    size_t str_len = end - str;

    debugf("parseString(report_as_member: %s) -> \"%.*s\"\n", report_as_member ? "true" : "false", (int)str_len, str);

    if (report_as_member) {
        if (member_handler && !member_handler(str, str_len)) {
            reportError(Error::Aborted);
            return false;
        }
    }
    else {
        if (string_handler && !string_handler(str, str_len)) {
            reportError(Error::Aborted);
            return false;
        }
    }

    done();

    if (!next()) {
        return false;
    }

    return true;
}

bool TFJsonDeserializer::parseNumber() {
    char *number = buf + idx_cur;

    if (cur == '-') {
        if (!next()) {
            return false;
        }
    }

    if (!isDigit()) {
        reportError(Error::ExpectingNumber);
        return false;
    }

    char first_digit = cur;

    if (!next()) {
        return false;
    }

    if (first_digit != '0') {
        while (isDigit()) {
            if (!next()) {
                return false;
            }
        }
    }

    bool has_fraction_or_exponent = false;

    if (cur == '.') {
        if (!next()) {
            return false;
        }

        has_fraction_or_exponent = true;

        if (!isDigit()) {
            okay(-1);

            reportError(Error::ExpectingFractionDigits);
            return false;
        }

        while (isDigit()) {
            if (!next()) {
                return false;
            }
        }
    }

    if (cur == 'e' || cur == 'E') {
        if (!next()) {
            return false;
        }

        has_fraction_or_exponent = true;

        if (cur == '-' || cur == '+') {
            if (!next()) {
                return false;
            }
        }

        if (!isDigit()) {
            okay(-1);

            reportError(Error::ExpectingExponentDigits);
            return false;
        }

        while (isDigit()) {
            if (!next()) {
                return false;
            }
        }
    }

    size_t number_len = buf + idx_cur - number;

    debugf("parseNumber() -> \"%.*s\"\n", (int)number_len, number);

    if (has_fraction_or_exponent) {
        char backup = number[number_len];

        number[number_len] = '\0';
        errno = 0;

        double result = strtod(number, nullptr);

        number[number_len] = backup;

        okay(-1);

        if (errno != 0) {
            if (number_handler && !number_handler(number, number_len)) {
                reportError(Error::Aborted);
                return false;
            }
        }
        else {
            debugf("parseNumber() -> \"%.*s\" = %f\n", (int)number_len, number, result);

            if (double_handler && !double_handler(result)) {
                reportError(Error::Aborted);
                return false;
            }
        }
    }
    else if (*number == '-') {
        char backup = number[number_len];

        number[number_len] = '\0';
        errno = 0;

        int64_t result = strtoll(number, nullptr, 10);

        number[number_len] = backup;

        okay(-1);

        if (errno != 0) {
            if (number_handler && !number_handler(number, number_len)) {
                reportError(Error::Aborted);
                return false;
            }
        }
        else {
            debugf("parseNumber() -> \"%.*s\" = %" PRIi64 "\n", (int)number_len, number, result);

            if (int64_handler && !int64_handler(result)) {
                reportError(Error::Aborted);
                return false;
            }
        }
    }
    else {
        char backup = number[number_len];

        number[number_len] = '\0';
        errno = 0;

        uint64_t result = strtoull(number, nullptr, 10);

        number[number_len] = backup;

        okay(-1);

        if (errno != 0) {
            if (number_handler && !number_handler(number, number_len)) {
                reportError(Error::Aborted);
                return false;
            }
        }
        else {
            debugf("parseNumber() -> \"%.*s\" = %" PRIu64 "\n", (int)number_len, number, result);

            if (uint64_handler && !uint64_handler(result)) {
                reportError(Error::Aborted);
                return false;
            }
        }
    }

    done();

    return true;
}

bool TFJsonDeserializer::parseNull() {
    if (cur != 'n') {
        reportError(Error::ExpectingNull);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (cur != 'u') {
        reportError(Error::ExpectingNull);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (cur != 'l') {
        reportError(Error::ExpectingNull);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (cur != 'l') {
        reportError(Error::ExpectingNull);
        return false;
    }

    okay();

    if (null_handler && !null_handler()) {
        reportError(Error::Aborted);
        return false;
    }

    done();

    if (!next()) {
        return false;
    }

    return true;
}

bool TFJsonDeserializer::parseTrue() {
    if (cur != 't') {
        reportError(Error::ExpectingTrue);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (cur != 'r') {
        reportError(Error::ExpectingTrue);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (cur != 'u') {
        reportError(Error::ExpectingTrue);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (cur != 'e') {
        reportError(Error::ExpectingTrue);
        return false;
    }

    okay();

    if (bool_handler && !bool_handler(true)) {
        reportError(Error::Aborted);
        return false;
    }

    done();

    if (!next()) {
        return false;
    }

    return true;
}

bool TFJsonDeserializer::parseFalse() {
    if (cur != 'f') {
        reportError(Error::ExpectingFalse);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (cur != 'a') {
        reportError(Error::ExpectingFalse);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (cur != 'l') {
        reportError(Error::ExpectingFalse);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (cur != 's') {
        reportError(Error::ExpectingFalse);
        return false;
    }

    if (!next()) {
        return false;
    }

    if (cur != 'e') {
        reportError(Error::ExpectingFalse);
        return false;
    }

    okay();

    if (bool_handler && !bool_handler(false)) {
        reportError(Error::Aborted);
        return false;
    }

    done();

    if (!next()) {
        return false;
    }

    return true;
}

#endif

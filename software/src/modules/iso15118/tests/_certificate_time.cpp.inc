#include "tools/certificate_time.h"
#include <cassert>
#include <cstdio>

int main()
{
#ifdef MBEDTLS_HAVE_TIME_DATE
#error This regression must run with global date checking disabled
#endif
    mbedtls_x509_crt cert{};
    cert.valid_from = {2026, 1, 1, 0, 0, 0};
    cert.valid_to = {2026, 1, 2, 0, 0, 0};
    constexpr time_t start = 1767225600;
    constexpr time_t end = start + 86400;
    assert(certificate_time_flags(cert, start - 1) == MBEDTLS_X509_BADCERT_FUTURE);
    assert(certificate_time_flags(cert, start) == 0);
    assert(certificate_time_flags(cert, end) == 0);
    assert(certificate_time_flags(cert, end + 1) == MBEDTLS_X509_BADCERT_EXPIRED);
    assert(certificate_time_flags(cert, 0) == MBEDTLS_X509_BADCERT_OTHER);
    assert(certificate_time_flags(cert, 3600) == MBEDTLS_X509_BADCERT_OTHER);
    cert.valid_from = {2024, 2, 29, 0, 0, 0};
    cert.valid_to = {2024, 3, 1, 0, 0, 0};
    assert(certificate_time_flags(cert, 1709164800) == 0);
    assert(certificate_time_flags(cert, 1709251200) == 0);
    puts("PASS explicit certificate date boundaries, leap day and boot epoch");
}

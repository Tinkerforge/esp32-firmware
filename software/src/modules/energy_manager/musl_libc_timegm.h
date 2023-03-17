#pragma once
// timegm is a nonstandard GNU extension that are also present on the BSDs. Declare extern here to require platform to implement it.
#include <time.h>

#if !defined(timegm)
    time_t timegm(struct tm *tm);
#endif

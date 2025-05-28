/* esp32-firmware
 * Copyright (C) 2025 Matthias Bolte <matthias@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "tools/float.h"

#include <math.h>

#include "gcc_warnings.h"

[[gnu::const]]
bool is_exactly_zero(float f)
{
    #if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
    return f == 0.0f; // Really compare exactly with 0.0f
#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
}

[[gnu::const]]
float nan_safe_sum(float a, float b)
{
    if (isnan(a)) {
        return b;
    }

    if (isnan(b)) {
        return a;
    }

    return a + b;
}

[[gnu::const]]
float nan_safe_avg(float a, float b)
{
    if (isnan(a)) {
        return b;
    }

    if (isnan(b)) {
        return a;
    }

    return (a + b) / 2.0f;
}

[[gnu::const]]
float zero_safe_negation(float f)
{
    if (!is_exactly_zero(f)) {
        // Don't convert 0.0f into -0.0f
        f *= -1.0f;
    }

    return f;
}

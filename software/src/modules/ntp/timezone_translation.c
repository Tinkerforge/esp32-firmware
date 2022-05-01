/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include <string.h>
#include <stdlib.h>

#include "timezone_translation.h"

static size_t timezone_prefix_len_global = 0;
static int compare(const void *const left, const void *const right)
{
    const char *const l = (const char *const)left;
    const char *const r = *((const char *const *)right);
    int cmp = strncmp(l, r, timezone_prefix_len_global);
    if (cmp == 0)
        return (int)(timezone_prefix_len_global - strlen(r));
    return cmp;
}

static const char *lookup_timezone_internal(const char *const timezone, const size_t timezone_len, const struct Table table)
{
    const char *const sep = strchr(timezone, '/');
    timezone_prefix_len_global = timezone_len;

    if (sep != NULL && sep > timezone)
        timezone_prefix_len_global = (size_t)(sep - timezone);

    const char *const *key = (const char *const *)bsearch(timezone, table.keys, table.len, sizeof(const char *), compare);

    if (key == NULL) {
        return NULL;
    }

    size_t idx = (size_t)(key - table.keys);
    const TableValue val = table.values[idx];
    if (val.leaf)
        return val.value.posix_tz;

    if (sep == NULL) {
        return NULL;
    }

    return lookup_timezone_internal(sep + 1, timezone_len - timezone_prefix_len_global - 1, *val.value.sub_table);
}

const char *lookup_timezone(const char *timezone)
{
    return lookup_timezone_internal(timezone, strlen(timezone), global);
}

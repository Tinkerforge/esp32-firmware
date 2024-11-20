/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#pragma once

#include "esp_netif_ip_addr.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"

    // Cast from sockaddr to sockaddr_in increases required alignment
    #pragma GCC diagnostic ignored "-Wcast-align"
#endif

// lwIP ipaddr

extern inline void tf_ip4addr_ntoa(const ip4_addr_t *addr, char *buf, int buflen) {
    ip4addr_ntoa_r(addr, buf, buflen);
}

extern inline void tf_ip4addr_ntoa(const ip_addr_t *addr, char *buf, int buflen) {
    tf_ip4addr_ntoa(&addr->u_addr.ip4, buf, buflen);
}

// lwIP inaddr

extern inline void tf_ip4addr_ntoa(const struct in_addr *addr, char *buf, int buflen) {
    tf_ip4addr_ntoa(reinterpret_cast<const ip4_addr_t *>(addr), buf, buflen);
}

// lwIP sockaddr

extern inline void tf_ip4addr_ntoa(const struct sockaddr_in *addr, char *buf, int buflen) {
    tf_ip4addr_ntoa(&addr->sin_addr, buf, buflen);
}

extern inline void tf_ip4addr_ntoa(const struct sockaddr *addr, char *buf, int buflen) {
    tf_ip4addr_ntoa(reinterpret_cast<const struct sockaddr_in *>(addr), buf, buflen);
}

extern inline void tf_ip4addr_ntoa(const struct sockaddr_storage *addr, char *buf, int buflen) {
    tf_ip4addr_ntoa(reinterpret_cast<const struct sockaddr_in *>(addr), buf, buflen);
}

// ESP netif IP adresses

extern inline void tf_ip4addr_ntoa(const esp_ip4_addr_t *addr, char *buf, int buflen) {
    tf_ip4addr_ntoa(reinterpret_cast<const ip4_addr_t *>(addr), buf, buflen);
}

extern inline void tf_ip4addr_ntoa(const esp_ip_addr_t *addr, char *buf, int buflen) {
    tf_ip4addr_ntoa(reinterpret_cast<const ip_addr_t *>(addr), buf, buflen);
}

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

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

#include <esp_netif_ip_addr.h>
#include <IPAddress.h>
#include <lwip/ip_addr.h>
#include <lwip/sockets.h>
#include <stdint.h>

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"

    // Cast from sockaddr to sockaddr_in increases required alignment
    #pragma GCC diagnostic ignored "-Wcast-align"
#endif

// IPv4

// lwIP ipaddr

extern inline void tf_ip4addr_ntoa(const ip4_addr_t *addr, char buf[INET_ADDRSTRLEN], int buflen) {
    ip4addr_ntoa_r(addr, buf, buflen);
}

extern inline void tf_ip4addr_ntoa(const ip_addr_t *addr, char buf[INET_ADDRSTRLEN], int buflen) {
    tf_ip4addr_ntoa(&addr->u_addr.ip4, buf, buflen);
}

// lwIP inaddr

extern inline void tf_ip4addr_ntoa(const struct in_addr *addr, char buf[INET_ADDRSTRLEN], int buflen) {
    tf_ip4addr_ntoa(reinterpret_cast<const ip4_addr_t *>(addr), buf, buflen);
}

// lwIP sockaddr

extern inline void tf_ip4addr_ntoa(const struct sockaddr_in *addr, char buf[INET_ADDRSTRLEN], int buflen) {
    tf_ip4addr_ntoa(&addr->sin_addr, buf, buflen);
}

extern inline void tf_ip4addr_ntoa(const struct sockaddr *addr, char buf[INET_ADDRSTRLEN], int buflen) {
    tf_ip4addr_ntoa(reinterpret_cast<const struct sockaddr_in *>(addr), buf, buflen);
}

extern inline void tf_ip4addr_ntoa(const struct sockaddr_storage *addr, char buf[INET_ADDRSTRLEN], int buflen) {
    tf_ip4addr_ntoa(reinterpret_cast<const struct sockaddr_in *>(addr), buf, buflen);
}

// ESP netif IP adresses

extern inline void tf_ip4addr_ntoa(const esp_ip4_addr_t *addr, char buf[INET_ADDRSTRLEN], int buflen) {
    tf_ip4addr_ntoa(reinterpret_cast<const ip4_addr_t *>(addr), buf, buflen);
}

extern inline void tf_ip4addr_ntoa(const esp_ip_addr_t *addr, char buf[INET_ADDRSTRLEN], int buflen) {
    tf_ip4addr_ntoa(reinterpret_cast<const ip_addr_t *>(addr), buf, buflen);
}

// TFModbusTCP

extern inline void tf_ip4addr_ntoa(const uint32_t *addr, char buf[INET_ADDRSTRLEN], int buflen) {
    ip4addr_ntoa_r(reinterpret_cast<const ip4_addr_t *>(addr), buf, buflen);
}


// IPv6

// lwIP ipaddr

extern inline void tf_ip6addr_ntoa(const ip6_addr_t *addr, char buf[INET6_ADDRSTRLEN], int buflen) {
    ip6addr_ntoa_r(addr, buf, buflen);
}

extern inline void tf_ip6addr_ntoa(const ip_addr_t *addr, char buf[INET6_ADDRSTRLEN], int buflen) {
    tf_ip6addr_ntoa(&addr->u_addr.ip6, buf, buflen);
}

// lwIP inaddr

extern inline void tf_ip6addr_ntoa(const struct in6_addr *addr, char buf[INET6_ADDRSTRLEN], int buflen) {
    tf_ip6addr_ntoa(reinterpret_cast<const ip6_addr_t *>(addr), buf, buflen);
}

// lwIP sockaddr

extern inline void tf_ip6addr_ntoa(const struct sockaddr_in6 *addr, char buf[INET6_ADDRSTRLEN], int buflen) {
    tf_ip6addr_ntoa(&addr->sin6_addr, buf, buflen);
}

extern inline void tf_ip6addr_ntoa(const struct sockaddr_storage *addr, char buf[INET6_ADDRSTRLEN], int buflen) {
    tf_ip6addr_ntoa(reinterpret_cast<const struct sockaddr_in6 *>(addr), buf, buflen);
}

// ESP netif IP adresses

extern inline void tf_ip6addr_ntoa(const esp_ip6_addr_t *addr, char buf[INET6_ADDRSTRLEN], int buflen) {
    tf_ip6addr_ntoa(reinterpret_cast<const ip6_addr_t *>(addr), buf, buflen);
}

extern inline void tf_ip6addr_ntoa(const esp_ip_addr_t *addr, char buf[INET6_ADDRSTRLEN], int buflen) {
    tf_ip6addr_ntoa(reinterpret_cast<const ip_addr_t *>(addr), buf, buflen);
}

// TFModbusTCP


// IPv4 or v6

// lwIP ipaddr

extern inline void tf_ipaddr_ntoa(const ip_addr_t *addr, char buf[INET6_ADDRSTRLEN], int buflen) {
    ipaddr_ntoa_r(addr, buf, buflen);
}


// lwIP inaddr

// lwIP sockaddr

extern inline void tf_ipaddr_ntoa(const struct sockaddr_storage *addr, char buf[INET6_ADDRSTRLEN], int buflen) {
    if (addr->ss_family == AF_INET)
        tf_ip4addr_ntoa(reinterpret_cast<const struct sockaddr_in *>(addr), buf, buflen);
    else if (addr->ss_family == AF_INET6)
        tf_ip6addr_ntoa(reinterpret_cast<const struct sockaddr_in6 *>(addr), buf, buflen);
    else if (buflen > 0)
        buf[0] = '\0';
}

// ESP netif IP adresses

extern inline void tf_ipaddr_ntoa(const esp_ip_addr_t *addr, char buf[INET6_ADDRSTRLEN], int buflen) {
    tf_ipaddr_ntoa(reinterpret_cast<const ip_addr_t *>(addr), buf, buflen);
}

// TFModbusTCP

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

IPAddress tf_sockaddr_storage2IPAddress(struct sockaddr_storage *addr, socklen_t addr_len);
IPAddress tf_local_address_of_sockfd(int sockfd);
IPAddress tf_peer_address_of_sockfd(int sockfd);

// IPv4 subnet functions
bool is_in_subnet(const IPAddress &ip, const IPAddress &subnet, const IPAddress &to_check);
bool is_valid_subnet_mask(const IPAddress &subnet);

[[gnu::const]] extern uint8_t tf_ip4addr_mask2cidr(const ip4_addr_t subnet);
[[gnu::const]] extern ip4_addr_t tf_ip4addr_cidr2mask(uint32_t cidr);

// IPv6 prefix/subnet functions

// Check if two IPv6 addresses share the same prefix of the given length (0-128).
bool is_in_subnet_ip6(const ip6_addr_t &addr_a, const ip6_addr_t &addr_b, uint8_t prefix_len);

// Check if an IPv6 prefix length is valid (0-128).
[[gnu::const]] extern bool is_valid_ip6_prefix_len(uint8_t prefix_len);

// Convert an IPv6 prefix length (0-128) to a 128-bit mask stored in an ip6_addr_t.
[[gnu::const]] extern ip6_addr_t tf_ip6addr_cidr2mask(uint8_t prefix_len);

// Convert a contiguous 128-bit mask in an ip6_addr_t to a prefix length (0-128).
// Returns 0 if the mask is not a valid contiguous prefix mask.
extern uint8_t tf_ip6addr_mask2cidr(const ip6_addr_t &mask);

// Dual-stack helpers

// Try to parse a string as an IPv4 or IPv6 address.
// Returns the IP type (IPv4 or IPv6) if parsing succeeded.
IPType tf_parse_ip_address(const char *str, ip_addr_t *out);

// Convert a contiguous ipv4 or ipv6 mask in an ip_addr_t to a prefix length (0-128).
// Returns 0 if the mask is not a valid contiguous prefix mask.
extern uint8_t tf_ipaddr_mask2cidr(const ip_addr_t &mask);

// Convert an lwIP ip_addr_t to an Arduino IPAddress. Handles both IPv4 and IPv6.
IPAddress tf_ip_addr_to_IPAddress(const ip_addr_t *addr);

// Check whether an IPAddress is IPv6.
extern inline bool tf_ip_is_v6(const IPAddress &addr)
{
    return addr.type() == IPv6;
}

// Check whether an IPAddress is IPv4.
extern inline bool tf_ip_is_v4(const IPAddress &addr)
{
    return addr.type() == IPv4;
}

void poke_localhost();

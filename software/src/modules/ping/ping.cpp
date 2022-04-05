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
#include "ping.h"

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "esp_event.h"
#include "ping/ping_sock.h"

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"

extern API api;
extern EventLog logger;
extern TaskScheduler task_scheduler;


// Used to print success/timeout only once
// 0 - init
// 1 - success
// 2 - timeout
uint8_t last_state = 0;
static void on_ping_success(esp_ping_handle_t hdl, void *args)
{
    if(last_state == 1) {
        return;
    }
    last_state = 1;

    uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
    logger.printfln("ping: %d bytes from %s icmp_seq=%d ttl=%d time=%d ms\n",
           recv_len, ipaddr_ntoa((ip_addr_t*)&target_addr), seqno, ttl, elapsed_time);
}

static void on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
    if(last_state == 2) {
        return;
    }
    last_state = 2;

    uint16_t seqno;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    logger.printfln("ping: From %s icmp_seq=%d timeout\n",ipaddr_ntoa((ip_addr_t*)&target_addr), seqno);
}


Ping::Ping()
{
    config = Config::Object({
        {"enable", Config::Bool(false)},
        {"host", Config::Str("", 0, 64)},
        {"interval", Config::Uint(60, 10)}
    });
}

void Ping::setup()
{
    initialized = true;

    api.restorePersistentConfig("ping/config", &config);
    if (!config.get("enable")->asBool())
        return;

    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.count = ESP_PING_COUNT_INFINITE;
    ping_config.interval_ms = config.get("interval")->asUint() * 1000;

    // parse IP address
    struct sockaddr_in6 sock_addr6;
    ip_addr_t target_addr;
    memset(&target_addr, 0, sizeof(target_addr));

    const char *host = config.get("host")->asCStr();

    if (inet_pton(AF_INET6, host, &sock_addr6.sin6_addr) == 1) {
        /* convert ip6 string to ip6 address */
        ipaddr_aton(host, &target_addr);
    } else {
        struct addrinfo hint;
        struct addrinfo *res = NULL;
        memset(&hint, 0, sizeof(hint));
        /* convert ip4 string or hostname to ip4 or ip6 address */
        if (getaddrinfo(host, NULL, &hint, &res) != 0) {
            logger.printfln("ping: unknown host %s\n", host);
            return;
        }
        if (res->ai_family == AF_INET) {
            struct in_addr addr4 = ((struct sockaddr_in *) (res->ai_addr))->sin_addr;
            inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
        } else {
            struct in6_addr addr6 = ((struct sockaddr_in6 *) (res->ai_addr))->sin6_addr;
            inet6_addr_to_ip6addr(ip_2_ip6(&target_addr), &addr6);
        }
        freeaddrinfo(res);
    }
    ping_config.target_addr = target_addr;

    /* set callback functions */
    esp_ping_callbacks_t cbs;
    cbs.cb_args = nullptr;
    cbs.on_ping_success = on_ping_success;
    cbs.on_ping_timeout = on_ping_timeout;
    cbs.on_ping_end = nullptr;

    esp_ping_handle_t ping;
    esp_ping_new_session(&ping_config, &cbs, &ping);
    esp_ping_start(ping);
}

void Ping::register_urls()
{
    api.addPersistentConfig("ping/config", &config, {}, 1000);
}

void Ping::loop()
{

}

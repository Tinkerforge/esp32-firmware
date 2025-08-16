/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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
#include "ship.h"

#include <esp_https_server.h>

#include "build.h"
#include "event_log_prefix.h"

#include "module_dependencies.h"
#include "tools.h"

#define SHIP_USE_INTERNAL_CERTS

#ifdef SHIP_USE_INTERNAL_CERTS
// .crt, .key und the corresponding SKI for testing
const char *ship_crt = "-----BEGIN CERTIFICATE-----\n\
MIIBxDCCAWugAwIBAgIRAtyCMn08UNUlV7c18gP8QvgwCgYIKoZIzj0EAwIwQjEL\n\
MAkGA1UEBhMCREUxDTALBgNVBAoTBERlbW8xDTALBgNVBAsTBERlbW8xFTATBgNV\n\
BAMTDERlbW8tVW5pdC0wMjAeFw0yMjA0MTcxNzQ2NDlaFw0zMjA0MTQxNzQ2NDla\n\
MEIxCzAJBgNVBAYTAkRFMQ0wCwYDVQQKEwREZW1vMQ0wCwYDVQQLEwREZW1vMRUw\n\
EwYDVQQDEwxEZW1vLVVuaXQtMDIwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARv\n\
6RjCK1tqv1ogMaVtlPcI4GXDas42IGvOyebPeeuFqImf6B4oVosaXa96I/IsxV73\n\
iUzXntJq0Q/SChBQKKWlo0IwQDAOBgNVHQ8BAf8EBAMCB4AwDwYDVR0TAQH/BAUw\n\
AwEB/zAdBgNVHQ4EFgQUQcmLG75fx2V84xGYGVHxLTBKtBkwCgYIKoZIzj0EAwID\n\
RwAwRAIgcN7IhCSK2T1VzWI6EbGEa8Uv1Cl8J5RV3833d/AEBqkCIDKGf5I42AuP\n\
nvBcrk/bo5UsVFlRPRqw0tCoTtqZY7et\n\
-----END CERTIFICATE-----\n";

const char *ship_key = "-----BEGIN EC PRIVATE KEY-----\n\
MHcCAQEEILjqbjlvJr6x1Us8xdSm7lc67zRuUzTXsrWnv3gtVU3YoAoGCCqGSM49\n\
AwEHoUQDQgAEb+kYwitbar9aIDGlbZT3COBlw2rONiBrzsnmz3nrhaiJn+geKFaL\n\
Gl2veiPyLMVe94lM157SatEP0goQUCilpQ==\n\
-----END EC PRIVATE KEY-----\n";

const char *ship_ski = "41c98b1bbe5fc7657ce311981951f12d304ab419";
#endif

extern "C" esp_err_t esp_crt_bundle_attach(void *conf);

void Ship::pre_setup()
{
    web_sockets.pre_setup();
}

void Ship::setup()
{
#ifdef SHIP_USE_INTERNAL_CERTS
    eebus.state.get("ski")->updateString(ship_ski);
#endif
    setup_wss();
    setup_mdns();
}

void Ship::setup_wss()
{
    logger.printfln("setup_wss_server start");

#ifndef SHIP_USE_INTERNAL_CERTS
    int cert_id = eebus.config.get("cert_id")->asInt();
    if (cert_id == -1) {
        logger.printfln("Certificate ID is not set");
        return;
    }

    size_t cert_crt_len = 0;
    auto cert_crt = certs.get_cert(cert_id, &cert_crt_len);
    if (cert_crt == nullptr) {
        logger.printfln("Certificate with ID 0 is not available");
        return;
    }

    int key_id = eebus.config.get("key_id")->asInt();
    if (key_id == -1) {
        logger.printfln("Key ID is not set");
        return;
    }

    size_t cert_key_len = 0;
    auto cert_key = certs.get_cert(1, &cert_key_len);
    if (cert_key == nullptr) {
        logger.printfln("Certificate with ID 1 is not available");
        return;
    }

    // Get subject key id from certificate
    mbedtls_x509_crt x509_crt;
    mbedtls_x509_crt_init(&x509_crt);
    int mbedtls_ret = mbedtls_x509_crt_parse(&x509_crt, (const unsigned char *)cert_crt.get(), cert_crt_len + 1);
    if (mbedtls_ret != 0) {
        logger.printfln("Failed to parse certificate: %d", mbedtls_ret);
        return;
    }

    char ship_ski[64] = {0};
    for (size_t i = 0; i < x509_crt.subject_key_id.len; i++) {
        sprintf(&ship_ski[i * 2], "%02x", x509_crt.subject_key_id.p[i]);
    }
    eebus.state.get("ski")->updateString(ship_ski);
#endif

    // HTTPS server configuration.
    // This HTTPS server is just used to provide the send/recv for a secure websocket.
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();

    // HTTPD config
    // config.httpd.stack_size           = TODO;
    // config.httpd.max_uri_handlers     = TODO;
    config.httpd.lru_purge_enable = true;
    config.httpd.global_user_ctx = this;
    // httpd_stop calls free on the pointer passed as global_user_ctx if we don't override the free_fn.
    config.httpd.global_user_ctx_free_fn = [](void *foo) {};
    config.httpd.max_open_sockets = 3;
    config.httpd.enable_so_linger = true;
    config.httpd.linger_timeout = 100;
    // TODO: We could implement a mechanism that makes sure the ctrl ports are unique.
    //       By default, the ctrl port is 32768. If we have multiple instances of the
    //       httpd server running, we need to increment the ctrl port each time.
    config.httpd.ctrl_port = 32769;

    // SSL config
    config.transport_mode = HTTPD_SSL_TRANSPORT_SECURE;
    config.port_secure = 4712;
    config.port_insecure = 0;

#ifdef SHIP_USE_INTERNAL_CERTS
    config.servercert = (uint8_t *)ship_crt;
    config.servercert_len = strlen(ship_crt) + 1; // +1 since the length must include the null terminator
    config.prvtkey_pem = (uint8_t *)ship_key;
    config.prvtkey_len = strlen(ship_key) + 1; // +1 since the length must include the null terminator
#else
    config.servercert = cert_crt.release();
    config.servercert_len = cert_crt_len + 1; // +1 since the length must include the null terminator
    config.prvtkey_pem = cert_key.release();
    config.prvtkey_len = cert_key_len + 1; // +1 since the length must include the null terminator
#endif

    // Start HTTPS server
    httpd_handle_t httpd = nullptr;
    esp_err_t ret = httpd_ssl_start(&httpd, &config);
    if (ESP_OK != ret) {
        logger.printfln("Error starting server: %d", ret);
    }

    web_sockets.onConnect_HTTPThread([this](WebSocketsClient ws_client) {
        ship_connections.push_back(ShipConnection{ws_client, ShipConnection::Role::Server});
        logger.printfln("WebSocketsClient connected");
    });

    web_sockets.onBinaryDataReceived_HTTPThread([this](const int fd, httpd_ws_frame_t *ws_pkt) {
        for (auto &ship_connection : ship_connections) {
            if (ship_connection.ws_client.fd == fd) {
                ship_connection.frame_received(ws_pkt);
                return;
            }
        }

        logger.printfln("No ShipConnection found for fd %d", fd);
    });

    // Start websocket on the HTTPS server
    web_sockets.start("/ship/", "info/ship_wss", httpd, "ship");

    logger.printfln("setup_wss_server done");
}



void Ship::setup_mdns()
{
    logger.printfln("setup_mdns start");

    logger.printfln("mdns_service_add");
    // SHIP 7.2 Service Name
    mdns_service_add(NULL, "_ship", "_tcp", 4712, NULL, 0);

    // SHIP 7.3.2 TXT Record
    // Mandatory Fields
    logger.printfln("mdns_service_txt_item_set");
    mdns_service_txt_item_set("_ship", "_tcp", "txtvers", "1");
    // TODO: Use UID instead of 12345
    mdns_service_txt_item_set("_ship", "_tcp", "id", "Tinkerforge-WARP3-12345"); // ManufaturerName-Model-UniqueID (max 63 bytes)
    mdns_service_txt_item_set("_ship", "_tcp", "path", "/ship/");
    mdns_service_txt_item_set("_ship",
                              "_tcp",
                              "ski",
                              eebus.state.get("ski")->asEphemeralCStr()); // 40 byte hexadecimal digits representing the 160 bit SKI value
    mdns_service_txt_item_set("_ship", "_tcp", "register", "false");
    // Optional Fields
    mdns_service_txt_item_set("_ship", "_tcp", "brand", "Tinkerforge");
    mdns_service_txt_item_set("_ship", "_tcp", "model", "WARP3");
    mdns_service_txt_item_set("_ship", "_tcp", "type", "Wallbox"); // Or EVSE?

    logger.printfln("setup_mdns done");
}

void Ship::scan_skis()
{
    logger.printfln("discover_mdns start");
    const char * service = "_ship";
    const char * proto = "_tcp";
    mdns_result_t * results = NULL;
    esp_err_t err = mdns_query_ptr(service, proto, 3000, 20,  &results);
    if(err){
        logger.printfln("EEBUS MDNS Query Failed");
        return;
    }
    if(!results){
        logger.printfln("EEBUS MDNS: No results found!");
        return;
    }

    while (results) {
        mdns_results.push_back(*results);
        results = results->next;
    }

    mdns_query_results_free(results);

}

void Ship::print_skis(StringBuilder *sb)
{
    for (uint16_t i=0; i<mdns_results.size(); i++) {
        sb->printf("SKI: %s\n", mdns_results[i].hostname); // TODO: Add more information
    }
}

void Ship::remove(const ShipConnection &ship_connection)
{
    ship_connections.erase(std::remove(ship_connections.begin(), ship_connections.end(), ship_connection), ship_connections.end());
}
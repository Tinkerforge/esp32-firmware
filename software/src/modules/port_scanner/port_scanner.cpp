/* esp32-firmware
 * Copyright (C) 2026 Matthias Bolte <matthias@tinkerforge.com>
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

#include "port_scanner.h"

#include <esp_random.h>
#include <TFJson.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "build.h"
#include "tools/printf.h"

#include "options.h"
#include "gcc_warnings.h"

void PortScanner::pre_setup()
{
    scan_start_config = ConfigRoot{Config::Object({
        {"ip4_first", Config::Str("", 0, 15)},
        {"ip4_last", Config::Str("", 0, 15)},
        {"port", Config::Uint16(502)},
        {"cookie", Config::Uint32(0)},
    })};

    scan_keep_alive_config = ConfigRoot{Config::Object({
        {"cookie", Config::Uint32(0)},
    })};

    scan_abort_config = scan_keep_alive_config;
}

void PortScanner::register_urls()
{
    api.addCommand("port_scanner/scan_start", &scan_start_config, {}, [this](Language language, String &errmsg) {
        if (scan != nullptr) {
            errmsg = language == Language::English
                     ? "Another search is already in progress, please try again later!"
                     : "Eine andere Suche läuft bereits, bitte später noch einmal versuchen!";
            return;
        }

        scan = new_psram_or_dram<Scan>();

        scan->language = language;
        scan->port = scan_start_config.get("port")->asUint16();
        scan->cookie = scan_start_config.get("cookie")->asUint();

        if (!tf_ip4addr_aton(scan_start_config.get("ip4_first")->asString().c_str(), &scan->ip4_first)) {
            errmsg = language == Language::English
                     ? "The first IP address is invalid"
                     : "Die erste IP-Adresse ist ungültig";
            return;
        }

        if (!tf_ip4addr_aton(scan_start_config.get("ip4_last")->asString().c_str(), &scan->ip4_last)) {
            errmsg = language == Language::English
                     ? "The letzte IP address is invalid"
                     : "Die letzte IP-Adresse ist ungültig";
            return;
        }

        if (scan->ip4_last.addr < scan->ip4_first.addr) {
            scan->ip4_last = scan->ip4_first;
        }

        scan->ip4 = scan->ip4_first;

        tf_ip4addr_ntoa(&scan->ip4, scan->ip4_str, sizeof(scan->ip4_str));

        scan->last_keep_alive = now_us();

        scan_printfln(scan->language == Language::English
                      ? "Starting search [%s, version: %s]"
                      : "Beginne Suche [%s, version: %s]",
                      OPTIONS_PRODUCT_NAME(), build_version_full_str());
    }, true);

    api.addCommand("port_scanner/scan_keep_alive", &scan_keep_alive_config, {}, [this](Language language, String &errmsg) {
        if (scan == nullptr) {
            return;
        }

        uint32_t cookie = scan_keep_alive_config.get("cookie")->asUint();

        if (cookie != scan->cookie) {
            errmsg = language == Language::English ? "Cannot keep another search alive" : "Kann keine andere Suche aufrechterhalten";
            return;
        }

        scan->language        = language;
        scan->last_keep_alive = now_us();
    }, true);

    api.addCommand("port_scanner/scan_abort", &scan_abort_config, {}, [this](Language language, String &errmsg) {
        if (scan == nullptr) {
            return;
        }

        uint32_t cookie = scan_abort_config.get("cookie")->asUint();

        if (cookie != scan->cookie) {
            errmsg = language == Language::English ? "Cannot abort another scan" : "Kann keine andere Suche abbrechen";
            return;
        }

        scan->language = language;
        scan->abort    = true;
    }, true);
}

void PortScanner::loop()
{
    if (scan == nullptr) {
        return;
    }

    if (scan->printfln_buffer_used > 0 && deadline_elapsed(scan->printfln_last_flush + 500_ms)) {
        scan_flush_log();
    }

    if (!scan->abort && deadline_elapsed(scan->last_keep_alive + 10_s)) {
        const char *message = scan->language == Language::English
                              ? "Aborting search because no keep-alive call was received for more than 10 seconds"
                              : "Breche Suche ab, da für mehr als 10 Sekunden kein Aufrechterhalten-Aufruf empfangen wurde";

        logger.printfln("%s", message);
        scan_printfln("%s", message);

        scan->abort = true;
    }

    switch (scan->state) {
    case ScanState::Connect: {
            if (scan->abort) {
                scan->state = ScanState::Done;
                break;
            }

            scan_printfln(scan->language == Language::English
                          ? "Connecting to %s:%u"
                          : "Verbinde zu %s:%u",
                          scan->ip4_str, scan->port);

            close(scan->socket_fd);
            scan->socket_fd = socket(AF_INET, SOCK_STREAM, 0);

            if (scan->socket_fd < 0) {
                scan_printfln(scan->language == Language::English
                              ? "Error: Creating socket failed: %s (%d)"
                              : "Fehler: Erstellen des Sockets fehlgeschlagen: %s (%d)", strerror(errno), errno);

                scan->state = ScanState::Done;
                break;
            }

            int flags = fcntl(scan->socket_fd, F_GETFL, 0);

            if (flags < 0) {
                scan_printfln(scan->language == Language::English
                              ? "Error: Getting socket flags failed: %s (%d)"
                              : "Fehler: Abfragen der Socket Flags fehlgeschlagen: %s (%d)", strerror(errno), errno);

                scan->state = ScanState::Done;
                break;
            }

            if (fcntl(scan->socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
                scan_printfln(scan->language == Language::English
                              ? "Error: Setting socket flags failed: %s (%d)"
                              : "Fehler: Setzen der Socket Flags fehlgeschlagen: %s (%d)",
                              strerror(errno), errno);

                scan->state = ScanState::Done;
                break;
            }

            struct sockaddr_in addr_in;

            memset(&addr_in, 0, sizeof(addr_in));

            addr_in.sin_addr.s_addr = scan->ip4.addr;
            addr_in.sin_family = AF_INET;
            addr_in.sin_port = htons(scan->port);

            if (connect(scan->socket_fd, reinterpret_cast<struct sockaddr *>(&addr_in), sizeof(addr_in)) < 0 && errno != EINPROGRESS) {
                scan_printfln(scan->language == Language::English
                              ? "Connection attempt failed synchronous: %s (%d)"
                              : "Verbindungsversuch synchron fehlgeschlagen: %s (%d)",
                              strerror(errno), errno);

                scan->state = ScanState::Next;
                break;
            }

            scan->connect_deadline = calculate_deadline(3_s);
            scan->state = ScanState::Connecting;
        }

        break;

    case ScanState::Connecting: {
            if (scan->abort) {
                scan->state = ScanState::Done;
                break;
            }

            if (deadline_elapsed(scan->connect_deadline)) {
                scan_printfln(scan->language == Language::English
                              ? "Connection attempt timed out"
                              : "Zeitüberschreitung bei Verbindungsversuch");

                scan->state = ScanState::Next;
                break;
            }

            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(scan->socket_fd, &fdset);

            struct timeval tv;
            tv.tv_sec  = 0;
            tv.tv_usec = 0;

            int result = select(scan->socket_fd + 1, nullptr, &fdset, nullptr, &tv);

            if (result < 0) {
                scan_printfln(scan->language == Language::English
                              ? "Error: Socket select failed: %s (%d)"
                              : "Fehler: Socket Select fehlgeschlagen: %s (%d)", strerror(errno), errno);

                scan->state = ScanState::Done;
                break;
            }

            if (result == 0) {
                break; // connect() in progress
            }

            if (!FD_ISSET(scan->socket_fd, &fdset)) {
                break; // connect() in progress
            }

            int socket_errno;
            socklen_t socket_errno_length = sizeof(socket_errno);

            if (getsockopt(scan->socket_fd, SOL_SOCKET, SO_ERROR, &socket_errno, &socket_errno_length) < 0) {
                scan_printfln(scan->language == Language::English
                              ? "Error: Getting socket error failed: %s (%d)"
                              : "Fehler: Abfragen des Socket-Fehlers fehlgeschlagen: %s (%d)",
                              strerror(errno), errno);

                scan->state = ScanState::Done;
                break;
            }

            if (socket_errno == ECONNRESET) {
                scan_printfln(scan->language == Language::English
                              ? "Connection attempt was refused"
                              : "Verbindungsversuch wurde abgelehnt");

                scan->state = ScanState::PortClosed;
                break;
            }

            if (socket_errno != 0) {
                scan_printfln(scan->language == Language::English
                              ? "Connection attempt failed asynchronous: %s (%d)"
                              : "Verbindungsversuch asynchron fehlgeschlagen: %s (%d)",
                              strerror(socket_errno), socket_errno);

                scan->state = ScanState::Next;
                break;
            }

            scan->state = ScanState::PortOpen;
        }

        break;

    case ScanState::PortOpen: {
            if (scan->abort) {
                scan->state = ScanState::Done;
                break;
            }

            scan->connected = true;

            scan_printfln(scan->language == Language::English
                          ? "Connected to %s:%u"
                          : "Verbunden zu %s:%u",
                          scan->ip4_str, scan->port);

#if MODULE_WS_AVAILABLE()
            char buf[512];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", scan->cookie);
            json.addMemberString("ip4", scan->ip4_str);
            json.addMemberNumber("port", scan->port);
            json.addMemberBoolean("open", true);
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "port_scanner/scan_result")) {
                break; // need to report the scan result before doing something else
            }
#endif

            scan->state = ScanState::Next;
        }

        break;

    case ScanState::PortClosed: {
            if (scan->abort) {
                scan->state = ScanState::Done;
                break;
            }

#if MODULE_WS_AVAILABLE()
            char buf[512];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", scan->cookie);
            json.addMemberString("ip4", scan->ip4_str);
            json.addMemberNumber("port", scan->port);
            json.addMemberBoolean("open", false);
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "port_scanner/scan_result")) {
                break; // need to report the scan result before doing something else
            }
#endif

            scan->state = ScanState::Next;
        }

        break;

    case ScanState::Next:
        if (scan->abort) {
            scan->state = ScanState::Done;
            break;
        }

        if (scan->socket_fd >= 0) {
            if (scan->connected) {
                scan->connected = false;

                scan_printfln(scan->language == Language::English
                              ? "Disconnecting from %s:%u"
                              : "Trenne Verbindung zu %s:%u",
                              scan->ip4_str, scan->port);
            }

            close(scan->socket_fd);
            scan->socket_fd = -1;
        }

        if (scan->ip4.addr >= scan->ip4_last.addr) {
            scan->state = ScanState::Done;
            break;
        }

#if MODULE_WS_AVAILABLE()
        {
            char buf[128];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", scan->cookie);
            json.addMemberNumber("progress", static_cast<float>(scan->ip4.addr + 1u - scan->ip4_first.addr) * 100.0f / static_cast<float>(scan->ip4_last.addr - scan->ip4_first.addr));
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "port_scanner/scan_progress")) {
                break; // need to report scan progress before doing something else
            }
        }
#endif

        scan->ip4.addr = htonl(ntohl(scan->ip4.addr) + 1);
        tf_ip4addr_ntoa(&scan->ip4, scan->ip4_str, sizeof(scan->ip4_str));

        scan->state = ScanState::Connect;
        break;

    case ScanState::Done: {
            if (scan->socket_fd >= 0) {
                if (scan->connected) {
                    scan->connected = false;

                    scan_printfln(scan->language == Language::English
                                  ? "Disconnecting from %s:%u"
                                  : "Trenne Verbindung zu %s:%u",
                                  scan->ip4_str, scan->port);
                }

                close(scan->socket_fd);
                scan->socket_fd = -1;
            }

            scan_printfln(scan->language == Language::English
                          ? (scan->abort ? "Search aborted" : "Search finished")
                          : (scan->abort ? "Suche abgebrochen" : "Suche abgeschlossen"));
            scan_flush_log();

#if MODULE_WS_AVAILABLE()
            char buf[128];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", scan->cookie);
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "port_scanner/scan_done")) {
                break; // need to report the scan as done before doing something else
            }
#endif

            delete_psram_or_dram(scan);
            scan = nullptr;
        }

        break;

    default:
        esp_system_abortf<48>("Invalid state during scan: %d", static_cast<int>(scan->state));
    }
}

void PortScanner::scan_flush_log()
{
    if (scan == nullptr) {
        return;
    }

#if MODULE_WS_AVAILABLE()
    char buf[1024];
    TFJsonSerializer json{buf, sizeof(buf)};

    json.addObject();
    json.addMemberNumber("cookie", scan->cookie);
    json.addMemberString("message", scan->printfln_buffer);
    json.endObject();
    json.end();
#endif

    scan->printfln_buffer_used = 0;
    scan->printfln_last_flush = now_us();

#if MODULE_WS_AVAILABLE()
    ws.pushRawStateUpdate(buf, "port_scanner/scan_log"); // FIXME: error handling
#endif
}

void PortScanner::scan_printfln(const char *fmt, ...)
{
    if (scan == nullptr) {
        return;
    }

    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);
    size_t used = vsnprintf_u(nullptr, 0, fmt, args_copy);
    va_end(args_copy);

    if (scan->printfln_buffer_used + used + 1 /* for \n */ >= sizeof(scan->printfln_buffer)) {
        scan_flush_log();
    }

    scan->printfln_buffer_used += vsnprintf_u(scan->printfln_buffer + scan->printfln_buffer_used, sizeof(scan->printfln_buffer) - scan->printfln_buffer_used, fmt, args);
    va_end(args);

    scan->printfln_buffer[scan->printfln_buffer_used++] = '\n';
    scan->printfln_buffer[scan->printfln_buffer_used] = '\0';
}

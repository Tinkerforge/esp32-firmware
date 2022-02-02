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

#include "login_page.h"
#include "web_server.h"
#include "build.h"

#include "login.html.h"

extern WebServer server;

LoginPage::LoginPage()
{

}

void LoginPage::setup()
{

}

void LoginPage::register_urls()
{
    server.onNotAuthorized([](WebServerRequest request) {
        if (request.uri() == "/") {
            // Safari does not support an unauthenticated login page and an authenticated main page on the same url,
            // as it does not proactively send the credentials if the same url is known to have an unauthenticated
            // version.
            const String &user_agent = request.header("User-Agent");
            bool is_safari = user_agent.indexOf("Safari/") >= 0 && user_agent.indexOf("Version/") >= 0 && user_agent.indexOf("Chrome/") == -1 && user_agent.indexOf("Chromium/") == -1;
            if (is_safari) {
                request.requestAuthentication();
                return;
            }

            request.addResponseHeader("Content-Encoding", "gzip");
            request.addResponseHeader("ETag", BUILD_TIMESTAMP_HEX_STR);
            request.send(200, "text/html", login_html_gz, login_html_gz_len);
        } else if (request.uri() == "/login_state") {
            // Same reasoning as above. If we don't force Safari, it does not send credentials, which breaks the login_state check.
            const String &user_agent = request.header("User-Agent");
            bool is_safari = user_agent.indexOf("Safari/") >= 0 && user_agent.indexOf("Version/") >= 0 && user_agent.indexOf("Chrome/") == -1 && user_agent.indexOf("Chromium/") == -1;
            if (is_safari) {
                request.requestAuthentication();
                return;
            }

            request.send(200, "text/plain", "Not logged in");
        } else {
            request.requestAuthentication();
            return;
        }
    });

    server.on("/credential_check", HTTP_GET, [](WebServerRequest request) {
        request.send(200, "text/plain", "Credentials okay");
    });

    server.on("/login_state", HTTP_GET, [](WebServerRequest request) {
        request.send(200, "text/plain", "Logged in");
    });
}

void LoginPage::loop()
{

}

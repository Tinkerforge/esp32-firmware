#include "authentication.h"

#include "Arduino.h"

#include "api.h"
#include "event_log.h"
#include "web_server.h"

#include "login.html.h"

extern WebServer server;
extern API api;
extern EventLog logger;

Authentication::Authentication()
{
    authentication_config = Config::Object({
        {"enable_auth", Config::Bool(false)},
        {"username", Config::Str("", 64)},
        {"password", Config::Str("", 64)},
    }, [](Config::ConfObject &update) {
        if (update.get("enable_auth")->asBool() && update.get("password")->asString() == "")
            return String("Authentication can not be enabled if no password is set.");

        if (update.get("enable_auth")->asBool() && update.get("username")->asString() == "")
            return String("Authentication can not be enabled if no username is set.");

        if (!update.get("enable_auth")->asBool() && update.get("password")->asString() != "")
            update.get("password")->updateString("");

        return String("");
    });
}

void Authentication::setup()
{
    api.restorePersistentConfig("authentication/config", &authentication_config);

    if (authentication_config.get("enable_auth")->asBool()) {
        String user = authentication_config.get("username")->asString();
        String pass = authentication_config.get("password")->asString();

        server.setAuthentication(user.c_str(), pass.c_str());
        logger.printfln("Web interface authentication enabled.");
    }

    initialized = true;
}

void Authentication::register_urls()
{
    api.addPersistentConfig("authentication/config", &authentication_config, {"password"}, 10000);

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
            request.addResponseHeader("ETag", String((uint32_t)(_BUILD_TIME_), 16).c_str());
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

void Authentication::loop()
{

}

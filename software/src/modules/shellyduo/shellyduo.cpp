/* warp-charger
 * Copyright (C)      2021 Birger Schmidt <bs-warp@netgaroo.com>
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

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"
#include "web_server.h"
#include "modules.h"

#include <AsyncTCP.h>

#include "shellyduo.h"
#include "event_log.h"
extern EventLog logger;

String _uri = "";
bool initialized = false;
int _intensity = 10; //percent
String _host = "";

unsigned long target_time = 0L;
const unsigned long PERIOD = 15*1000UL; //miliseconds

AsyncClient *client_tcp = new AsyncClient;

extern EventLog logger;

extern TaskScheduler task_scheduler;

extern API api;

void replyToServer(void *arg)
{
    AsyncClient *client = reinterpret_cast<AsyncClient *>(arg);
    // prepare the http get
    String payload = 
        String("GET ") + 
        // set the light to whatever status shall be displayed
        _uri + 
        // add the turn off timeout as a watchdog that ensures there is no stale status
        "&timer=22 HTTP/1.1\n" +
        // add the host header
        "Host: " + _host + "\n\n";
    // send reply
    if (client->space() > strlen(payload.c_str()) && client->canSend())
    {
        client->add(payload.c_str(), strlen(payload.c_str()));
        client->send();
    }
}

void handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
    /* char reply[len + 1]; */
    /* memcpy(reply, data, len); */
    /* reply[len] = 0; // Null termination. */

    //logger.printfln("<%s: %s", client->remoteIP().toString().c_str(), reply);
    //Serial.write((uint8_t *)data, len);
}

void onConnect(void *arg, AsyncClient *client)
{
    replyToServer(client);
}


ShellyDuo::ShellyDuo()
{
}

void ShellyDuo::pre_setup()
{
    shellyduo_prepare_command = Config::Object({
        {"uri", Config::Str("",500)}
    });
    shellyduo_color = Config::Object({
        {"color", Config::Uint(0xFFFFFF)}
    });

    shellyduo_config = ConfigRoot(Config::Object({
        {"enable", Config::Bool(false)},
        {"host", Config::Str("", 0, 128)},
        {"intensity", Config::Uint8(10)},
    }), [](Config &cfg) -> String {
        return "";
    });
}

void ShellyDuo::setup()
{
    client_tcp->onData(handleData, client_tcp);
    client_tcp->onConnect(onConnect, client_tcp);

    if(!api.restorePersistentConfig("shellyduo/config", &shellyduo_config)) {
         logger.printfln("Status light error, could not restore persistent config, defaults to disabled.");
    }
    initialized = true;

    logger.printfln("Statuslight initialized. Host: %s, %s", shellyduo_config.get("host")->asEphemeralCStr(), shellyduo_config.get("enable")->asBool() ? "enabled" : "disabled");

    task_scheduler.scheduleWithFixedDelay([this](){
        this->update();
    }, 0, 5000);
}

void ShellyDuo::register_urls()
{
    api.addCommand("statuslight/prepare_command", &shellyduo_prepare_command, {}, [this](){
        this->prepareCommand(shellyduo_prepare_command.get("uri")->asString());
    }, true);
    api.addCommand("statuslight/color", &shellyduo_color, {}, [this](){
        this->color(shellyduo_color.get("color")->asUint());
    }, true);
    api.addCommand("statuslight/update", Config::Null(), {}, [this](){
        this->update();
    }, true);
    api.addPersistentConfig("shellyduo/config", &shellyduo_config, {}, 1000);
}

void ShellyDuo::loop()
{
}

void ShellyDuo::sendCommand() {
    if (initialized) {
        shellyduo_config.get("host")->asString() + "\n\n";
        client_tcp->connect(shellyduo_config.get("host")->asString().c_str(), 80);
        client_tcp->close();
    }
}

void ShellyDuo::prepareCommand(String uri)
{
    if (_uri != uri) {
       _uri = uri;
       shellyduo_prepare_command.get("uri")->updateString(uri);
       logger.printfln("%s", _uri.c_str());
    }
}

void ShellyDuo::color(int color)
{
    uint8_t red   = (color>>16)&0x0ff;
    uint8_t green = (color>>8) &0x0ff;
    uint8_t blue  = (color)    &0x0ff;
    shellyduo_color.get("color")->updateUint(color);
    this->prepareCommand(String("/color/0?turn=on&red=" + String(red) + "&green=" + String(green) + "&blue=" + String(blue) + "&effect=0&transition=3000&gain=10"));
}

void ShellyDuo::update()
{
    // TODO do not hammer the shellyduo with commands, because it will ignore us if the threshold is reached

    /* NOTE: Currently WiFi smart device support up to 4 simultaneous TCP connections, any further */
    /* connect attempt will be rejected. For each connection, there is a command message quota, */
    /* that is 60 commands per minute. There is also a total quota for all the LAN commands: 144 */
    /* commands per minute 4 × 60 × 60%. */

    if(shellyduo_config.get("enable")->asBool()){
        _host = shellyduo_config.get("host")->asString();
        // To decide what status needs to be shown, we hope to get a better source of truth. See: https://github.com/warp-more-hardware/esp32-firmware/issues/19
        switch(charge_manager.charge_manager_state.get("state")->asUint()) {
            case 0: // not configured
                this->color(0xAAAAAA); // grey
                break;
            case 1: // active
                this->color(0x00FF00); // green
                break;
            case 2: // shutdown
                this->color(0xFF00FF); // pink
                break;
        }
        this->sendCommand();
    }
}

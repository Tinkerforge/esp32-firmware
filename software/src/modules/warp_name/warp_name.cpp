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
#include "warp_name.h"

#include "build.h"
#include "modules.h"
#include "task_scheduler.h"

extern char local_uid_str[7];
extern TaskScheduler task_scheduler;

WARPName::WARPName()
{
    name = Config::Object({
        {"name", Config::Str("")},
        {"type", Config::Str(BUILD_HOST_PREFIX)},
        {"display_type", Config::Str("", 0, 32)},
        {"uid", Config::Str("", 0, 32)}
    });

    display_name = Config::Object({
        {"display_name", Config::Str("", 0, 32)}
    });
}

void WARPName::updateDisplayType() {
    String display_type = "WARP";
    if (api.getState("evse/hardware_configuration")->get("evse_version")->asUint() >= 20)
        display_type += "2";
    display_type += " Charger ";

    display_type += api.hasFeature("meter") ? "Pro " : "Smart ";

    display_type += api.getState("evse/slots")->get(1)->get("max_current")->asUint() <= 20000 ? "11" : "22";
    display_type += "kW";

    if (api.hasFeature("nfc"))
        display_type += " +NFC";

    if (api.hasFeature("rtc"))
        display_type += " +RTC";

    name.get("display_type")->updateString(display_type);
}

void WARPName::setup()
{
    name.get("name")->updateString(String(BUILD_HOST_PREFIX) + "-" +local_uid_str);
    name.get("uid")->updateString(String(local_uid_str));

    if(!api.restorePersistentConfig("info/display_name", &display_name)) {
        display_name.get("display_name")->updateString(name.get("name")->asString());
    }

    task_scheduler.scheduleWithFixedDelay([this](){
        this->updateDisplayType();
    }, 0, 60000);

    initialized = true;
}

void WARPName::register_urls()
{
   api.addState("info/name", &name, {}, 1000);
   api.addPersistentConfig("info/display_name", &display_name, {}, 1000);
}

void WARPName::loop()
{

}

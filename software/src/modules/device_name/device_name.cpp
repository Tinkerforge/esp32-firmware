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
#include "device_name.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"

extern char local_uid_str[32];

void DeviceName::pre_setup()
{
    name = Config::Object({
        {"name", Config::Str("", 0, BUILD_HOST_PREFIX_LENGTH + 1 + ARRAY_SIZE(local_uid_str))},
        {"type", Config::Str(BUILD_HOST_PREFIX, 0, BUILD_HOST_PREFIX_LENGTH)},
        {"display_type", Config::Str("", 0, 64)},
        {"uid", Config::Str("", 0, 32)}
    });

    display_name = Config::Object({
        {"display_name", Config::Str("", 0, 32)}
    });
}

#if BUILD_IS_WARP() || BUILD_IS_WARP2() || BUILD_IS_WARP3()
String getWarpDisplayName()
{
    String display_type = api.hasFeature("meter") ? " Pro" : " Smart";

    if (api.hasFeature("evse")) {
        display_type += api.getState("evse/slots")->get(1)->get("max_current")->asUint() <= 20000 ? " 11" : " 22";
        display_type += "kW";
    } else {
        display_type += " without EVSE";
    }

#if BUILD_IS_WARP()
    if (api.hasFeature("nfc")) {
        display_type += " +NFC";
    }
#endif

#if BUILD_IS_WARP() || BUILD_IS_WARP2()
    if (api.hasFeature("rtc")) {
        display_type += " +RTC";
    }
#endif
    return display_type;
}
#endif

static bool isVowel(char c)
{
    return (0x208222 >> (c & 0x1f)) & 1;
}

void DeviceName::updateDisplayType()
{
    String display_type = BUILD_DISPLAY_NAME;
#if BUILD_IS_WARP() || BUILD_IS_WARP2() || BUILD_IS_WARP3()
    display_type += getWarpDisplayName(); // FIXME: Also add more details for WARP Energy Manager, similar to WARP[2] here?
#endif

    const char *indef_article = isVowel(display_type[0]) ? "an" : "a";

    if (name.get("display_type")->updateString(display_type)) {
        logger.printfln("This is %s (%s), %s %s", display_name.get("display_name")->asEphemeralCStr(), name.get("name")->asEphemeralCStr(), indef_article, display_type.c_str());
    }
}

void DeviceName::setup()
{
    name.get("name")->updateString(String(BUILD_HOST_PREFIX) + "-" + local_uid_str);
    name.get("uid")->updateString(String(local_uid_str));

    // We intentionally don't use the display_name_in_use = display_name construction here:
    // We want to be able to change the display_name without a reboot, because in the web interface
    // we don't use the usual save + reboot modal code-path.
    if (!api.restorePersistentConfig("info/display_name", &display_name)) {
        display_name.get("display_name")->updateString(name.get("name")->asString());
    }

    task_scheduler.scheduleWithFixedDelay([this](){
        this->updateDisplayType();
    }, 0, 60000);

    initialized = true;
}

void DeviceName::register_urls()
{
    api.addState("info/name", &name);
    api.addPersistentConfig("info/display_name", &display_name);
}

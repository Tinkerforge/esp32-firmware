/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "config.h"

#include "module.h"

class IRtcBackend
{
public:
    IRtcBackend() {}
    virtual ~IRtcBackend() {}

    virtual void set_time(const timeval &time) = 0;
    virtual void set_time(const tm &time) = 0;
    virtual struct timeval get_time() = 0;
    virtual void update_system_time() = 0;
    virtual void reset() = 0;
};

class Rtc final : public IModule
{
private:
    ConfigRoot time;
    ConfigRoot time_update;
    ConfigRoot rtc_config;

    IRtcBackend *backend = NULL;

public:
    Rtc() {}

    void pre_setup();
    void setup();

    void register_backend(IRtcBackend *_backend);

    void reset();

    void set_time(const timeval &_time);
    timeval get_time();
    void update_system_time();
};

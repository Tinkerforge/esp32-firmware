/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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

class IModule
{
public:
    IModule() = default;

    // Do not allow modules to be copied or assigned. Note: Use references or pointers instead.
    IModule(const IModule &other) = delete;
    IModule(IModule &&other) = delete;
    IModule& operator=(IModule &other) = delete;
    IModule& operator=(IModule &&other) = delete;

    // Cannot remove destructor because all modules have static storage duration.
    // From § 3.6.3 of the C++03 standard: Destructors (12.4) for initialized objects of static storage duration (declared at block scope
    // or at namespace scope) are called as a result of returning from main and as a result of calling exit (18.3).
    virtual ~IModule() = default;

    // pre_init() is only for special handlers that must run very early.
    // No config, HAL or data partition available.
    virtual void pre_init() {}

    virtual void pre_setup() {}
    virtual void setup() { initialized = true; }
    virtual void register_urls() {}
    virtual void register_events() {}
    virtual void loop() {}
    virtual void pre_reboot() {}

    bool initialized = false;
};

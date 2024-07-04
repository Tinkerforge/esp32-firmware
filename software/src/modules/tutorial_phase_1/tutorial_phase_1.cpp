/* esp32-firmware
 * Copyright (C) 2022 Matthias Bolte <matthias@tinkerforge.com>
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

#include "tutorial_phase_1.h"

#include <Arduino.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"

void TutorialPhase1::pre_setup()
{
}

void TutorialPhase1::setup()
{
    logger.printfln("Tutorial (Phase 1) module initialized");

    initialized = true;
}

void TutorialPhase1::register_urls()
{
}

void TutorialPhase1::loop()
{
}

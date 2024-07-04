/* esp32-firmware
 * Copyright (C) 2020-2024 Erik Fleckstein <erik@tinkerforge.com>
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

#include "config/private.h"

Config::Wrap::Wrap(Config *_conf)
{
    conf = _conf;
}

Config * Config::Wrap::operator->() {
    return conf;
}

Config::Wrap::operator Config*(){return conf;}

std::vector<Config>::iterator Config::Wrap::begin() {return conf->begin();}
std::vector<Config>::iterator Config::Wrap::end() {return conf->end();}

Config::ConstWrap::ConstWrap(const Config *_conf)
{
    conf = _conf;
}

const Config * Config::ConstWrap::operator->() const {
    return conf;
}

Config::ConstWrap::operator const Config*() const {return conf;}

std::vector<Config>::const_iterator Config::ConstWrap::begin() const {return conf->begin();}
std::vector<Config>::const_iterator Config::ConstWrap::end() const {return conf->end();}

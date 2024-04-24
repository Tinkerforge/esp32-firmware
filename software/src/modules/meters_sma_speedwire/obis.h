/* esp32-firmware
 * Copyright (C) 2023 Thomas Hein
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

#include <Arduino.h>

class obis {
  public:
    obis(uint8_t kanal, uint8_t messgroesse, uint8_t messart, uint8_t tarifstufe, float faktor, uint8_t valueSize);

    float value(uint8_t buf[], size_t bufSize);

  private:
    bool compareObis(uint8_t buf[]) const;
    static uint32_t convert_uint32(uint8_t buf[]);
    static uint64_t convert_uint64(uint8_t buf[]);

  private:
    uint8_t _obis[4];
    float  _faktor;
    uint8_t _valueSize;
    size_t  _position;
};

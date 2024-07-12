/* esp32-firmware
 * Copyright (C) 2024 Erik Fleckstein <erik@tinkerforge.com>
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

#include <stdint.h>
#include <stddef.h>

#define PHASE_ROTATION(x, y, z) (0 << 6) | (((int)x << 4) | ((int)y << 2) | (int)z)

enum class ChargerPhase {
    PV,
    P1, // L1 if rotation is L123
    P2,
    P3
};

enum class GridPhase {
    PV,
    L1,
    L2,
    L3
};

enum class PhaseRotation {
    Unknown = 0, // Make unknown 0 so that memsetting the ChargerState struct sets this as expected.
    NotApplicable = 1, // Put this here so that enum >> 1 is the number of the first phase and enum & 1 is is the rotation
    L123 = PHASE_ROTATION(GridPhase::L1,GridPhase::L2,GridPhase::L3), // Standard Reference Phasing (= RST in OCPP)
    L132 = PHASE_ROTATION(GridPhase::L1,GridPhase::L3,GridPhase::L2), // Reversed Reference Phasing (= RTS in OCPP)
    L231 = PHASE_ROTATION(GridPhase::L2,GridPhase::L3,GridPhase::L1), // Standard 120 degree rotation (= STR in OCPP)
    L213 = PHASE_ROTATION(GridPhase::L2,GridPhase::L1,GridPhase::L3), // Reversed 240 degree rotation (= SRT in OCPP)
    L321 = PHASE_ROTATION(GridPhase::L3,GridPhase::L2,GridPhase::L1), // Reversed 120 degree rotation (= TSR in OCPP)
    L312 = PHASE_ROTATION(GridPhase::L3,GridPhase::L1,GridPhase::L2), // Standard 240 degree rotation (= TRS in OCPP)
};

struct Cost {
    int pv = 0;
    int l1 = 0;
    int l2 = 0;
    int l3 = 0;

    Cost () : pv(0), l1(0), l2(0), l3(0) {}
    Cost (int pv, int l1, int l2, int l3) : pv(pv), l1(l1), l2(l2), l3(l3) {}

    int &operator[](size_t idx) { return *(&pv + idx); }
    const int &operator[](size_t idx) const { return *(&pv + idx); }

    int &operator[](GridPhase p) { return *(&pv + (int)p); }
    const int &operator[](GridPhase p) const { return *(&pv + (int)p); }

    // Looks strange but seems to be best practice:
    // https://en.cppreference.com/w/cpp/language/operators#Binary_arithmetic_operators
    Cost &operator+=(const Cost& rhs) {
        this->pv += rhs.pv;
        this->l1 += rhs.l1;
        this->l2 += rhs.l2;
        this->l3 += rhs.l3;

        return *this;
    }

    friend Cost operator+(Cost lhs, const Cost &rhs) {
        lhs += rhs;
        return lhs;
    }

    Cost &operator-=(const Cost& rhs) {
        this->pv -= rhs.pv;
        this->l1 -= rhs.l1;
        this->l2 -= rhs.l2;
        this->l3 -= rhs.l3;

        return *this;
    }

    friend Cost operator-(Cost lhs, const Cost &rhs) {
        lhs -= rhs;
        return lhs;
    }
};
static_assert(sizeof(Cost) == 4 * sizeof(int), "Unexpected size of Cost");

struct CurrentLimits {
    Cost raw;
    Cost min;
    Cost spread;
    int max_pv;
};

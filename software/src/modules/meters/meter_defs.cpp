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

#include "meter_defs.h"

const char * const meter_all_values_names[METER_ALL_VALUES_LEGACY_COUNT] = {
    "line_to_neutral_volts_l1",
    "line_to_neutral_volts_l2",
    "line_to_neutral_volts_l3",
    "current_l1_a",
    "current_l2_a",
    "current_l3_a",
    "power_l1_w",
    "power_l2_w",
    "power_l3_w",
    "volt_amps_l1",
    "volt_amps_l2",
    "volt_amps_l3",
    "volt_amps_reactive_l1",
    "volt_amps_reactive_l2",
    "volt_amps_reactive_l3",
    "power_factor_l1",
    "power_factor_l2",
    "power_factor_l3",
    "phase_angle_l1_degree",
    "phase_angle_l2_degree",
    "phase_angle_l3_degree",
    "average_line_to_neutral_volts",
    "average_line_current_a",
    "sum_of_line_currents_a",
    "total_system_power_w",
    "total_system_volt_amps",
    "total_system_var",
    "total_system_power_factor",
    "total_system_phase_angle_degree",
    "frequency_of_supply_voltages_hertz",
    "total_import_kwh",
    "total_export_kwh",
    "total_import_kvarh",
    "total_export_kvarh",
    "total_vah",
    "ah",
    "total_system_power_demand_w",
    "maximum_total_system_power_demand_w",
    "total_system_va_demand",
    "maximum_total_system_va_demand",
    "neutral_current_demand_a",
    "maximum_neutral_current_demand_a",
    "line1_to_line2_volts",
    "line2_to_line3_volts",
    "line3_to_line1_volts",
    "average_line_to_line_volts",
    "neutral_current_a",
    "ln_volts_thd_l1",
    "ln_volts_thd_l2",
    "ln_volts_thd_l3",
    "current_thd_l1_a",
    "current_thd_l2_a",
    "current_thd_l3_a",
    "average_line_to_neutral_volts_thd",
    "average_line_current_thd_a",
    "current_demand_l1_a",
    "current_demand_l2_a",
    "current_demand_l3_a",
    "maximum_current_demand_l1_a",
    "maximum_current_demand_l2_a",
    "maximum_current_demand_l3_a",
    "line1_to_line2_volts_thd_percent",
    "line2_to_line3_volts_thd_percent",
    "line3_to_line1_volts_thd_percent",
    "average_line_to_line_volts_thd_percent",
    "total_kwh_sum",
    "total_kvarh_sum",
    "import_kwh_l1",
    "import_kwh_l2",
    "import_kwh_l3",
    "export_kwh_l1",
    "export_kwh_l2",
    "export_kwh_l3",
    "total_kwh_l1",
    "total_kwh_l2",
    "total_kwh_l3",
    "import_kvarh_l1",
    "import_kvarh_l2",
    "import_kvarh_l3",
    "export_kvarh_l1",
    "export_kvarh_l2",
    "export_kvarh_l3",
    "total_kvarh_l1",
    "total_kvarh_l2",
    "total_kvarh_l3",
};

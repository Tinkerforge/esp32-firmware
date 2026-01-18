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

import * as API from "../../ts/api";
import { __ } from "../../ts/translation";
import { register_status_provider, ModuleStatus } from "../../ts/status_registry";

export { EVSE, EVSENavbar } from "./evse_content";
export { EVSESettings, EVSESettingsNavbar } from "./evse_settings";
export { EVSEStatus } from "./evse_status";

export function pre_init() {
}

export function init() {
    register_status_provider("evse", {
        get_status: () => {
            const state = API.get("evse/state");

            switch (state?.charger_state) {
                case 0: // Not connected
                    return {
                        id: "evse",
                        name: () => __("evse.navbar.evse"),
                        status: ModuleStatus.Ok,
                        text: () => __("evse.status.not_connected"),
                        priority: 950,
                        href: "#evse"
                    };
                case 1: // Waiting for charge release
                    return {
                        id: "evse",
                        name: () => __("evse.navbar.evse"),
                        status: ModuleStatus.Warning, // Maybe also OK?
                        text: () => __("evse.status.waiting_for_charge_release"),
                        priority: 950,
                        href: "#evse"
                    };
                case 2: // Ready to charge
                    return {
                        id: "evse",
                        name: () => __("evse.navbar.evse"),
                        status: ModuleStatus.Ok,
                        text: () => __("evse.status.ready_to_charge"),
                        priority: 950,
                        href: "#evse"
                    };
                case 3: // Charging
                    return {
                        id: "evse",
                        name: () => __("evse.navbar.evse"),
                        status: ModuleStatus.Ok,
                        text: () => __("evse.status.charging"),
                        priority: 950,
                        href: "#evse"
                    };
                case 4: // Error
                    return {
                        id: "evse",
                        name: () => __("evse.navbar.evse"),
                        status: ModuleStatus.Error,
                        text: () => __("evse.status.error"),
                        priority: 950,
                        href: "#evse"
                    };
                default:
                    return {
                        id: "evse",
                        name: () => __("evse.navbar.evse"),
                        status: ModuleStatus.Ok,
                        priority: 950,
                        href: "#evse"
                    };
            }
        }
    });
}

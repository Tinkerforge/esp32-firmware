/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

import { ComponentChildren } from "preact";
import { STATUS_PROVIDER_ORDER } from "../options";

export const enum ModuleStatus {
    Disabled = 0,
    Ok = 1,
    Warning = 2,
    Error = 3
}

export interface StatusResult {
    status: ModuleStatus;
    text?: () => string; // Optional detail text (e.g. IP address, "Connecting...")
    icon?: () => ComponentChildren; // Optional icon (e.g. WiFi signal strength)
}

export interface ModuleStatusEntry {
    id: string;
    name: () => string;
    status: ModuleStatus;
    text?: () => string;
    icon?: () => ComponentChildren;
    order: number;
    href?: string;
}

export interface StatusProviderConfig {
    name: () => string;
    href?: string;
    get_status: () => StatusResult | null;
}

// Registry of all status providers
const status_providers: Map<string, StatusProviderConfig> = new Map();

export function register_status_provider(module_id: string, config: StatusProviderConfig): void {
    status_providers.set(module_id, config);
}

export function get_all_statuses(): ModuleStatusEntry[] {
    const entries: ModuleStatusEntry[] = [];

    for (const [id, config] of status_providers) {
        try {
            const result = config.get_status();
            if (result) {
                // Use index in STATUS_PROVIDER_ORDER for sorting
                // IDs not in list get a high number to appear at the end
                const order_index = STATUS_PROVIDER_ORDER.indexOf(id);
                const order = order_index >= 0 ? order_index : STATUS_PROVIDER_ORDER.length + 1000;

                entries.push({
                    id: id,
                    name: config.name,
                    order: order,
                    href: config.href,
                    ...result
                });
            }
        } catch (e) {
            console.error(`Error getting status from provider:`, e);
        }
    }

    // Separate enabled and disabled entries
    const enabled = entries.filter(e => e.status !== ModuleStatus.Disabled);
    const disabled = entries.filter(e => e.status === ModuleStatus.Disabled);

    // Sort by configured order (from custom_frontend_components in .ini)
    enabled.sort((a, b) => a.order - b.order);
    disabled.sort((a, b) => a.order - b.order);

    // Enabled entries first, then disabled at the bottom
    return [...enabled, ...disabled];
}

// Status for the status button itself
export function get_overall_status(entries: ModuleStatusEntry[]): ModuleStatus {
    if (entries.some(e => e.status === ModuleStatus.Error)) {
        return ModuleStatus.Error;
    } else if (entries.some(e => e.status === ModuleStatus.Warning)) {
        return ModuleStatus.Warning;
    }

    // If either OK is present or no active entries, return OK
    return ModuleStatus.Ok;
}

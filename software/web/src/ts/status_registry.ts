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

export const enum ModuleStatus {
    Disabled = 0,
    Ok = 1,
    Warning = 2,
    Error = 3
}

// Displayed in the header status dropdown
export interface ModuleStatusEntry {
    id: string; // Unique identifier
    name: () => string;
    status: ModuleStatus;
    text?: () => string; // Optional detail text function (e.g., IP address, "Connecting...")
    priority: number; // priority for sorting, 0 = low
    href?: string; // Optional link to configuration page (e.g., "#ethernet")
}

export interface StatusProviderConfig {
    get_status: () => ModuleStatusEntry | ModuleStatusEntry[] | null;
}

// Registry of all status providers
const status_providers: Map<string, StatusProviderConfig> = new Map();

export function register_status_provider(module_id: string, config: StatusProviderConfig): void {
    status_providers.set(module_id, config);
}

export function get_all_statuses(): ModuleStatusEntry[] {
    const entries: ModuleStatusEntry[] = [];

    for (const [_id, config] of status_providers) {
        try {
            const result = config.get_status();
            if (result) {
                if (Array.isArray(result)) {
                    entries.push(...result);
                } else {
                    entries.push(result);
                }
            }
        } catch (e) {
            console.error(`Error getting status from provider:`, e);
        }
    }

    // Separate enabled and disabled entries
    const enabled = entries.filter(e => e.status !== ModuleStatus.Disabled);
    const disabled = entries.filter(e => e.status === ModuleStatus.Disabled);

    // Sort by priority descending (higher priority = shown first)
    enabled.sort((a, b) => b.priority - a.priority);
    disabled.sort((a, b) => b.priority - a.priority);

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

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

interface DiscoveredCharger {
    host: string;
}

// Extend the global Window interface for the discovery and provisioning bridges
declare global {
    interface Window {
        tinkerforge_discovery?: {
            isSupported(): boolean;
            startDiscovery(): void;
            stopDiscovery(): void;
            getChargers(): string;
            navigateToCharger(host: string): void;
        };
        tinkerforge_provisioning?: {
            isSupported(): boolean;
            isProvisioning(): boolean;
            startProvisioning(): void;
            stopProvisioning(): void;
        };
        tinkerforge_devices?: {
            resetToDevices(): void;
        };
        onWarpChargersChanged?: (chargers: DiscoveredCharger[]) => void;
        onWarpDiscoveryStopped?: () => void;
        onWarpProvisioningFailed?: (reason: string) => void;
        onWarpProvisioningScanCancelled?: () => void;
        onWarpProvisioningStopped?: () => void;
    }
}

export {};

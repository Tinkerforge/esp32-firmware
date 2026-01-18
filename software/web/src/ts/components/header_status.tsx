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

import { h, Component } from "preact";
import * as API from "../api";
import * as util from "../util";
import { StatusVariant } from "./header_status_dropdown";
import { HeaderStatusDropdown } from "./header_status_dropdown";
import { get_all_statuses, get_overall_status, ModuleStatus, ModuleStatusEntry } from "../status_registry";

function status_to_variant(status: ModuleStatus): StatusVariant {
    switch (status) {
        case ModuleStatus.Ok: return "success";
        case ModuleStatus.Warning: return "warning";
        case ModuleStatus.Error: return "danger";
        case ModuleStatus.Disabled: return "disabled";
    }
}

interface HeaderStatusState {
    initialized: boolean;
}

export class HeaderStatus extends Component<{}, HeaderStatusState> {
    constructor() {
        super();

        this.state = {
            initialized: false
        };

        // Trigger initial render once modules info is available
        util.addApiEventListener("info/modules", () => {
            if (!this.state.initialized) {
                this.setState({ initialized: true });
            }
        });
    }

    override componentDidMount() {
        // Handle the case where info/modules was already received before this component mounted
        const modules = API.get("info/modules");
        if (modules !== null && modules !== undefined && !this.state.initialized) {
            this.setState({ initialized: true });
        }
    }

    render() {
        if (!util.render_allowed() || !this.state.initialized) {
            return null;
        }

        // get_all_statuses() calls API.get() for each provider, which subscribes
        // this component to those DeepSignal values. When any value changes,
        // Preact will automatically re-render this component.
        const entries = get_all_statuses();

        if (entries.length === 0) {
            return null;
        }

        const overall_status = get_overall_status(entries);
        const overall_variant = status_to_variant(overall_status);

        // Note: name and text are functions to support translations and dynamic values
        const dropdown_entries = entries.map(entry => ({
            label: entry.name(),
            variant: status_to_variant(entry.status),
            detail: entry.text ? entry.text() : undefined,
            href: entry.href
        }));

        return (
            <div class="header-status">
                <HeaderStatusDropdown
                    entries={dropdown_entries}
                    overall_variant={overall_variant}
                />
            </div>
        );
    }
}

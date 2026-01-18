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

import { h, Fragment } from "preact";
import { useState } from "preact/hooks";
import { Dropdown } from "react-bootstrap";
import { Activity, ChevronDown, ChevronRight } from "react-feather";
import { __ } from "../translation";

export type StatusVariant = "success" | "warning" | "danger" | "disabled";

interface StatusEntry {
    label: string;
    variant: StatusVariant;
    detail?: string;
    href?: string;
}

interface HeaderStatusDropdownProps {
    entries: StatusEntry[];
    overall_variant: StatusVariant;
}

function variant_to_bg_class(variant: StatusVariant): string {
    switch (variant) {
        case "success": return "bg-success";
        case "warning": return "bg-warning";
        case "danger": return "bg-danger";
        case "disabled": return "bg-secondary opacity-50";
        default: return "bg-secondary";
    }
}

function StatusEntryItem({ entry }: { entry: StatusEntry }) {
    return (
        <Dropdown.Item
            as={entry.href ? "a" : "div"}
            href={entry.href}
            className="d-flex align-items-center gap-2"
        >
            <span class={`rounded-circle ${variant_to_bg_class(entry.variant)}`}
                  style="width: 8px; height: 8px; flex-shrink: 0;"></span>
            <span class="flex-grow-1">{entry.label}</span>
            {entry.detail && (
                <span class="small text-secondary">{entry.detail}</span>
            )}
        </Dropdown.Item>
    );
}

function LegendDot({ variant }: { variant: StatusVariant }) {
    return (
        <span class={`rounded-circle ${variant_to_bg_class(variant)}`}
              style="width: 6px; height: 6px;"></span>
    );
}

export function HeaderStatusDropdown(props: HeaderStatusDropdownProps) {
    const [show_disabled, set_show_disabled] = useState(false);

    const enabled_entries = props.entries.filter(e => e.variant !== "disabled");
    const disabled_entries = props.entries.filter(e => e.variant === "disabled");

    return (
        <Dropdown align="end">
            <Dropdown.Toggle
                variant={props.overall_variant === "disabled" ? "secondary" : props.overall_variant}
                className="btn-sm rounded-pill d-inline-flex align-items-center gap-1"
                id="header-status-dropdown"
            >
                <Activity size={18} />
                <span class="d-none d-md-inline">
                    {__("component.header_status.status")}
                </span>
            </Dropdown.Toggle>

            <Dropdown.Menu style={{minWidth: "180px", zIndex: 1030}}>
                {enabled_entries.map((entry, i) => (
                    <StatusEntryItem key={i} entry={entry} />
                ))}
                {disabled_entries.length > 0 && (
                    <>
                        <Dropdown.Divider />
                        <div
                            class="d-flex align-items-center gap-1 px-3 py-2 small text-secondary"
                            style="cursor: pointer; user-select: none;"
                            onClick={(e) => {
                                e.stopPropagation();
                                set_show_disabled(!show_disabled);
                            }}
                        >
                            {show_disabled ? <ChevronDown size={16} /> : <ChevronRight size={16} />}
                            <span>{__("component.header_status.show_disabled")} ({disabled_entries.length})</span>
                        </div>
                        {show_disabled && disabled_entries.map((entry, i) => (
                            <StatusEntryItem key={`disabled-${i}`} entry={entry} />
                        ))}
                    </>
                )}
                <Dropdown.Divider />
                <div class="d-flex flex-wrap gap-2 px-3 py-2 small text-secondary">
                    <span class="d-flex align-items-center gap-1">
                        <LegendDot variant="success" />
                        {__("component.header_status.legend_ok")}
                    </span>
                    <span class="d-flex align-items-center gap-1">
                        <LegendDot variant="warning" />
                        {__("component.header_status.legend_warning")}
                    </span>
                    <span class="d-flex align-items-center gap-1">
                        <LegendDot variant="danger" />
                        {__("component.header_status.legend_error")}
                    </span>
                    {show_disabled &&
                        <span class="d-flex align-items-center gap-1">
                            <LegendDot variant="disabled" />
                            {__("component.header_status.legend_disabled")}
                        </span>
                    }
                </div>
            </Dropdown.Menu>
        </Dropdown>
    );
}

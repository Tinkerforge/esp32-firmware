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

import { Component, Fragment, h } from "preact";
import { Button } from "react-bootstrap";
import { FormRow } from "./form_row";

const CALENDAR_SLOTS_PER_DAY = 96;
const CALENDAR_DAYS = 7;
const CALENDAR_TOTAL_SLOTS = CALENDAR_DAYS * CALENDAR_SLOTS_PER_DAY;

function price_to_color(price: number): string {
    if (price === 0) return "var(--bs-secondary-bg)";
    if (price < 0) {
        // Negative prices: light blue -> deep blue, mapped over -5000..0 (-5..0 ct/kWh)
        const p = Math.min(Math.max(-price, 0), 5000);
        const ratio = p / 5000;
        const r = Math.round((1 - ratio) * 100);
        const g = Math.round((1 - ratio) * 160 + ratio * 80);
        const b = Math.round((1 - ratio) * 220 + ratio * 255);
        return `rgb(${r},${g},${b})`;
    }
    // Positive prices: green -> yellow -> red, mapped over 0..50000 (0..50 ct/kWh)
    const p = Math.min(Math.max(price, 0), 50000);
    const ratio = p / 50000;
    if (ratio < 0.5) {
        // green to yellow
        const r = Math.round(ratio * 2 * 255);
        const g = 200;
        return `rgb(${r},${g},60)`;
    } else {
        // yellow to red
        const g = Math.round((1 - (ratio - 0.5) * 2) * 200);
        return `rgb(255,${g},60)`;
    }
}

export interface CalendarGridProps {
    prices: number[];
    onPricesChanged?: (prices: number[]) => void;
    onEditClick?: () => void;
    edit_label?: string;
    day_names: string[];
    label?: string;
    label_muted?: string;
    unit: string;
    apply_label?: string;
    selection_label?: string;
    readonly?: boolean;
    split_below?: number;
}

interface CalendarGridState {
    sel_start_day: number;
    sel_start_slot: number;
    sel_end_day: number;
    sel_end_slot: number;
    selecting: boolean;
    has_selection: boolean;
    price_input: string;
    is_mobile: boolean;
}

export class CalendarGrid extends Component<CalendarGridProps, CalendarGridState> {
    constructor(props: CalendarGridProps) {
        super(props);

        this.state = {
            sel_start_day: -1,
            sel_start_slot: -1,
            sel_end_day: -1,
            sel_end_slot: -1,
            selecting: false,
            has_selection: false,
            price_input: "",
            is_mobile: window.innerWidth < (props.split_below || 768),
        };
    }

    get_day_name(day: number): string {
        if (day >= 0 && day < this.props.day_names.length) {
            return this.props.day_names[day];
        }
        return "";
    }

    get_selection_bounds() {
        const {sel_start_day, sel_start_slot, sel_end_day, sel_end_slot} = this.state;
        return {
            day_min: Math.min(sel_start_day, sel_end_day),
            day_max: Math.max(sel_start_day, sel_end_day),
            slot_min: Math.min(sel_start_slot, sel_end_slot),
            slot_max: Math.max(sel_start_slot, sel_end_slot),
        };
    }

    is_selected(day: number, slot: number): boolean {
        if (!this.state.has_selection) return false;
        const {day_min, day_max, slot_min, slot_max} = this.get_selection_bounds();
        return day >= day_min && day <= day_max && slot >= slot_min && slot <= slot_max;
    }

    get_selection_borders(day: number, slot: number): {top: boolean, bottom: boolean, left: boolean, right: boolean} {
        if (!this.state.has_selection) return {top: false, bottom: false, left: false, right: false};
        const {day_min, day_max, slot_min, slot_max} = this.get_selection_bounds();
        if (day < day_min || day > day_max || slot < slot_min || slot > slot_max) {
            return {top: false, bottom: false, left: false, right: false};
        }
        return {
            top: day === day_min,
            bottom: day === day_max,
            left: slot === slot_min,
            right: slot === slot_max,
        };
    }

    on_cell_mouse_down(day: number, slot: number, e: MouseEvent) {
        e.preventDefault();
        this.start_selection(day, slot);
    }

    on_cell_touch_start(day: number, slot: number, e: TouchEvent) {
        e.preventDefault();
        this.start_selection(day, slot);
    }

    start_selection(day: number, slot: number) {
        this.setState({
            sel_start_day: day,
            sel_start_slot: slot,
            sel_end_day: day,
            sel_end_slot: slot,
            selecting: true,
            has_selection: true,
        });
    }

    on_cell_mouse_enter(day: number, slot: number) {
        if (this.state.selecting) {
            this.setState({
                sel_end_day: day,
                sel_end_slot: slot,
            });
        }
    }

    on_touch_move = (e: TouchEvent) => {
        if (!this.state.selecting) return;
        const touch = e.touches[0];
        const el = document.elementFromPoint(touch.clientX, touch.clientY) as HTMLElement;
        if (!el) return;
        // Dragging over grid cells
        if (el.dataset["day"] !== undefined && el.dataset["slot"] !== undefined) {
            const day = parseInt(el.dataset["day"]);
            const slot = parseInt(el.dataset["slot"]);
            if (!isNaN(day) && !isNaN(slot)) {
                e.preventDefault();
                this.setState({
                    sel_end_day: day,
                    sel_end_slot: slot,
                });
            }
        }
        // Dragging over day labels
        else if (el.dataset["headerDay"] !== undefined) {
            const day = parseInt(el.dataset["headerDay"]);
            if (!isNaN(day)) {
                e.preventDefault();
                this.setState({
                    sel_start_slot: 0,
                    sel_end_day: day,
                    sel_end_slot: CALENDAR_SLOTS_PER_DAY - 1,
                });
            }
        }
        // Dragging over hour headers
        else if (el.dataset["headerHour"] !== undefined) {
            const hour = parseInt(el.dataset["headerHour"]);
            if (!isNaN(hour)) {
                e.preventDefault();
                const dragging_left = hour * 4 < this.state.sel_start_slot;
                const start_slot = dragging_left ? (Math.floor(this.state.sel_start_slot / 4) * 4 + 3) : (Math.floor(this.state.sel_start_slot / 4) * 4);
                const end_slot = dragging_left ? hour * 4 : hour * 4 + 3;
                this.setState({
                    sel_start_day: 0,
                    sel_end_day: CALENDAR_DAYS - 1,
                    sel_start_slot: start_slot,
                    sel_end_slot: end_slot,
                });
            }
        }
    }

    on_mouse_up = () => {
        if (this.state.selecting) {
            this.setState({selecting: false});
        }
    }

    on_touch_end = () => {
        if (this.state.selecting) {
            this.setState({selecting: false});
        }
    }

    on_day_mouse_down(day: number, e: MouseEvent) {
        e.preventDefault();
        this.setState({
            sel_start_day: day,
            sel_start_slot: 0,
            sel_end_day: day,
            sel_end_slot: CALENDAR_SLOTS_PER_DAY - 1,
            selecting: true,
            has_selection: true,
        });
    }

    on_day_mouse_enter(day: number) {
        if (this.state.selecting) {
            this.setState({
                sel_start_slot: 0,
                sel_end_day: day,
                sel_end_slot: CALENDAR_SLOTS_PER_DAY - 1,
            });
        }
    }

    on_day_touch_start(day: number, e: TouchEvent) {
        e.preventDefault();
        this.setState({
            sel_start_day: day,
            sel_start_slot: 0,
            sel_end_day: day,
            sel_end_slot: CALENDAR_SLOTS_PER_DAY - 1,
            selecting: true,
            has_selection: true,
        });
    }

    on_hour_mouse_down(hour: number, e: MouseEvent) {
        e.preventDefault();
        this.setState({
            sel_start_day: 0,
            sel_start_slot: hour * 4,
            sel_end_day: CALENDAR_DAYS - 1,
            sel_end_slot: hour * 4 + 3,
            selecting: true,
            has_selection: true,
        });
    }

    on_hour_mouse_enter(hour: number) {
        if (this.state.selecting) {
            const dragging_left = hour * 4 < this.state.sel_start_slot;
            // When dragging left, move start to end of original hour so full hour stays selected
            // When dragging right, move start to beginning of original hour
            const start_slot = dragging_left ? (Math.floor(this.state.sel_start_slot / 4) * 4 + 3) : (Math.floor(this.state.sel_start_slot / 4) * 4);
            const end_slot = dragging_left ? hour * 4 : hour * 4 + 3;
            this.setState({
                sel_start_day: 0,
                sel_end_day: CALENDAR_DAYS - 1,
                sel_start_slot: start_slot,
                sel_end_slot: end_slot,
            });
        }
    }

    on_hour_touch_start(hour: number, e: TouchEvent) {
        e.preventDefault();
        this.setState({
            sel_start_day: 0,
            sel_start_slot: hour * 4,
            sel_end_day: CALENDAR_DAYS - 1,
            sel_end_slot: hour * 4 + 3,
            selecting: true,
            has_selection: true,
        });
    }

    apply_price() {
        const val = parseFloat(this.state.price_input.replace(",", "."));
        if (isNaN(val)) return;
        // Convert ct/kWh to ct/1000 per kWh
        const price_internal = Math.round(val * 1000);
        this.set_selected_price(price_internal);
    }

    set_selected_price(price: number) {
        if (!this.state.has_selection) return;
        const {day_min, day_max, slot_min, slot_max} = this.get_selection_bounds();
        const prices = this.props.prices.slice();
        for (let d = day_min; d <= day_max; d++) {
            for (let s = slot_min; s <= slot_max; s++) {
                prices[d * CALENDAR_SLOTS_PER_DAY + s] = price;
            }
        }
        this.props.onPricesChanged(prices);
    }

    on_resize = () => {
        const mobile = window.innerWidth < (this.props.split_below || 768);
        if (mobile !== this.state.is_mobile) {
            this.setState({is_mobile: mobile});
        }
    }

    render_grid(slot_start: number, slot_end: number, hour_start: number, hour_end: number) {
        const hours = Array.from({length: hour_end - hour_start + 1}, (_, i) => hour_start + i);
        const prices = this.props.prices;
        const ro = this.props.readonly;

        return (
            <div class="calendar-grid" style="overflow-x: auto;">
                <table class="calendar-table" style="border-collapse: collapse; user-select: none; width: 100%; table-layout: fixed; touch-action: none;"
                       onTouchMove={ro ? undefined : this.on_touch_move}>
                    <thead>
                        <tr>
                            <th class="calendar-corner" style="min-width: 32px; width: 32px;"></th>
                            {hours.map((hr) =>
                                <th
                                    key={hr}
                                    colSpan={4}
                                    class="calendar-hour-header"
                                    data-header-hour={hr}
                                    style={`text-align: center; font-size: 0.7em; padding: 6px 1px; border: 1px solid var(--bs-border-color);${ro ? "" : " cursor: pointer;"}`}
                                    onMouseDown={ro ? undefined : (e: MouseEvent) => this.on_hour_mouse_down(hr, e)}
                                    onMouseEnter={ro ? undefined : () => this.on_hour_mouse_enter(hr)}
                                    onTouchStart={ro ? undefined : (e: TouchEvent) => this.on_hour_touch_start(hr, e)}
                                >
                                    {hr}
                                </th>
                            )}
                        </tr>
                    </thead>
                    <tbody>
                        {Array.from({length: CALENDAR_DAYS}, (_, day) =>
                            <tr key={day}>
                                <td
                                    class="calendar-day-label"
                                    data-header-day={day}
                                    style={`font-size: 0.75em; font-weight: bold; padding: 1px 3px; white-space: nowrap; border: 1px solid var(--bs-border-color);${ro ? "" : " cursor: pointer;"}`}
                                    onMouseDown={ro ? undefined : (e: MouseEvent) => this.on_day_mouse_down(day, e)}
                                    onMouseEnter={ro ? undefined : () => this.on_day_mouse_enter(day)}
                                    onTouchStart={ro ? undefined : (e: TouchEvent) => this.on_day_touch_start(day, e)}
                                >
                                    {this.get_day_name(day)}
                                </td>
                                {Array.from({length: slot_end - slot_start + 1}, (_, i) => {
                                    const slot = slot_start + i;
                                    const idx = day * CALENDAR_SLOTS_PER_DAY + slot;
                                    const price = prices[idx];
                                    const selected = ro ? false : this.is_selected(day, slot);
                                    const borders = ro ? {top: false, bottom: false, left: false, right: false} : this.get_selection_borders(day, slot);
                                    const bg = price_to_color(price);
                                    const hour_str = ("0" + Math.floor(slot / 4)).slice(-2);
                                    const min_str = ("0" + (slot % 4) * 15).slice(-2);
                                    const border_color = "var(--bs-primary)";
                                    // Build inset box-shadow for selection borders (no layout shift)
                                    const shadows: string[] = [];
                                    if (borders.top)    shadows.push(`inset 0 1px 0 0 ${border_color}`);
                                    if (borders.bottom) shadows.push(`inset 0 -1px 0 0 ${border_color}`);
                                    if (borders.left)   shadows.push(`inset 1px 0 0 0 ${border_color}`);
                                    if (borders.right)  shadows.push(`inset -1px 0 0 0 ${border_color}`);
                                    const box_shadow = shadows.length > 0 ? `box-shadow: ${shadows.join(", ")};` : "";
                                    return (
                                        <td
                                            key={slot}
                                            class="calendar-cell"
                                            data-day={day}
                                            data-slot={slot}
                                            style={`background: ${bg}; padding: 0; height: 20px; min-width: 4px; border: 1px solid var(--bs-border-color);${ro ? "" : " cursor: crosshair;"}`
                                                   + box_shadow
                                                   + (selected ? " opacity: 0.85;" : "")}
                                            title={`${this.get_day_name(day)} ${hour_str}:${min_str} - ${(price / 1000).toFixed(2)} ${this.props.unit}`}
                                            onMouseDown={ro ? undefined : (e: MouseEvent) => this.on_cell_mouse_down(day, slot, e)}
                                            onMouseEnter={ro ? undefined : () => this.on_cell_mouse_enter(day, slot)}
                                            onTouchStart={ro ? undefined : (e: TouchEvent) => this.on_cell_touch_start(day, slot, e)}
                                        />
                                    );
                                })}
                            </tr>
                        )}
                    </tbody>
                </table>
            </div>
        );
    }

    override componentDidMount() {
        document.addEventListener("mouseup", this.on_mouse_up);
        document.addEventListener("touchend", this.on_touch_end);
        window.addEventListener("resize", this.on_resize);
    }

    override componentWillUnmount() {
        document.removeEventListener("mouseup", this.on_mouse_up);
        document.removeEventListener("touchend", this.on_touch_end);
        window.removeEventListener("resize", this.on_resize);
    }

    render() {
        const prices = this.props.prices;
        const ro = this.props.readonly;

        const grids = this.state.is_mobile
            ? <>
                {this.render_grid(0, 47, 0, 11)}
                {this.render_grid(48, 95, 12, 23)}
              </>
            : this.render_grid(0, 95, 0, 23);

        return (
            <div class="calendar-grid-container">
                {this.props.onEditClick
                    ? <div style="position: relative; cursor: pointer;" onClick={() => this.props.onEditClick()}>
                        {grids}
                        <div style="position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); pointer-events: none;">
                            <Button variant="primary" style="box-shadow: 0 0 20px 10px var(--bs-body-bg);">{this.props.edit_label}</Button>
                        </div>
                      </div>
                    : grids
                }
                {!ro && <FormRow label={this.props.label} label_muted={this.props.label_muted} class="mt-3">
                    <div class="input-group">
                        <input
                            type="text"
                            class="form-control"
                            value={this.state.price_input}
                            placeholder="0.00"
                            onInput={(e) => this.setState({price_input: (e.target as HTMLInputElement).value})}
                            onKeyDown={(e) => { if (e.key === "Enter") this.apply_price(); }}
                        />
                        <span class="input-group-text">{this.props.unit}</span>
                        <Button variant="primary" disabled={!this.state.has_selection} onClick={() => this.apply_price()}>
                            {this.props.apply_label}
                        </Button>
                    </div>
                </FormRow>}
                {/* Legend */}
                <div class="mt-2 d-flex align-items-center" style="font-size: 0.75em; gap: 6px; flex-wrap: wrap;">
                    {[-5000, 0, 5000, 10000, 15000, 20000, 25000, 30000, 40000, 50000].map((price) =>
                        <div key={price} class="d-flex align-items-center" style="gap: 2px;">
                            <div style={`width: 14px; height: 12px; background: ${price_to_color(price)}; border: 1px solid var(--bs-border-color);`} />
                            <span>{price / 1000}</span>
                        </div>
                    )}
                    <span>{this.props.unit}</span>
                </div>
                {/* Selected prices */}
                {!ro && (() => {
                    if (this.state.has_selection) {
                        const {day_min, day_max, slot_min, slot_max} = this.get_selection_bounds();
                        const unique_prices = new Set<number>();
                        for (let d = day_min; d <= day_max; d++) {
                            for (let s = slot_min; s <= slot_max; s++) {
                                unique_prices.add(prices[d * CALENDAR_SLOTS_PER_DAY + s]);
                            }
                        }
                        const fmt_time = (slot: number) => ("0" + Math.floor(slot / 4)).slice(-2) + ":" + ("0" + (slot % 4) * 15).slice(-2);
                        const end_slot_time = slot_max + 1;
                        const end_time = ("0" + Math.floor(end_slot_time / 4)).slice(-2) + ":" + ("0" + (end_slot_time % 4) * 15).slice(-2);
                        const range = day_min === day_max
                            ? `${this.get_day_name(day_min)} ${fmt_time(slot_min)}-${end_time}`
                            : `${this.get_day_name(day_min)} ${fmt_time(slot_min)} - ${this.get_day_name(day_max)} ${end_time}`;
                        const sorted = Array.from(unique_prices).sort((a, b) => a - b);
                        const price_str = sorted.map((p) => (p / 1000).toFixed(2)).join(", ");
                        return (
                            <div class="mt-1" style="font-size: 0.8em;">
                                <strong>{this.props.selection_label}:</strong> {range}: {price_str} {this.props.unit}
                            </div>
                        );
                    }
                    return <div class="mt-1" style="font-size: 0.8em;">{"\u00A0"}</div>;
                })()}
            </div>
        );
    }
}

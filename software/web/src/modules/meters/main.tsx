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

import { METERS_SLOTS } from "../../build";
import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { __, translate_unchecked } from "../../ts/translation";
import { h, createRef, Fragment, Component, RefObject, ComponentChild } from "preact";
import { Button, ButtonGroup } from "react-bootstrap";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "../../ts/components/input_select";
import { FormSeparator } from "../../ts/components/form_separator";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { OutputFloat } from "../../ts/components/output_float";
import { SubPage } from "../../ts/components/sub_page";
import { UplotLoader } from "../../ts/components/uplot_loader";
import { UplotWrapper, UplotData } from "../../ts/components/uplot_wrapper";
import { MeterValueID, METER_VALUE_IDS, METER_VALUE_INFOS, METER_VALUE_ORDER } from "./meter_value_id";
import { MeterClassID } from "./meter_class_id.enum";
import { MeterConfig, MeterConfigPlugin } from "./types";
import { Table } from "../../ts/components/table";
import { PageHeader } from "../../ts/components/page_header";
import { plugins_init } from "./plugins";
import { InputDate } from "../../ts/components/input_date";
import { InputTime } from "../../ts/components/input_time";
import { InputText } from "../../ts/components/input_text";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { HelpCircle, Zap, ZapOff, ChevronRight, BarChart2 } from "react-feather";

export function MetersNavbar() {
    return <NavbarItem name="meters" module="meters" title={__("meters.navbar.meters")} symbol={<BarChart2 />} />;
}

const PHASE_CONNECTED_VOLTAGE_THRESHOLD = 180.0; // V
const PHASE_ACTIVE_CURRENT_THRESHOLD = 0.3; // A

let config_plugins: {[meter_class: number]: MeterConfigPlugin} = {};

interface CachedData {
    timestamps: number[];
    samples: number[/*meter_slot*/][];
}

type NumberToNumber = {[id: number]: number};

interface MetersProps {
    status_ref?: RefObject<MetersStatus>;
}

interface MetersState {
    states: {[meter_slot: number]: Readonly<API.getType['meters/0/state']>};
    configs_plot: {[meter_slot: number]: MeterConfig};
    configs_table: {[meter_slot: number]: MeterConfig};
    values_by_id: {[meter_slot: number]: NumberToNumber};
    chart_selected: "history_48"|"history_24"|"history_12"|"history_6"|"history_3"|"live";
    charger_meter_slot: number;
    addMeterSlot: number;
    addMeter: MeterConfig;
    editMeterSlot: number;
    editMeter: MeterConfig;
    extraShow: boolean[/*meter_slot*/];
}

export function get_meter_power_index(value_ids: Readonly<number[]>) {
    let idx = -1;

    if (value_ids && value_ids.length > 0) {
        idx = value_ids.indexOf(MeterValueID.PowerActiveLSumImExDiff);

        if (idx < 0) {
            idx = value_ids.indexOf(MeterValueID.PowerDCImExDiff);

            if (idx < 0) {
                idx = value_ids.indexOf(MeterValueID.PowerDCChaDisDiff);
            }
        }
    }

    return idx;
}

function calculate_live_data(offset: number, samples_per_second: number, samples: number[/*meter_slot*/][]): CachedData {
    let timestamp_slot_count: number = 0;

    if (samples_per_second == 0) { // implies atmost one sample
        timestamp_slot_count = 1;
    } else {
        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            if (samples[meter_slot] !== null) {
                timestamp_slot_count = Math.max(timestamp_slot_count, samples[meter_slot].length);
            }
        }
    }

    let data: CachedData = {timestamps: new Array(timestamp_slot_count), samples: new Array(METERS_SLOTS)};
    let now = Date.now();
    let start: number;
    let step: number;

    if (samples_per_second == 0) {
        start = now - offset;
        step = 0;
    } else {
        // (timestamp_slot_count - 1) because samples_per_second defines the gaps between
        // two samples. with N samples there are (N - 1) gaps, while the lastest/newest
        // sample is offset milliseconds old
        start = now - (timestamp_slot_count - 1) / samples_per_second * 1000 - offset;
        step = 1 / samples_per_second * 1000;
    }

    for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
        data.timestamps[timestamp_slot] = (start + timestamp_slot * step) / 1000;
    }

    for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
        if (samples[meter_slot] === null) {
            data.samples[meter_slot] = [];
        }
        else {
            data.samples[meter_slot] = samples[meter_slot];
        }
    }

    return data;
}

function calculate_history_data(offset: number, samples: number[/*meter_slot*/][]): CachedData {
    const HISTORY_MINUTE_INTERVAL = 4;

    let timestamp_slot_count: number = 0;

    for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
        if (samples[meter_slot] !== null) {
            timestamp_slot_count = Math.max(timestamp_slot_count, samples[meter_slot].length);
        }
    }

    let data: CachedData = {timestamps: new Array(timestamp_slot_count), samples: new Array(METERS_SLOTS)};
    let now = Date.now();
    let step = HISTORY_MINUTE_INTERVAL * 60 * 1000;

    // (timestamp_slot_count - 1) because step defines the gaps between two samples.
    // with N samples there are (N - 1) gaps, while the lastest/newest sample is
    // offset milliseconds old. there might be no data point on a full hour
    // interval. to get nice aligned ticks nudge the ticks by at most half of a
    // sampling interval
    let start = Math.round((now - (timestamp_slot_count - 1) * step - offset) / step) * step;

    for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
        data.timestamps[timestamp_slot] = (start + timestamp_slot * step) / 1000;
    }

    for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
        if (samples[meter_slot] === null) {
            data.samples[meter_slot] = [];
        }
        else {
            data.samples[meter_slot] = samples[meter_slot];
        }
    }

    return data;
}

function array_append<T>(a: Array<T>, b: Array<T>, tail: number): Array<T> {
    a.push(...b);

    return a.slice(-tail);
}

type MetersConfig = API.getType['meters/0/config'];

export class Meters extends ConfigComponent<'meters/0/config', MetersProps, MetersState> {
    live_initialized = false;
    live_data: CachedData = {timestamps: [], samples: []};
    pending_live_data: CachedData;
    history_initialized = false;
    history_data: CachedData = {timestamps: [], samples: []};
    uplot_loader_live_ref = createRef();
    uplot_loader_history_ref = createRef();
    uplot_wrapper_live_ref = createRef();
    uplot_wrapper_history_ref = createRef();
    value_ids: {[meter_slot: number]: Readonly<number[]>} = {};
    value_idx_by_id: {[meter_slot: number]: NumberToNumber} = {};
    values: {[meter_slot: number]: Readonly<number[]>} = {};

    constructor() {
        super('meters/0/config',
              __("meters.script.save_failed"),
              __("meters.script.reboot_content_changed"), {
                  states: {},
                  configs_plot: {},
                  configs_table: {},
                  values_by_id: {},
                  chart_selected: "history_48",
                  charger_meter_slot: null,
                  addMeterSlot: null,
                  addMeter: [MeterClassID.None, null],
                  editMeterSlot: null,
                  editMeter: [MeterClassID.None, null],
                  extraShow: new Array<boolean>(7),
              });

        this.state.extraShow.fill(false);

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            this.live_data.samples.push([]);
            this.history_data.samples.push([]);
        }

        util.addApiEventListener('info/modules', () => {
            this.update_live_cache();
            this.update_history_cache();
        });

        util.addApiEventListener_unchecked('evse/meter_config', () => {
            let config = API.get_unchecked('evse/meter_config');

            this.setState({
                charger_meter_slot: config.slot,
            });
        });

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            util.addApiEventListener_unchecked(`meters/${meter_slot}/state`, () => {
                let state = API.get_unchecked(`meters/${meter_slot}/state`);

                this.setState((prevState) => ({
                    states: {
                        ...prevState.states,
                        [meter_slot]: state
                    }
                }));
            });

            util.addApiEventListener_unchecked(`meters/${meter_slot}/config`, () => {
                let config = API.get_unchecked(`meters/${meter_slot}/config`);

                this.setState((prevState) => ({
                    configs_plot: {
                        ...prevState.configs_plot,
                        [meter_slot]: config
                    }
                }));

                if (!this.isDirty()) {
                    this.setState((prevState) => ({
                        configs_table: {
                            ...prevState.configs_table,
                            [meter_slot]: config
                        }
                    }));
                }
            });

            util.addApiEventListener_unchecked(`meters/${meter_slot}/value_ids`, () => {
                this.value_ids[meter_slot] = API.get_unchecked(`meters/${meter_slot}/value_ids`);
                let value_idx_by_id: NumberToNumber = {};
                let values_by_id: NumberToNumber = {};

                for (let id of METER_VALUE_IDS) {
                    let idx = this.value_ids[meter_slot].indexOf(id);

                    if (idx >= 0) {
                        value_idx_by_id[id] = idx;

                        if (this.values[meter_slot]) {
                            values_by_id[id] = this.values[meter_slot][idx];
                        }
                    }
                }

                this.value_idx_by_id[meter_slot] = value_idx_by_id;

                this.setState((prevState) => ({
                    values_by_id: {
                        ...prevState.values_by_id,
                        [meter_slot]: values_by_id
                    }
                }));
            });

            util.addApiEventListener_unchecked(`meters/${meter_slot}/values`, () => {
                this.values[meter_slot] = API.get_unchecked(`meters/${meter_slot}/values`);
                let values_by_id: NumberToNumber = {};

                if (this.value_idx_by_id[meter_slot]) {
                    for (let id in this.value_idx_by_id[meter_slot]) {
                        let idx = this.value_idx_by_id[meter_slot][id];

                        values_by_id[id] = this.values[meter_slot][idx];
                    }
                }

                this.setState((prevState) => ({
                    values_by_id: {
                        ...prevState.values_by_id,
                        [meter_slot]: values_by_id
                    }
                }));
            });
        }

        util.addApiEventListener("meters/live_samples", () => {
            if (!this.live_initialized) {
                // received live_samples before live cache initialization
                this.update_live_cache();
                return;
            }

            let live = API.get("meters/live_samples");
            let live_extra = calculate_live_data(0, live.samples_per_second, live.samples);

            this.pending_live_data.timestamps.push(...live_extra.timestamps);

            for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                this.pending_live_data.samples[meter_slot].push(...live_extra.samples[meter_slot]);
            }

            if (this.pending_live_data.timestamps.length >= 5) {
                this.live_data.timestamps = array_append(this.live_data.timestamps, this.pending_live_data.timestamps, 720);

                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                    this.live_data.samples[meter_slot] = array_append(this.live_data.samples[meter_slot], this.pending_live_data.samples[meter_slot], 720);
                }

                this.pending_live_data.timestamps = [];
                this.pending_live_data.samples = [];

                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                    this.pending_live_data.samples.push([]);
                }

                if (this.state.chart_selected == "live") {
                    this.update_live_uplot();
                }
            }
        });

        util.addApiEventListener("meters/history_samples", () => {
            if (!this.history_initialized) {
                // received history_samples before history cache initialization
                this.update_history_cache();
                return;
            }

            let history = API.get("meters/history_samples");
            let history_samples: number[][] = new Array(METERS_SLOTS);

            for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                if (history.samples[meter_slot] !== null) {
                    history_samples[meter_slot] = array_append(this.history_data.samples[meter_slot], history.samples[meter_slot], 720);
                }
            }

            this.history_data = calculate_history_data(0, history_samples);

            if (this.state.chart_selected.startsWith("history_")) {
                this.update_history_uplot();
            }

            this.update_status_uplot();
        });
    }

    componentDidMount() {
        if (this.props.status_ref) {
            this.props.status_ref.current.set_on_mount(() => this.update_status_uplot());
        }
    }

    update_live_cache() {
        this.update_live_cache_async()
            .then((success: boolean) => {
                if (!success) {
                    window.setTimeout(() => {
                        this.update_live_cache();
                    }, 100);

                    return;
                }

                this.update_live_uplot();
            });
    }

    async update_live_cache_async() {
        let response: string = '';

        try {
            response = await (await util.download('meters/live')).text();
        } catch (e) {
            console.log('Meters: Could not get meters live data: ' + e);
            return false;
        }

        let payload = JSON.parse(response);

        this.live_initialized = true;
        this.live_data = calculate_live_data(payload.offset, payload.samples_per_second, payload.samples);
        this.pending_live_data = {timestamps: [], samples: []}

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            this.pending_live_data.samples.push([]);
        }

        return true;
    }

    update_history_cache() {
        this.update_history_cache_async()
            .then((success: boolean) => {
                if (!success) {
                    window.setTimeout(() => {
                        this.update_history_cache();
                    }, 100);

                    return;
                }

                this.update_history_uplot();
                this.update_status_uplot();
            });
    }

    async update_history_cache_async() {
        let response: string = '';

        try {
            response = await (await util.download('meters/history')).text();
        } catch (e) {
            console.log('Meters: Could not get meters history data: ' + e);
            return false;
        }

        let payload = JSON.parse(response);

        this.history_initialized = true;
        this.history_data = calculate_history_data(payload.offset, payload.samples);

        return true;
    }

    update_live_uplot() {
        if (this.live_initialized && this.uplot_loader_live_ref.current && this.uplot_wrapper_live_ref.current) {
            let live_data: UplotData = {
                keys: [null],
                names: [null],
                values: [this.live_data.timestamps],
            };

            for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                if (this.live_data.samples[meter_slot].length > 0) {
                    live_data.keys.push('meter_' + meter_slot);
                    live_data.names.push(get_meter_name(this.state.configs_plot, meter_slot));
                    live_data.values.push(this.live_data.samples[meter_slot]);
                }
            }

            this.uplot_loader_live_ref.current.set_data(live_data.keys.length > 1);
            this.uplot_wrapper_live_ref.current.set_data(live_data);
        }
    }

    update_history_uplot() {
        if (this.history_initialized && this.uplot_loader_history_ref.current && this.uplot_wrapper_history_ref.current) {
            let history_tail = 720; // history_48

            if (this.state.chart_selected == 'history_24') {
                history_tail = 360;
            }
            else if (this.state.chart_selected == 'history_12') {
                history_tail = 180;
            }
            else if (this.state.chart_selected == 'history_6') {
                history_tail = 90;
            }
            else if (this.state.chart_selected == 'history_3') {
                history_tail = 45;
            }

            let history_data: UplotData = {
                keys: [null],
                names: [null],
                values: [this.history_data.timestamps.slice(-history_tail)],
            };

            for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                if (this.history_data.samples[meter_slot].length > 0) {
                    history_data.keys.push('meter_' + meter_slot);
                    history_data.names.push(get_meter_name(this.state.configs_plot, meter_slot));
                    history_data.values.push(this.history_data.samples[meter_slot].slice(-history_tail));
                }
            }

            this.uplot_loader_history_ref.current.set_data(history_data.keys.length > 1);
            this.uplot_wrapper_history_ref.current.set_data(history_data);
        }
    }

    update_status_uplot() {
        if (this.history_initialized && this.props.status_ref && this.props.status_ref.current && this.props.status_ref.current.uplot_loader_ref.current && this.props.status_ref.current.uplot_wrapper_ref.current) {
            let status_data: UplotData = {
                keys: [null],
                names: [null],
                values: [this.history_data.timestamps],
            };

            let meter_slot = this.props.status_ref.current.state.charger_meter_slot;

            if (this.history_data.samples[meter_slot].length > 0) {
                status_data.keys.push('meter_' + meter_slot);
                status_data.names.push(get_meter_name(this.state.configs_plot, meter_slot));
                status_data.values.push(this.history_data.samples[meter_slot]);
            }

            this.props.status_ref.current.uplot_loader_ref.current.set_data(status_data.keys.length > 1);
            this.props.status_ref.current.uplot_wrapper_ref.current.set_data(status_data);
        }
    }

    override async sendSave(t: "meters/0/config", new_config: MetersConfig) {
        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            await API.save_unchecked(
                `meters/${meter_slot}/config`,
                this.state.configs_table[meter_slot],
                __("meters.script.save_failed"),
                meter_slot == METERS_SLOTS - 1 ? this.reboot_string : undefined);
        }
    }

    override async sendReset(t: "meters/0/config") {
        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            await API.reset_unchecked(`meters/${meter_slot}/config`, this.error_string, this.reboot_string);
        }
    }

    override getIsModified(t: "meters/0/config"): boolean {
        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            if (API.is_modified_unchecked(`meters/${meter_slot}/config`))
                return true;
        }

        return false;
    }

    render(props: {}, state: Readonly<MetersState>) {
        if (!util.render_allowed())
            return <SubPage name="meters" />;

        let active_meter_slots = Object.keys(state.configs_table).filter((meter_slot_str) => state.configs_table[parseInt(meter_slot_str)][0] != MeterClassID.None);
        let show_plot = API.hasFeature("meters");

        return (
            <SubPage name="meters" colClasses="col-xl-10">
                {show_plot ? <><PageHeader title={__("meters.content.meters")}/>

                <FormSeparator heading={__("meters.status.power_history")} first={true} colClasses={"justify-content-between align-items-center col"} extraClasses={"pr-0 pr-lg-3"} >
                    <div class="mb-2">
                        <InputSelect value={this.state.chart_selected} onValue={(v) => {
                            let chart_selected: "history_48"|"history_24"|"history_12"|"history_6"|"history_3"|"live" = v as any;

                            this.setState({chart_selected: chart_selected}, () => {
                                if (chart_selected == 'live') {
                                    this.uplot_loader_live_ref.current.set_show(true);
                                    this.uplot_wrapper_live_ref.current.set_show(true);
                                    this.uplot_loader_history_ref.current.set_show(false);
                                    this.uplot_wrapper_history_ref.current.set_show(false);

                                    this.update_live_uplot();
                                }
                                else {
                                    this.uplot_loader_history_ref.current.set_show(true);
                                    this.uplot_wrapper_history_ref.current.set_show(true);
                                    this.uplot_loader_live_ref.current.set_show(false);
                                    this.uplot_wrapper_live_ref.current.set_show(false);

                                    this.update_history_uplot();
                                }
                            });
                        }}
                            items={[
                                ["history_48", __("meters.content.history_48")],
                                ["history_24", __("meters.content.history_24")],
                                ["history_12", __("meters.content.history_12")],
                                ["history_6", __("meters.content.history_6")],
                                ["history_3", __("meters.content.history_3")],
                                ["live", __("meters.content.live")],
                            ]}/>
                    </div>
                </FormSeparator></>
                : undefined}

                <div hidden={!show_plot}>
                    <div style="position: relative;"> {/* this plain div is neccessary to make the size calculation stable in safari. without this div the height continues to grow */}
                        <UplotLoader ref={this.uplot_loader_live_ref}
                                        show={false}
                                        marker_class={'h3'}
                                        no_data={__("meters.content.no_data")}
                                        loading={__("meters.content.loading")} >
                            <UplotWrapper ref={this.uplot_wrapper_live_ref}
                                            class="meters-chart pb-3"
                                            sub_page="meters"
                                            color_cache_group="meters.default"
                                            show={false}
                                            on_mount={() => this.update_live_uplot()}
                                            legend_time_label={__("meters.script.time")}
                                            legend_time_with_seconds={true}
                                            aspect_ratio={3}
                                            x_height={30}
                                            x_padding_factor={0}
                                            x_include_date={false}
                                            y_diff_min={100}
                                            y_unit="W"
                                            y_label={__("meters.script.power") + " [Watt]"}
                                            y_digits={0} />
                        </UplotLoader>
                        <UplotLoader ref={this.uplot_loader_history_ref}
                                        show={true}
                                        marker_class={'h3'}
                                        no_data={__("meters.content.no_data")}
                                        loading={__("meters.content.loading")} >
                            <UplotWrapper ref={this.uplot_wrapper_history_ref}
                                            class="meters-chart pb-3"
                                            sub_page="meters"
                                            color_cache_group="meters.default"
                                            show={true}
                                            on_mount={() => this.update_history_uplot()}
                                            legend_time_label={__("meters.script.time")}
                                            legend_time_with_seconds={false}
                                            aspect_ratio={3}
                                            x_height={50}
                                            x_padding_factor={0}
                                            x_include_date={true}
                                            y_min={0}
                                            y_max={1500}
                                            y_unit="W"
                                            y_label={__("meters.script.power") + " [Watt]"}
                                            y_digits={0} />
                        </UplotLoader>
                    </div>
                </div>

                <ConfigForm id="meters_config_form" title={show_plot ? __("meters.content.settings") : __("meters.content.meters")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty} small={show_plot}>
                    <div class="mb-3">
                        <Table
                            tableTill="lg"
                            columnNames={[__("meters.content.table_display_name"), __("meters.content.table_power"), __("meters.content.table_energy_import"), __("meters.content.table_energy_export"), __("meters.content.table_phases")]}
                            rows={active_meter_slots.map((meter_slot_str) => {
                                let meter_slot = parseInt(meter_slot_str);
                                let config = state.configs_table[meter_slot];
                                let values_by_id = state.values_by_id[meter_slot];
                                let power: number = null;
                                let energy_import: number = null;
                                let energy_export: number = null;
                                let phases: ["?"|"d"|"c"|"a", "?"|"d"|"c"|"a", "?"|"d"|"c"|"a"] = ["?", "?", "?"]; // [d]isconected, [c]onnected, [a]ctive
                                let highlighted_value_ids: number[] = [];

                                if (util.hasValue(values_by_id)) {
                                    let power_idx = get_meter_power_index(this.value_ids[meter_slot]);

                                    if (power_idx >= 0) {
                                        power = this.values[meter_slot][power_idx];
                                        highlighted_value_ids.push(this.value_ids[meter_slot][power_idx]);
                                    }

                                    energy_import = values_by_id[MeterValueID.EnergyActiveLSumImportResettable];

                                    if (util.hasValue(energy_import)) {
                                        highlighted_value_ids.push(MeterValueID.EnergyActiveLSumImportResettable);
                                    }
                                    else {
                                        energy_import = values_by_id[MeterValueID.EnergyActiveLSumImport];

                                        if (util.hasValue(energy_import)) {
                                            highlighted_value_ids.push(MeterValueID.EnergyActiveLSumImport);
                                        }
                                        else {
                                            energy_import = values_by_id[MeterValueID.EnergyDCImportResettable];

                                            if (util.hasValue(energy_import)) {
                                                highlighted_value_ids.push(MeterValueID.EnergyDCImportResettable);
                                            }
                                            else {
                                                energy_import = values_by_id[MeterValueID.EnergyDCImport];

                                                if (util.hasValue(energy_import)) {
                                                    highlighted_value_ids.push(MeterValueID.EnergyDCImport);
                                                }
                                                else {
                                                    energy_import = values_by_id[MeterValueID.EnergyDCChargeResettable];

                                                    if (util.hasValue(energy_import)) {
                                                        highlighted_value_ids.push(MeterValueID.EnergyDCChargeResettable);
                                                    }
                                                    else {
                                                        energy_import = values_by_id[MeterValueID.EnergyDCCharge];
                                                        highlighted_value_ids.push(MeterValueID.EnergyDCCharge);
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    energy_export = values_by_id[MeterValueID.EnergyActiveLSumExportResettable];

                                    if (util.hasValue(energy_export)) {
                                        highlighted_value_ids.push(MeterValueID.EnergyActiveLSumExportResettable);
                                    }
                                    else {
                                        energy_export = values_by_id[MeterValueID.EnergyActiveLSumExport];

                                        if (util.hasValue(energy_export)) {
                                            highlighted_value_ids.push(MeterValueID.EnergyActiveLSumExport);
                                        }
                                        else {
                                            energy_export = values_by_id[MeterValueID.EnergyDCExportResettable];

                                            if (util.hasValue(energy_export)) {
                                                highlighted_value_ids.push(MeterValueID.EnergyDCExportResettable);
                                            }
                                            else {
                                                energy_export = values_by_id[MeterValueID.EnergyDCExport];

                                                if (util.hasValue(energy_export)) {
                                                    highlighted_value_ids.push(MeterValueID.EnergyDCExport);
                                                }
                                                else {
                                                    energy_export = values_by_id[MeterValueID.EnergyDCDischargeResettable];

                                                    if (util.hasValue(energy_export)) {
                                                        highlighted_value_ids.push(MeterValueID.EnergyDCDischargeResettable);
                                                    }
                                                    else {
                                                        energy_export = values_by_id[MeterValueID.EnergyDCDischarge];
                                                        highlighted_value_ids.push(MeterValueID.EnergyDCDischarge);
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    let voltage_L1 = values_by_id[MeterValueID.VoltageL1N];
                                    let voltage_L2 = values_by_id[MeterValueID.VoltageL2N];
                                    let voltage_L3 = values_by_id[MeterValueID.VoltageL3N];

                                    let current_L1_import = values_by_id[MeterValueID.CurrentL1Import];
                                    let current_L2_import = values_by_id[MeterValueID.CurrentL2Import];
                                    let current_L3_import = values_by_id[MeterValueID.CurrentL3Import];

                                    let current_L1_export = values_by_id[MeterValueID.CurrentL1Export];
                                    let current_L2_export = values_by_id[MeterValueID.CurrentL2Export];
                                    let current_L3_export = values_by_id[MeterValueID.CurrentL3Export];

                                    let current_L1_im_ex_sum = values_by_id[MeterValueID.CurrentL1ImExSum];
                                    let current_L2_im_ex_sum = values_by_id[MeterValueID.CurrentL2ImExSum];
                                    let current_L3_im_ex_sum = values_by_id[MeterValueID.CurrentL3ImExSum];

                                    let current_L1_im_ex_diff = values_by_id[MeterValueID.CurrentL1ImExDiff];
                                    let current_L2_im_ex_diff = values_by_id[MeterValueID.CurrentL2ImExDiff];
                                    let current_L3_im_ex_diff = values_by_id[MeterValueID.CurrentL3ImExDiff];

                                    if (util.hasValue(voltage_L1)) {
                                        phases[0] = voltage_L1 > PHASE_CONNECTED_VOLTAGE_THRESHOLD ? "c" : "d";
                                    }

                                    if (util.hasValue(voltage_L2)) {
                                        phases[1] = voltage_L2 > PHASE_CONNECTED_VOLTAGE_THRESHOLD ? "c" : "d";
                                    }

                                    if (util.hasValue(voltage_L3)) {
                                        phases[2] = voltage_L3 > PHASE_CONNECTED_VOLTAGE_THRESHOLD ? "c" : "d";
                                    }

                                    if ((util.hasValue(current_L1_import) && current_L1_import > PHASE_ACTIVE_CURRENT_THRESHOLD)
                                        || (util.hasValue(current_L1_export) && current_L1_export > PHASE_ACTIVE_CURRENT_THRESHOLD)
                                        || (util.hasValue(current_L1_im_ex_sum) && current_L1_im_ex_sum > PHASE_ACTIVE_CURRENT_THRESHOLD)
                                        || (util.hasValue(current_L1_im_ex_diff) && Math.abs(current_L1_im_ex_diff) > PHASE_ACTIVE_CURRENT_THRESHOLD)) {
                                        phases[0] = "a";
                                    }

                                    if ((util.hasValue(current_L2_import) && current_L2_import > PHASE_ACTIVE_CURRENT_THRESHOLD)
                                        || (util.hasValue(current_L2_export) && current_L2_export > PHASE_ACTIVE_CURRENT_THRESHOLD)
                                        || (util.hasValue(current_L2_im_ex_sum) && current_L2_im_ex_sum > PHASE_ACTIVE_CURRENT_THRESHOLD)
                                        || (util.hasValue(current_L2_im_ex_diff) && Math.abs(current_L2_im_ex_diff) > PHASE_ACTIVE_CURRENT_THRESHOLD)) {
                                        phases[1] = "a";
                                    }

                                    if ((util.hasValue(current_L3_import) && current_L3_import > PHASE_ACTIVE_CURRENT_THRESHOLD)
                                        || (util.hasValue(current_L3_export) && current_L3_export > PHASE_ACTIVE_CURRENT_THRESHOLD)
                                        || (util.hasValue(current_L3_im_ex_sum) && current_L3_im_ex_sum > PHASE_ACTIVE_CURRENT_THRESHOLD)
                                        || (util.hasValue(current_L3_im_ex_diff) && Math.abs(current_L3_im_ex_diff) > PHASE_ACTIVE_CURRENT_THRESHOLD)) {
                                        phases[2] = "a";
                                    }
                                }

                                let phase_variant = {
                                    "?": "dark",
                                    "d": "dark",
                                    "c": "primary",
                                    "a": "success",
                                };

                                let phase_icon = {
                                    "?": <HelpCircle/>,
                                    "d": <ZapOff/>,
                                    "c": <Zap/>,
                                    "a": <Zap/>,
                                };

                                let meter_is_resettable = false;
                                for (let value_id in state.values_by_id[meter_slot]) {
                                    let path = METER_VALUE_INFOS[parseInt(value_id)].tree_path;
                                    meter_is_resettable ||= path[path.length - 1] == "resettable";
                                }

                                let input_time: h.JSX.Element;
                                let input_date: h.JSX.Element;
                                const last_reset = API.get_unchecked(`meters/${meter_slot}/last_reset`);
                                if (!last_reset || !last_reset.last_reset) {
                                    input_time = <InputText class="form-control-sm" value={__("meters.content.never")}/>
                                    input_date = <InputText class="form-control-sm" value={__("meters.content.never")}/>
                                } else if (last_reset.last_reset * 1000 < 1000000000) {
                                    input_time = <InputText class="form-control-sm" value={__("meters.content.last_reset_counter")(last_reset.last_reset)}/>
                                    input_date = <InputText class="form-control-sm" value={__("meters.content.last_reset_unknown")}/>
                                } else {
                                    input_date = <InputDate className={"form-control-sm"} date={new Date(last_reset.last_reset * 1000)}/>
                                    input_time = <InputTime className={"form-control-sm"} date={new Date(last_reset.last_reset * 1000)}/>
                                }

                                let meter_reset_row: ComponentChild[] = !meter_is_resettable ? [] : [
                                    <FormRow label={__("meters.content.last_reset")} small>
                                        <div class="row mx-n1 mx-xl-n3">
                                            <div class="col-sm-4 px-1 px-xl-3">
                                                {input_date}
                                            </div>
                                            <div class="col-sm-4 px-1 px-xl-3">
                                                {input_time}
                                            </div>
                                            <div class="col-sm-4 px-1 px-xl-3">
                                                <Button size="sm" className="form-control" variant="danger" style="height: calc(1.5em + .5rem + 2px);" onClick={async () => {
                                                    const modal = util.async_modal_ref.current;
                                                    if (!await modal.show({
                                                            title: __("meters.content.reset_modal"),
                                                            body: __("meters.content.reset_modal_body")(get_meter_name(state.configs_table, meter_slot)),
                                                            no_text: __("meters.content.reset_modal_abort"),
                                                            yes_text: __("meters.content.reset_modal_confirm"),
                                                            no_variant: "secondary",
                                                            yes_variant: "danger"
                                                        }))
                                                        return;

                                                    API.call_unchecked(`meters/${meter_slot}/reset`, null, __("meters.content.reset_failed"))
                                                    }}>{__("meters.content.reset")}</Button>
                                            </div>
                                        </div>
                                    </FormRow>
                                ]

                                let allValues = METER_VALUE_ORDER.filter((order) => order.ids.filter((id) => util.hasValue(state.values_by_id[meter_slot][id])).length > 0)
                                    .map((order) => order.group ?
                                        <FormRow label={translate_unchecked(`meters.content.group_${order.group}`)} label_muted={util.joinNonEmpty("; ", [translate_unchecked(`meters.content.group_${order.group}_muted`), order.phases])} small={true}>
                                            <div class="row mx-n1 mx-xl-n3">
                                            {order.ids.map((id) =>
                                                <div class="col-sm-4 px-1 px-xl-3">
                                                    <OutputFloat
                                                        value={util.hasValue(state.values_by_id[meter_slot][id]) ? this.state.values_by_id[meter_slot][id] : null}
                                                        digits={METER_VALUE_INFOS[id].digits}
                                                        scale={0}
                                                        unit={METER_VALUE_INFOS[id].unit}
                                                        small={true}
                                                        class={util.hasValue(state.values_by_id[meter_slot][id]) ? (highlighted_value_ids.indexOf(id) >= 0 ? "input-indicator input-indicator-primary" : undefined) : "input-indicator input-indicator-warning"}
                                                    />
                                                </div>)}
                                            </div>
                                        </FormRow>
                                        : <FormRow label={translate_unchecked(`meters.content.value_${order.ids[0]}`)} label_muted={translate_unchecked(`meters.content.value_${order.ids[0]}_muted`)} small={true}>
                                            <div class="row mx-n1 mx-xl-n3">
                                                <div class="col-sm-4 px-1 px-xl-3">
                                                    <OutputFloat
                                                        value={this.state.values_by_id[meter_slot][order.ids[0]]}
                                                        digits={METER_VALUE_INFOS[order.ids[0]].digits}
                                                        scale={0}
                                                        unit={METER_VALUE_INFOS[order.ids[0]].unit}
                                                        small={true}
                                                        class={util.hasValue(state.values_by_id[meter_slot][order.ids[0]]) ? (highlighted_value_ids.indexOf(order.ids[0]) >= 0 ? "input-indicator input-indicator-primary" : undefined) : "input-indicator input-indicator-warning"}
                                                    />
                                                </div>
                                            </div>
                                        </FormRow>);

                                if (allValues.length == 0) {
                                    allValues = [<div class="form-group row"><span class="col-12">{__("meters.content.detailed_values_none")}</span></div>];
                                }

                                let extraValue = meter_reset_row;
                                if (config_plugins[config[0]]?.get_extra_rows)
                                    extraValue = extraValue.concat(<Fragment key={`extra_rows_${config[0]}`}>{config_plugins[config[0]].get_extra_rows(meter_slot)}</Fragment>);

                                extraValue = extraValue.concat(allValues);

                                return {
                                    columnValues: [
                                        <span class="row mx-n1 align-items-center"><span class="col-auto px-1"><Button className="mr-2" size="sm"
                                            onClick={() => {
                                                this.setState({extraShow: state.extraShow.map((show, i) => meter_slot == i ? !show : show)});
                                            }}>
                                            <ChevronRight {...{id:`meter-${meter_slot}-chevron`, class: state.extraShow[meter_slot] ? "rotated-chevron" : "unrotated-chevron"} as any}/>
                                            </Button></span><span class="col px-1">{get_meter_name(state.configs_table, meter_slot)}</span></span>,
                                        util.hasValue(power) ? util.toLocaleFixed(power, 0) + " W" : undefined,
                                        util.hasValue(energy_import) ? util.toLocaleFixed(energy_import, 3) + " kWh" : undefined,
                                        util.hasValue(energy_export) ? util.toLocaleFixed(energy_export, 3) + " kWh" : undefined,
                                        util.compareArrays(phases, ["?", "?", "?"]) ? undefined : <ButtonGroup>
                                            {phases.map((phase) =>
                                                <Button disabled size="sm" variant={phase_variant[phase]}>
                                                    {phase_icon[phase]}
                                                </Button>
                                            )}
                                        </ButtonGroup>
                                    ],
                                    extraShow: this.state.extraShow[meter_slot],
                                    extraFieldName: __("meters.content.detailed_values"),
                                    extraValue: extraValue,
                                    fieldWithBox: [true, true, true, true, false],
                                    editTitle: __("meters.content.edit_meter_title"),
                                    onEditShow: async () => {
                                        let config_plugin = config_plugins[config[0]];
                                        if (!config_plugin) {
                                            console.log("No config plugin available for meter type", config[0]);
                                            this.setState({editMeterSlot: meter_slot, editMeter: [0, null]});
                                            return;
                                        }
                                        this.setState({editMeterSlot: meter_slot, editMeter: config_plugin.clone_config(config)});
                                    },
                                    onEditGetChildren: () => {
                                        let slots: [string, string][] = [];
                                        let classes: [string, string][] = [];

                                        for (let free_meter_slot = 0; free_meter_slot < METERS_SLOTS; ++free_meter_slot) {
                                            if (state.configs_table[free_meter_slot][0] == MeterClassID.None || free_meter_slot == meter_slot) {
                                                slots.push([free_meter_slot.toString(), free_meter_slot.toString()]);
                                            }
                                        }

                                        for (let meter_class in config_plugins) {
                                            classes.push([meter_class.toString(), config_plugins[meter_class].name])
                                        }

                                        let rows: ComponentChild[] = [<>
                                            <FormRow label={__("meters.content.edit_meter_slot")}>
                                                <InputSelect
                                                    items={slots}
                                                    onValue={(v) => this.setState({editMeterSlot: parseInt(v)})}
                                                    value={state.editMeterSlot.toString()} />
                                            </FormRow>
                                            <FormRow label={__("meters.content.edit_meter_class")}>
                                                <InputSelect
                                                    placeholder={__("meters.content.edit_meter_class_select")}
                                                    items={classes}
                                                    onValue={(v) => {
                                                        let meter_class = parseInt(v);

                                                        if (meter_class != state.editMeter[0]) {
                                                            if (meter_class == MeterClassID.None) {
                                                                this.setState({editMeter: [MeterClassID.None, null]});
                                                            }
                                                            else {
                                                                this.setState({editMeter: config_plugins[meter_class].new_config()});
                                                            }
                                                        }
                                                    }}
                                                    value={state.editMeter[0].toString()} />
                                            </FormRow>
                                        </>]

                                        if (state.editMeter[0] != MeterClassID.None) {
                                            rows = rows.concat(<Fragment key={`edit_children_${state.editMeter[0]}`}>{config_plugins[state.editMeter[0]].get_edit_children(state.editMeter, (meter_config) => this.setState({editMeter: meter_config}))}</Fragment>);
                                        }

                                        return rows;
                                    },
                                    onEditSubmit: async () => {
                                        this.setState({configs_table: {...state.configs_table, [meter_slot]: [MeterClassID.None, null], [state.editMeterSlot]: state.editMeter}});
                                        this.setDirty(true);
                                    },
                                    onEditHide: async () => {
                                        if (state.editMeter[0] != MeterClassID.None && config_plugins[state.editMeter[0]].hide) {
                                            config_plugins[state.editMeter[0]].hide();
                                        }
                                    },
                                    onRemoveClick: async () => {
                                        this.setState({configs_table: {...state.configs_table, [meter_slot]: [MeterClassID.None, null]}});
                                        this.setDirty(true);
                                    }
                                }
                            })}
                            addEnabled={active_meter_slots.length < METERS_SLOTS}
                            addTitle={__("meters.content.add_meter_title")}
                            addMessage={__("meters.content.add_meter_prefix") + active_meter_slots.length + __("meters.content.add_meter_infix") + METERS_SLOTS + __("meters.content.add_meter_suffix")}
                            onAddShow={async () => {
                                let addMeterSlot = null;

                                // Don't auto-select charger meter slot
                                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                                    if (meter_slot != state.charger_meter_slot && state.configs_table[meter_slot][0] == MeterClassID.None) {
                                        addMeterSlot = meter_slot;
                                        break;
                                    }
                                }

                                this.setState({addMeterSlot: addMeterSlot, addMeter: [MeterClassID.None, null]});
                            }}
                            onAddGetChildren={() => {
                                let slots: [string, string][] = [];
                                let classes: [string, string][] = [];

                                for (let free_meter_slot = 0; free_meter_slot < METERS_SLOTS; ++free_meter_slot) {
                                    if (state.configs_table[free_meter_slot][0] == MeterClassID.None) {
                                        slots.push([free_meter_slot.toString(), free_meter_slot.toString()]);
                                    }
                                }

                                for (let meter_class in config_plugins) {
                                    classes.push([meter_class.toString(), config_plugins[meter_class].name])
                                }

                                let rows: ComponentChild[] = [
                                    <FormRow label={__("meters.content.add_meter_slot")}>
                                        <InputSelect
                                            placeholder={__("meters.content.add_meter_slot_select")}
                                            items={slots}
                                            onValue={(v) => this.setState({addMeterSlot: parseInt(v)})}
                                            value={state.addMeterSlot !== null ? state.addMeterSlot.toString() : null}
                                            required />
                                    </FormRow>,
                                    <FormRow label={__("meters.content.add_meter_class")}>
                                        <InputSelect
                                            placeholder={__("meters.content.add_meter_class_select")}
                                            items={classes}
                                            onValue={(v) => {
                                                let meter_class = parseInt(v);

                                                if (meter_class != state.addMeter[0]) {
                                                    if (meter_class == MeterClassID.None) {
                                                        this.setState({addMeter: [MeterClassID.None, null]});
                                                    }
                                                    else {
                                                        this.setState({addMeter: config_plugins[meter_class].new_config()});
                                                    }
                                                }
                                            }}
                                            value={this.state.addMeter[0].toString()}
                                            required />
                                    </FormRow>
                                ];

                                if (state.addMeter[0] != MeterClassID.None) {
                                    rows = rows.concat(<Fragment key={`edit_children_${state.editMeter[0]}`}>{config_plugins[state.addMeter[0]].get_edit_children(state.addMeter, (meter_config) => this.setState({addMeter: meter_config}))}</Fragment>);
                                }

                                return rows;
                            }}
                            onAddSubmit={async () => {
                                this.setState({configs_table: {...state.configs_table, [state.addMeterSlot]: state.addMeter}});
                                this.setDirty(true);
                            }}
                            onAddHide={async () => {
                                if (state.addMeter[0] != MeterClassID.None && config_plugins[state.addMeter[0]].hide) {
                                    await config_plugins[state.addMeter[0]].hide();
                                }
                            }}
                            />
                    </div>
                </ConfigForm>
            </SubPage>
        );
    }
}

interface MetersStatusState {
    charger_meter_slot: number,
    meter_configs: {[meter_slot: number]: MeterConfig},
}

function get_meter_name(meter_configs: {[meter_slot: number]: MeterConfig}, meter_slot: number) {
    let meter_name = __("meters.script.meter")(util.hasValue(meter_slot) ? meter_slot : '?');

    if (util.hasValue(meter_slot) && util.hasValue(meter_configs) && util.hasValue(meter_configs[meter_slot]) && util.hasValue(meter_configs[meter_slot][1])) {
        meter_name = meter_configs[meter_slot][1].display_name;
    }

    return meter_name;
}

export class MetersStatus extends Component<{}, MetersStatusState> {
    on_mount: () => void;
    on_mount_pending = false;
    uplot_loader_ref = createRef();
    uplot_wrapper_ref = createRef();

    constructor() {
        super();

        this.state = {
            charger_meter_slot: null,
            meter_configs: {},
        } as any;

        util.addApiEventListener_unchecked('evse/meter_config', () => {
            let config = API.get_unchecked('evse/meter_config');

            this.setState({
                charger_meter_slot: config.slot,
            });
        });

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            util.addApiEventListener_unchecked(`meters/${meter_slot}/config`, () => {
                let config = API.get_unchecked(`meters/${meter_slot}/config`);

                this.setState((prevState) => ({
                    meter_configs: {
                        ...prevState.meter_configs,
                        [meter_slot]: config
                    }
                }));
            });
        }
    }

    set_on_mount(on_mount: () => void) {
        this.on_mount = on_mount;

        if (this.on_mount_pending && this.on_mount) {
            this.on_mount_pending = false;

            this.on_mount();
        }
    }

    render() {
        if (!util.render_allowed() || !API.hasFeature("meters") || API.hasFeature("energy_manager") || this.state.charger_meter_slot === null)
            return <StatusSection name="meters" />;

        let value_ids = API.get_unchecked(`meters/${this.state.charger_meter_slot}/value_ids`);

        // The meters feature is set if any meter is connected. This includes a WARP Charger Smart
        // with external meter for PV excess charging. But in this case the status plot should not
        // be shown, because it will never have values. Only show the plot if the shown meter has
        // declared its value IDs to indicate that it actually is alive.
        if (value_ids.length == 0)
            return <StatusSection name="meters" />;

        let values = API.get_unchecked(`meters/${this.state.charger_meter_slot}/values`);
        let power: number = undefined;
        let power_idx = get_meter_power_index(value_ids);

        if (power_idx >= 0 && values && values.length > 0) {
            power = values[power_idx];
        }

        return (
            <StatusSection name="meters">
                <FormRow label={__("meters.status.power_history")}>
                    <div class="card pl-1 pb-1">
                        <div style="position: relative;"> {/* this plain div is neccessary to make the size calculation stable in safari. without this div the height continues to grow */}
                            <UplotLoader ref={this.uplot_loader_ref}
                                        show={true}
                                        marker_class={'h4'}
                                        no_data={__("meters.content.no_data")}
                                        loading={__("meters.content.loading")} >
                                <UplotWrapper ref={this.uplot_wrapper_ref}
                                            class="status-meters-chart"
                                            sub_page="status"
                                            color_cache_group="meters.default"
                                            show={true}
                                            on_mount={() => {
                                                if (this.on_mount) {
                                                    this.on_mount();
                                                }
                                                else {
                                                    this.on_mount_pending = true;
                                                }
                                            }}
                                            legend_time_label={__("meters.script.time")}
                                            legend_time_with_seconds={false}
                                            aspect_ratio={3}
                                            x_height={50}
                                            x_padding_factor={0}
                                            x_include_date={true}
                                            y_min={0}
                                            y_max={1500}
                                            y_unit="W"
                                            y_label={__("meters.script.power") + " [Watt]"}
                                            y_digits={0} />
                            </UplotLoader>
                        </div>
                    </div>
                </FormRow>
                <FormRow label={__("meters.status.current_power")} label_muted={get_meter_name(this.state.meter_configs, this.state.charger_meter_slot)}>
                    <OutputFloat value={power} digits={0} scale={0} unit="W" maxFractionalDigitsOnPage={0} maxUnitLengthOnPage={1}/>
                </FormRow>
            </StatusSection>
        );
    }
}

export function init() {
    let result = plugins_init();

    for (let plugins of result) {
        for (let i in plugins) {
            if (config_plugins[i]) {
                console.log('Meter: Overwriting class ID ' + i);
            }

            config_plugins[i] = plugins[i];
        }
    }
}

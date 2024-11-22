/* esp32-firmware
 * Copyright (C) 2022 Olaf Lüke <olaf@tinkerforge.com>
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

//#include "module_available.inc"

import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { METERS_SLOTS } from "../../build";
import { h, createRef, Fragment, Component, RefObject } from "preact";
import { __, translate_unchecked } from "../../ts/translation";
import { PageHeader } from "../../ts/components/page_header";
import { InputDate } from "../../ts/components/input_date";
import { InputMonth } from "../../ts/components/input_month";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { OutputFloat } from "../../ts/components/output_float";
import { SubPage } from "../../ts/components/sub_page";
import { FormSeparator } from "../../ts/components/form_separator";
import { UplotLoader } from "../../ts/components/uplot_loader";
import { UplotWrapper, UplotData, UplotPath } from "../../ts/components/uplot_wrapper_2nd";
import { UplotFlagsWrapper } from "../../ts/components/uplot_wrapper_3rd";
//#if MODULE_DAY_AHEAD_PRICES_AVAILABLE
import { get_price_from_index } from "../day_ahead_prices/main";
//#endif
import uPlot from "uplot";
import { MeterConfig } from "../meters/types";
import { NavbarItem } from "../../ts/components/navbar_item";
import { StatusSection } from "../../ts/components/status_section";
import { PieChart } from "react-feather";

export function EMEnergyAnalysisNavbar() {
    return <NavbarItem name="em_energy_analysis" module="em_common" title={__("em_energy_analysis.navbar.em_energy_analysis")} symbol={<PieChart />} />;
}

const UPDATE_RETRY_DELAY = 500; // ms

interface CachedData {
    update_timestamp: number;
    use_timestamp: number;
}

type CachedUplotData = CachedData & UplotData;

interface Wallbox5minData extends CachedData {
    empty: boolean;
    complete: boolean;
    flags: number[/*timestamp_slot*/]; // v1: bit 0-2 = charger state
                                       // v2: bit 0-2 = charger state, bit 3-4 = phases
    power: number[/*timestamp_slot*/]; // W
}

interface WallboxDailyData extends CachedData {
    empty: boolean;
    complete: boolean;
    energy: number[/*timestamp_slot*/]; // kWh
}

interface EnergyManager5minData extends CachedData {
    empty: boolean;
    complete: boolean;
    flags: number[/*timestamp_slot*/]; // v1: bit 0 = 1p/3p, bit 1-2 = inputs, bit 3 = output
                                       // v2: bit 0-3 = inputs, bit 4-5 = SG ready, bit 6-7 = relays
    power: number[/*meter_slot*/][/*timestamp_slot*/]; // W
    power_empty: boolean[/*meter_slot*/];
    price: number[/*timestamp_slot*/]; // mct/kWh
    price_empty: boolean;
}

interface EnergyManagerDailyData extends CachedData {
    empty: boolean;
    complete: boolean;
    energy_import: number[/*meter_slot*/][/*timestamp_slot*/]; // kWh
    energy_export: number[/*meter_slot*/][/*timestamp_slot*/]; // kWh
    price_min: number[/*timestamp_slot*/]; // ct/kWh
    price_avg: number[/*timestamp_slot*/]; // ct/kWh
    price_max: number[/*timestamp_slot*/]; // ct/kWh
}

const wb_state_names: {[id: number]: () => string} = {
    0: () => __("em_energy_analysis.content.state_not_connected"),
    1: () => __("em_energy_analysis.content.state_waiting_for_charge_release"),
    2: () => __("em_energy_analysis.content.state_ready_to_charge"),
    3: () => __("em_energy_analysis.content.state_charging"),
    4: () => __("em_energy_analysis.content.state_error"),
    5: () => __("em_energy_analysis.content.state_charging_single_phase"),
    6: () => __("em_energy_analysis.content.state_charging_three_phase"),
};

const wb_state_strokes: {[id: number]: string} = {
    0: 'rgb(  0, 123, 255)',
    1: 'rgb(255, 193,   7)',
    2: 'rgb( 13, 202, 240)',
    3: 'rgb( 40, 167,  69)',
    4: 'rgb(220,  53,  69)',
    5: 'rgb( 40, 167,  69)', // FIXME
    6: 'rgb( 40, 167,  69)', // FIXME
};

const wb_state_fills: {[id: number]: string} = {
    0: 'rgb(  0, 123, 255, 0.66)',
    1: 'rgb(255, 193,   7, 0.66)',
    2: 'rgb( 13, 202, 240, 0.66)',
    3: 'rgb( 40, 167,  69, 0.66)',
    4: 'rgb(220,  53,  69, 0.66)',
    5: 'rgb( 40, 167,  69, 0.66)', // FIXME
    6: 'rgb( 40, 167,  69, 0.66)', // FIXME
};

const em_phase_names: {[id: number]: () => string} = {
    0: () => __("em_energy_analysis.content.state_single_phase"),
    1: () => __("em_energy_analysis.content.state_three_phase"),
};

const em_phase_strokes: {[id: number]: string} = {
    0: 'rgb(108, 117, 125)',
    1: 'rgb( 40, 167,  69)',
};

const em_phase_fills: {[id: number]: string} = {
    0: 'rgb(108, 117, 125, 0.66)',
    1: 'rgb( 40, 167,  69, 0.66)',
};

const em_input_names: {[id: number]: () => string} = {
    0: () => __("em_energy_analysis.content.state_input_low"),
    1: () => __("em_energy_analysis.content.state_input_high"),
};

const em_input_strokes: {[id: number]: string} = {
    0: 'rgb(108, 117, 125)',
    1: 'rgb( 40, 167,  69)',
};

const em_input_fills: {[id: number]: string} = {
    0: 'rgb(108, 117, 125, 0.66)',
    1: 'rgb( 40, 167,  69, 0.66)',
};

const em_sg_ready1_strokes_active_closed: {[id: number]: string} = {
    0: 'rgb(108, 117, 125)',
    1: 'rgb(220,  53,  69)',
};

const em_sg_ready1_fills_active_closed: {[id: number]: string} = {
    0: 'rgb(108, 117, 125, 0.66)',
    1: 'rgb(220,  53,  69, 0.66)',
};

const em_sg_ready1_strokes_active_open: {[id: number]: string} = {
    0: 'rgb(220,  53,  69)',
    1: 'rgb(108, 117, 125)',
};

const em_sg_ready1_fills_active_open: {[id: number]: string} = {
    0: 'rgb(220,  53,  69, 0.66)',
    1: 'rgb(108, 117, 125, 0.66)',
};

const em_sg_ready2_strokes_active_closed: {[id: number]: string} = {
    0: 'rgb(108, 117, 125)',
    1: 'rgb( 40, 167,  69)',
};

const em_sg_ready2_fills_active_closed: {[id: number]: string} = {
    0: 'rgb(108, 117, 125, 0.66)',
    1: 'rgb( 40, 167,  69, 0.66)',
};

const em_sg_ready2_strokes_active_open: {[id: number]: string} = {
    0: 'rgb( 40, 167,  69)',
    1: 'rgb(108, 117, 125)',
};

const em_sg_ready2_fills_active_open: {[id: number]: string} = {
    0: 'rgb( 40, 167,  69, 0.66)',
    1: 'rgb(108, 117, 125, 0.66)',
};

const em_relay_names: {[id: number]: () => string} = {
    0: () => __("em_energy_analysis.content.state_relay_open"),
    1: () => __("em_energy_analysis.content.state_relay_closed"),
};

const em_relay_strokes: {[id: number]: string} = {
    0: 'rgb(108, 117, 125)',
    1: 'rgb( 40, 167,  69)',
};

const em_relay_fills: {[id: number]: string} = {
    0: 'rgb(108, 117, 125, 0.66)',
    1: 'rgb( 40, 167,  69, 0.66)',
};

function resolve_translation(names: {[id: number]: () => string}) {
    let result: {[id: number]: string} = {};

    for (let key in names) {
        result[key] = names[key]();
    }

    return result;
}

interface EMEnergyAnalysisStatusState {
    force_render: number,
}

function get_meter_name(meter_configs: {[meter_slot: number]: MeterConfig}, meter_slot: number) {
    let meter_name = __("em_energy_analysis.script.meter")(util.hasValue(meter_slot) ? meter_slot : '?');

    if (util.hasValue(meter_slot) && util.hasValue(meter_configs) && util.hasValue(meter_configs[meter_slot]) && util.hasValue(meter_configs[meter_slot][1])) {
        meter_name = meter_configs[meter_slot][1].display_name;
    }

    return meter_name;
}

function get_wallbox_state(flags: number) {
    let state = flags & 0b111;

    if (state == 3) { // charging
        let phases = (flags >> 3) & 0b11;

        if (phases == 1) {
            state = 5; // charging single-phase
        }
        else if (phases == 3) {
            state = 6; // charging three-phase
        }
    }

    return state;
}

export class EMEnergyAnalysisStatus extends Component<{}, EMEnergyAnalysisStatusState> {
    on_mount: () => void;
    on_mount_pending = false;
    uplot_loader_ref = createRef();
    uplot_wrapper_ref = createRef();

    constructor() {
        super();

        this.state = {
            force_render: 0,
        } as any;

        util.addApiEventListener('info/modules', () => {
            if (!API.hasFeature('energy_manager')) {
                console.log("Energy Analysis: energy_manager feature not available");
                return;
            }

            this.setState({force_render: Date.now()});
        });
    }

    set_on_mount(on_mount: () => void) {
        this.on_mount = on_mount;

        if (this.on_mount_pending && this.on_mount) {
            this.on_mount_pending = false;

            this.on_mount();
        }
    }

    render() {
        if (!util.render_allowed() || !API.hasFeature("energy_manager"))
            return <StatusSection name="em_energy_analysis" />;

        return <StatusSection name="em_energy_analysis">
            <FormRow label={__("em_energy_analysis.status.power_history")}>
                <div class="card pl-1 pb-1">
                    <div style="position: relative;"> {/* this plain div is neccessary to make the size calculation stable in safari. without this div the height continues to grow */}
                        <UplotLoader ref={this.uplot_loader_ref}
                                        show={true}
                                        marker_class={'h4'}
                                        no_data={__("em_energy_analysis.content.no_data")}
                                        loading={__("em_energy_analysis.content.loading")}>
                            <UplotWrapper ref={this.uplot_wrapper_ref}
                                            class="em-energy-analysis-status-chart"
                                            sub_page="status"
                                            color_cache_group="em_energy_analysis.status"
                                            show={true}
                                            on_mount={() => {
                                                if (this.on_mount) {
                                                    this.on_mount();
                                                }
                                                else {
                                                    this.on_mount_pending = true;
                                                }
                                            }}
                                            legend_time_label={__("em_energy_analysis.script.time_5min")}
                                            legend_time_with_minutes={true}
                                            aspect_ratio={3}
                                            x_height={35}
                                            x_format={{hour: '2-digit', minute: '2-digit'}}
                                            x_padding_factor={0}
                                            x_include_date={false}
                                            y_min={0}
                                            y_max={1500}
                                            y_unit={"W"}
                                            y_label={__("em_energy_analysis.script.power") + " [W]"}
                                            y_digits={0}
                                            padding={[null, 15, null, 5]} />
                        </UplotLoader>
                    </div>
                </div>
            </FormRow>
        </StatusSection>;
    }
}

interface EMEnergyAnalysisProps {
    status_ref: RefObject<EMEnergyAnalysisStatus>;
}

interface EMEnergyAnalysisState {
    force_render: number,
    meter_slot_status: number,
    meter_configs: {[meter_slot: number]: MeterConfig};
    data_type: '5min'|'daily';
    current_5min_date: Date;
    current_daily_date: Date;
    wallbox_5min_cache_energy_total: {[id: number]: {[id: string]: number[/*timestamp_slot*/]}};
    wallbox_daily_cache_energy_total: {[id: number]: {[id: string]: number}};
    energy_manager_5min_cache_energy_total: {[id: string]: {import: number[/*meter_slot*/][/*timestamp_slot*/], export: number[/*meter_slot*/][/*timestamp_slot*/]}};
    energy_manager_daily_cache_energy_total: {[id: string]: {import: number[/*meter_slot*/], export: number[/*meter_slot*/]}};
    sg_ready1_active_type: number; // 0 = closed, 1 = open
    sg_ready2_active_type: number; // 0 = closed, 1 = open
}

interface Charger {
    uid: number;
    name: string;
}

function array_last<T>(a: Array<T>): T {
    return a[a.length - 1];
}

export class EMEnergyAnalysis extends Component<EMEnergyAnalysisProps, EMEnergyAnalysisState> {
    uplot_sync = uPlot.sync('foobar');
    uplot_loader_5min_ref = createRef();
    uplot_wrapper_5min_flags_ref = createRef();
    uplot_wrapper_5min_power_ref = createRef();
    uplot_legend_div_5min_power_ref = createRef();
    uplot_legend_div_5min_flags_ref = createRef();
    uplot_loader_daily_ref = createRef();
    uplot_wrapper_daily_ref = createRef();
    uplot_update_timeout: number = null;
    uplot_5min_flags_cache: {[id: string]: CachedUplotData} = {};
    uplot_5min_power_cache: {[id: string]: CachedUplotData} = {};
    uplot_5min_status_cache: {[id: string]: CachedUplotData} = {};
    uplot_5min_cache_initalized: boolean = false;
    uplot_daily_cache: {[id: string]: CachedUplotData} = {};
    uplot_daily_cache_initalized: boolean = false;
    wallbox_5min_cache: {[id: number]: { [id: string]: Wallbox5minData}} = {};
    wallbox_daily_cache: {[id: string]: { [id: string]: WallboxDailyData}} = {};
    energy_manager_5min_cache: {[id: string]: EnergyManager5minData} = {};
    energy_manager_daily_cache: {[id: string]: EnergyManagerDailyData} = {};
    cache_limit = 100;
    chargers: Charger[] = [];
//#if MODULE_DAY_AHEAD_PRICES_AVAILABLE
    day_ahead_prices_update_timestamp = 0;
    day_ahead_prices_first_timestamp: number = null;
    day_ahead_prices_last_timestamp: number = null;
//#endif

    constructor() {
        super();

        let current_5min_date: Date = new Date();

        current_5min_date.setHours(0);
        current_5min_date.setMinutes(0);
        current_5min_date.setSeconds(0);
        current_5min_date.setMilliseconds(0);

        let current_daily_date: Date = new Date();

        current_daily_date.setDate(1);
        current_daily_date.setHours(0);
        current_daily_date.setMinutes(0);
        current_daily_date.setSeconds(0);
        current_daily_date.setMilliseconds(0);

        this.state = {
            force_render: 0,
            meter_slot_status: 0,
            data_type: '5min',
            current_5min_date: current_5min_date,
            current_daily_date: current_daily_date,
            wallbox_5min_cache_energy_total: {},
            wallbox_daily_cache_energy_total: {},
            energy_manager_5min_cache_energy_total: {},
            energy_manager_daily_cache_energy_total: {},
            sg_ready1_active_type: null,
            sg_ready2_active_type: null,
        } as any;

        util.addApiEventListener('info/modules', () => {
            if (!API.hasFeature('energy_manager')) {
                console.log("Energy Analysis: energy_manager feature not available");
                return;
            }

            this.update_current_5min_cache();
            this.update_current_daily_cache();

            this.setState({force_render: Date.now()});
        });

        util.addApiEventListener('charge_manager/state', () => {
            if (!API.hasFeature('energy_manager'))
                return;

            let state = API.get('charge_manager/state');
            let chargers: Charger[] = [];

            for (let charger of state.chargers) {
                if (charger.u > 0) {
                    chargers.push({uid: charger.u, name: charger.n});
                }
            }

            if (this.chargers.length != chargers.length) {
                this.chargers = chargers;
                this.reload_wallbox_cache();
            }
            else {
                for (let i = 0; i < this.chargers.length; ++i) {
                    if (this.chargers[i].uid != chargers[i].uid || this.chargers[i].name != chargers[i].name) {
                        this.chargers = chargers;
                        this.reload_wallbox_cache();
                        break;
                    }
                }
            }
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

        util.addApiEventListener('energy_manager/history_wallbox_5min_changed', () => {
            let changed = API.get('energy_manager/history_wallbox_5min_changed');
            let subcache = this.wallbox_5min_cache[changed.uid];
            let reload_subcache: boolean = false;

            if (!subcache) {
                // got changed event without having this UID cached before
                reload_subcache = true;
            } else {
                let key = `${changed.year}-${changed.month}-${changed.day}`;
                let data = subcache[key];

                if (!data) {
                    // got changed event without having this day cached before
                    reload_subcache = true;
                }
                else {
                    let timestamp_slot = Math.floor((changed.hour * 60 + changed.minute) / 5);

                    if (timestamp_slot > 0 && data.flags[timestamp_slot - 1] === null) {
                        // previous slot has no data. was a previous update event missed?
                        delete subcache[key];
                        reload_subcache = true;
                    }
                    else {
                        data.update_timestamp = Date.now();
                        data.empty = false;
                        data.complete = timestamp_slot == 287; // update for last 5min slot of the day
                        data.flags[timestamp_slot] = changed.flags;
                        data.power[timestamp_slot] = changed.power;

                        this.schedule_uplot_update();
                    }
                }
            }

            if (reload_subcache) {
                this.update_wallbox_5min_cache(changed.uid, new Date(changed.year, changed.month - 1, changed.day))
                    .then((success: boolean) => {
                        if (success) {
                            this.schedule_uplot_update();
                        }
                    });
            }
        });

        util.addApiEventListener('energy_manager/history_energy_manager_5min_changed', () => {
            let changed = API.get('energy_manager/history_energy_manager_5min_changed');
            let key = `${changed.year}-${changed.month}-${changed.day}`;
            let data = this.energy_manager_5min_cache[key];
            let reload_cache: boolean = false;

            if (!data) {
                // got changed event without having this day cached before
                reload_cache = true;
            } else {
                let timestamp_slot = Math.floor((changed.hour * 60 + changed.minute) / 5);

                if (timestamp_slot > 0 && data.flags[timestamp_slot - 1] === null) {
                    // previous slot has no data. was a previous update event missed?
                    delete this.energy_manager_5min_cache[key];
                    reload_cache = true;
                }
                else {
                    data.update_timestamp = Date.now();
                    data.empty = false;
                    data.complete = timestamp_slot == 287; // update for last 5min slot of the day
                    data.flags[timestamp_slot] = changed.flags;

                    for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                        data.power[meter_slot][timestamp_slot] = changed.power[meter_slot];

                        if (data.power[meter_slot][timestamp_slot] !== null) {
                            data.power_empty[meter_slot] = false;
                        }
                    }

                    data.price[timestamp_slot] = changed.price;

                    if (data.price[timestamp_slot] !== null) {
                        data.price_empty = false;
                    }

                    this.schedule_uplot_update();
                }
            }

            if (reload_cache) {
                this.update_energy_manager_5min_cache(new Date(changed.year, changed.month - 1, changed.day))
                    .then((success: boolean) => {
                        if (success) {
                            this.schedule_uplot_update();
                        }
                    });
            }
        });

        util.addApiEventListener('energy_manager/history_wallbox_daily_changed', () => {
            let changed = API.get('energy_manager/history_wallbox_daily_changed');
            let subcache = this.wallbox_daily_cache[changed.uid];
            let reload_subcache: boolean = false;

            if (!subcache) {
                // got changed event without having this UID cached before
                reload_subcache = true;
            } else {
                let key = `${changed.year}-${changed.month}`;
                let data = subcache[key];

                if (!data) {
                    // got changed event without having this month cached before
                    reload_subcache = true;
                }
                else {
                    let timestamp_slot = changed.day - 1;

                    if (timestamp_slot > 0 && data.energy[timestamp_slot - 1] == null) {
                        // previous slot has no data. was a previous update event missed?
                        delete subcache[key];
                        reload_subcache = true;
                    }
                    else {
                        data.update_timestamp = Date.now();
                        data.empty = false;
                        // FIXME: how to set complete = true here?
                        data.energy[timestamp_slot] = changed.energy;
                    }

                    this.schedule_uplot_update();
                }
            }

            if (reload_subcache) {
                this.update_wallbox_daily_cache(changed.uid, new Date(changed.year, changed.month - 1))
                    .then((success: boolean) => {
                        if (success) {
                            this.schedule_uplot_update();
                        }
                    });
            }
        });

        util.addApiEventListener('energy_manager/history_energy_manager_daily_changed', () => {
            let changed = API.get('energy_manager/history_energy_manager_daily_changed');
            let key = `${changed.year}-${changed.month}`;
            let data = this.energy_manager_daily_cache[key];
            let reload_cache: boolean = false;

            if (!data) {
                // got changed event without having this day cached before
                reload_cache = true;
            } else {
                let timestamp_slot = changed.day - 1;

                data.update_timestamp = Date.now();
                data.empty = false;
                // FIXME: how to set complete = true here?

                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                    data.energy_import[meter_slot][timestamp_slot] = changed.energy_import[meter_slot];
                    data.energy_export[meter_slot][timestamp_slot] = changed.energy_export[meter_slot];
                }

                data.price_min[timestamp_slot] = changed.price_min;
                data.price_avg[timestamp_slot] = changed.price_avg;
                data.price_max[timestamp_slot] = changed.price_max;

                this.schedule_uplot_update();
            }

            if (reload_cache) {
                this.update_energy_manager_daily_cache(new Date(changed.year, changed.month - 1))
                    .then((success: boolean) => {
                        if (success) {
                            this.schedule_uplot_update();
                        }
                    });
            }
        });

//#if MODULE_DAY_AHEAD_PRICES_AVAILABLE
        util.addApiEventListener('day_ahead_prices/prices', () => {
            this.update_day_ahead_prices_cache();
        });

        util.addApiEventListener('day_ahead_prices/config', () => {
            this.update_day_ahead_prices_cache();
        });
//#endif

        util.addApiEventListener("power_manager/config", () => {
            let config = API.get("power_manager/config");

            this.setState({meter_slot_status: config.meter_slot_grid_power});
        });

        util.addApiEventListener_unchecked('heating/config', () => {
            let config = API.get_unchecked("heating/config");

            this.update_current_5min_cache();
            this.update_current_daily_cache();

            this.setState({
                sg_ready1_active_type: config.sg_ready_blocking_active_type,
                sg_ready2_active_type: config.sg_ready_extended_active_type

            });
        });
    }

    componentDidMount() {
        this.props.status_ref.current.set_on_mount(() => this.update_status_uplot());
    }

    date_to_5min_key(date: Date) {
        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let day: number = date.getDate();

        if (isNaN(year)) {
            return null;
        }

        return `${year}-${month}-${day}`;
    }

    date_to_daily_key(date: Date) {
        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;

        if (isNaN(year)) {
            return null;
        }

        return`${year}-${month}`;
    }

    expire_cache(cache: {[id: string]: CachedData}) {
        let keys = Object.keys(cache);

        while (keys.length > this.cache_limit) {
            let oldest_key: string = keys[0];
            let oldest_timestamp: number = cache[oldest_key].use_timestamp;

            for (let key of keys) {
                if (oldest_timestamp > cache[key].use_timestamp) {
                    oldest_key = key;
                    oldest_timestamp = cache[oldest_key].use_timestamp;
                }
            }

            delete cache[oldest_key];

            keys = Object.keys(cache);
        }
    }

    update_uplot_5min_power_cache(date: Date) {
        let key = this.date_to_5min_key(date);

        if (key == null) {
            return;
        }

        let uplot_data = this.uplot_5min_power_cache[key];
        let needs_update = false;
        let now = Date.now();

        if (!uplot_data) {
            needs_update = true;
        }
        else {
            let energy_manager_data = this.energy_manager_5min_cache[key];
            let date_timestamp = date.getTime() / 1000;

            if ((energy_manager_data && uplot_data.update_timestamp < energy_manager_data.update_timestamp)
//#if MODULE_DAY_AHEAD_PRICES_AVAILABLE
             || (this.day_ahead_prices_first_timestamp !== null
              && date_timestamp >= this.day_ahead_prices_first_timestamp
              && date_timestamp < this.day_ahead_prices_last_timestamp
              && uplot_data.update_timestamp < this.day_ahead_prices_update_timestamp)
//#endif
            ) {
                needs_update = true;
            }

            if (!needs_update) {
                for (let charger of this.chargers) {
                    if (this.wallbox_5min_cache[charger.uid]) {
                        let wallbox_data = this.wallbox_5min_cache[charger.uid][key];

                        if (wallbox_data && uplot_data.update_timestamp < wallbox_data.update_timestamp) {
                            needs_update = true;
                            break;
                        }
                    }
                }
            }
        }

        if (!needs_update) {
            // cache is valid
            uplot_data.use_timestamp = now;
            return;
        }

        uplot_data = {
            update_timestamp: now,
            use_timestamp: now,
            keys: [null],
            names: [null],
            values: [null],
            extras: [null],
            stacked: [null],
            paths: [null],
            extra_names: [null],
            y_axes: [null],
        };

        let timestamp_slot_count: number = 0;
        let energy_manager_data = this.energy_manager_5min_cache[key];
        let price = undefined;
        let price_empty = true;

        if (energy_manager_data) {
            for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                if (!energy_manager_data.power_empty[meter_slot]) {
                    timestamp_slot_count = Math.max(timestamp_slot_count, energy_manager_data.power[meter_slot].length);

                    uplot_data.keys.push('em_power_' + meter_slot);
                    uplot_data.names.push(get_meter_name(this.state.meter_configs, meter_slot));
                    uplot_data.values.push(energy_manager_data.power[meter_slot]);
                    uplot_data.extras.push(null);
                    uplot_data.stacked.push(false);
                    uplot_data.paths.push(UplotPath.Line);
                    uplot_data.extra_names.push(null);
                    uplot_data.y_axes.push('y');
                }
            }

            price = energy_manager_data.price.concat(); // copy before adding future prices
            price_empty = energy_manager_data.price_empty;

            if (!price_empty) {
                timestamp_slot_count = Math.max(timestamp_slot_count, energy_manager_data.price.length);
            }
        }

//#if MODULE_DAY_AHEAD_PRICES_AVAILABLE
        let dap_prices = API.get("day_ahead_prices/prices");

        if (dap_prices.prices.length > 0) {
            let dap_config = API.get("day_ahead_prices/config");
            let resolution_multiplier = dap_prices.resolution == 0 ? 15 : 60;
            let grid_costs_and_taxes_and_supplier_markup = dap_config.grid_costs_and_taxes / 1000.0 + dap_config.supplier_markup / 1000.0;
            let first_timestamp = dap_prices.first_date * 60;
            let last_timestamp = first_timestamp + dap_prices.prices.length * 60 * resolution_multiplier; // exclusive range
            let timestamp_base = date.getTime() / 1000;

            timestamp_slot_count = Math.max(timestamp_slot_count, 24 * 12);

            if (price === undefined) {
                price = new Array(timestamp_slot_count);
                price.fill(null);
            }

            while (price.length < timestamp_slot_count) {
                price.push(null);
            }

            for (let timestamp_slot = timestamp_slot_count - 1; timestamp_slot >= 0; --timestamp_slot) {
                let timestamp = timestamp_base + timestamp_slot * 300; // seconds

                if (timestamp < first_timestamp || timestamp >= last_timestamp) {
                    continue; // no future price data available
                }

                if (price[timestamp_slot] !== null) {
                    break; // history price data available
                }

                let index = Math.floor((timestamp - first_timestamp) / (60 * resolution_multiplier));

                price[timestamp_slot] = get_price_from_index(index) / 1000.0 + grid_costs_and_taxes_and_supplier_markup;
                price_empty = false;
            }
        }
//#endif

        if (!price_empty) {
            uplot_data.keys.push('em_price');
            uplot_data.names.push(__("em_energy_analysis.script.price"));
            uplot_data.values.push(price);
            uplot_data.extras.push(null);
            uplot_data.stacked.push(false);
            uplot_data.paths.push(UplotPath.Step);
            uplot_data.extra_names.push(null);
            uplot_data.y_axes.push('y2');
        }

        for (let charger of this.chargers) {
            if (this.wallbox_5min_cache[charger.uid]) {
                let wallbox_data = this.wallbox_5min_cache[charger.uid][key];

                if (wallbox_data && !wallbox_data.empty) {
                    timestamp_slot_count = Math.max(timestamp_slot_count, wallbox_data.power.length);

                    let state = new Array(wallbox_data.flags.length);

                    for (let i = 0; i < wallbox_data.flags.length; ++i) {
                        if (wallbox_data.flags[i] === null) {
                            state[i] = null;
                        }
                        else {
                            state[i] = get_wallbox_state(wallbox_data.flags[i]);
                        }
                    }

                    uplot_data.keys.push('wb_power_' + charger.uid);
                    uplot_data.names.push(charger.name);
                    uplot_data.values.push(wallbox_data.power);
                    uplot_data.extras.push(state);
                    uplot_data.stacked.push(true);
                    uplot_data.paths.push(UplotPath.Line);
                    uplot_data.extra_names.push(resolve_translation(wb_state_names));
                    uplot_data.y_axes.push('y');
                }
            }
        }

        let timestamps: number[] = new Array(timestamp_slot_count);
        let base = date.getTime() / 1000;

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            timestamps[timestamp_slot] = base + timestamp_slot * 300;
        }

        uplot_data.values[0] = timestamps;

        this.uplot_5min_power_cache[key] = uplot_data;
        this.expire_cache(this.uplot_5min_power_cache);
    }

    update_uplot_5min_flags_cache(date: Date) {
        let key = this.date_to_5min_key(date);

        if (key == null) {
            return;
        }

        let uplot_data = this.uplot_5min_flags_cache[key];
        let needs_update = false;
        let now = Date.now();

        if (!uplot_data) {
            needs_update = true;
        }
        else {
            let energy_manager_data = this.energy_manager_5min_cache[key];

            if (energy_manager_data && uplot_data.update_timestamp < energy_manager_data.update_timestamp) {
                needs_update = true;
            }

            if (!needs_update) {
                for (let charger of this.chargers) {
                    if (this.wallbox_5min_cache[charger.uid]) {
                        let wallbox_data = this.wallbox_5min_cache[charger.uid][key];

                        if (wallbox_data && uplot_data.update_timestamp < wallbox_data.update_timestamp) {
                            needs_update = true;
                            break;
                        }
                    }
                }
            }
        }

        if (!needs_update) {
            // cache is valid
            uplot_data.use_timestamp = now;
            return;
        }

        uplot_data = {
            update_timestamp: now,
            use_timestamp: now,
            keys: [null],
            names: [null],
            values: [null],
            paths: [null],
            value_names: [null],
            value_strokes: [null],
            value_fills: [null],
            default_visibilty: [null],
        };

        let timestamp_slot_count: number = 0;
        let energy_manager_data = this.energy_manager_5min_cache[key];

        if (energy_manager_data && !energy_manager_data.empty) {
            timestamp_slot_count = Math.max(timestamp_slot_count, energy_manager_data.flags.length);

            if (API.hasModule("em_v1")) {
                let phase = new Array(energy_manager_data.flags.length);
                let input3 = new Array(energy_manager_data.flags.length);
                let input4 = new Array(energy_manager_data.flags.length);
                let relay = new Array(energy_manager_data.flags.length);

                for (let i = 0; i < energy_manager_data.flags.length; ++i) {
                    if (energy_manager_data.flags[i] === null) {
                        phase[i] = null;
                        input3[i] = null;
                        input4[i] = null;
                        relay[i] = null;
                    }
                    else {
                        if (i > 0 && energy_manager_data.flags[i - 1] !== null && (energy_manager_data.flags[i] & 0b0001) == (energy_manager_data.flags[i - 1] & 0b0001)) {
                            phase[i] = undefined;
                        }
                        else {
                            phase[i] = energy_manager_data.flags[i] & 0b0001;
                        }

                        if (i > 0 && energy_manager_data.flags[i - 1] !== null && (energy_manager_data.flags[i] & 0b0010) == (energy_manager_data.flags[i - 1] & 0b0010)) {
                            input3[i] = undefined;
                        }
                        else {
                            input3[i] = (energy_manager_data.flags[i] & 0b0010) >> 1;
                        }

                        if (i > 0 && energy_manager_data.flags[i - 1] !== null && (energy_manager_data.flags[i] & 0b0100) == (energy_manager_data.flags[i - 1] & 0b0100)) {
                            input4[i] = undefined;
                        }
                        else {
                            input4[i] = (energy_manager_data.flags[i] & 0b0100) >> 2;
                        }

                        if (i > 0 && energy_manager_data.flags[i - 1] !== null && (energy_manager_data.flags[i] & 0b1000) == (energy_manager_data.flags[i - 1] & 0b1000)) {
                            relay[i] = undefined;
                        }
                        else {
                            relay[i] = (energy_manager_data.flags[i] & 0b1000) >> 3;
                        }
                    }
                }

                uplot_data.keys.push('em_phase');
                uplot_data.names.push(__("em_energy_analysis.content.state_phase"));
                uplot_data.values.push(phase);
                uplot_data.paths.push(UplotPath.Line);
                uplot_data.value_names.push(resolve_translation(em_phase_names));
                uplot_data.value_strokes.push(em_phase_strokes);
                uplot_data.value_fills.push(em_phase_fills);
                uplot_data.default_visibilty.push(true);

                uplot_data.keys.push('em_input3');
                uplot_data.names.push(__("em_energy_analysis.content.state_input3"));
                uplot_data.values.push(input3);
                uplot_data.paths.push(UplotPath.Line);
                uplot_data.value_names.push(resolve_translation(em_input_names));
                uplot_data.value_strokes.push(em_input_strokes);
                uplot_data.value_fills.push(em_input_fills);
                uplot_data.default_visibilty.push(false);

                uplot_data.keys.push('em_input4');
                uplot_data.names.push(__("em_energy_analysis.content.state_input4"));
                uplot_data.values.push(input4);
                uplot_data.paths.push(UplotPath.Line);
                uplot_data.value_names.push(resolve_translation(em_input_names));
                uplot_data.value_strokes.push(em_input_strokes);
                uplot_data.value_fills.push(em_input_fills);
                uplot_data.default_visibilty.push(false);

                uplot_data.keys.push('em_relay');
                uplot_data.names.push(__("em_energy_analysis.content.state_relay"));
                uplot_data.values.push(relay);
                uplot_data.paths.push(UplotPath.Line);
                uplot_data.value_names.push(resolve_translation(em_relay_names));
                uplot_data.value_strokes.push(em_relay_strokes);
                uplot_data.value_fills.push(em_relay_fills);
                uplot_data.default_visibilty.push(false);
            }
            else if (API.hasModule("em_v2")) {
                let ios = [];

                for (let k = 0; k < 8; ++k) {
                    ios.push(new Array(energy_manager_data.flags.length));
                }

                for (let i = 0; i < energy_manager_data.flags.length; ++i) {
                    if (energy_manager_data.flags[i] === null) {
                        for (let k = 0; k < 8; ++k) {
                            ios[k][i] = null;
                        }
                    }
                    else {
                        for (let k = 0; k < 8; ++k) {
                            if (i > 0 && energy_manager_data.flags[i - 1] !== null && (energy_manager_data.flags[i] & (1 << k)) == (energy_manager_data.flags[i - 1] & (1 << k))) {
                                ios[k][i] = undefined;
                            }
                            else {
                                ios[k][i] = (energy_manager_data.flags[i] & (1 << k)) != 0 ? 1 : 0;
                            }
                        }
                    }
                }

                for (let k = 0; k < 4; ++k) {
                    uplot_data.keys.push(`em_input${k + 1}`);
                    uplot_data.names.push(translate_unchecked(`em_energy_analysis.content.state_input${k + 1}`));
                    uplot_data.values.push(ios[k]);
                    uplot_data.paths.push(UplotPath.Line);
                    uplot_data.value_names.push(resolve_translation(em_input_names));
                    uplot_data.value_strokes.push(em_input_strokes);
                    uplot_data.value_fills.push(em_input_fills);
                    uplot_data.default_visibilty.push(false);
                }

                let em_sg_ready1_names: {[id: number]: string} = {
                    0: __("em_energy_analysis.content.state_sg_ready_open"),
                    1: __("em_energy_analysis.content.state_sg_ready_closed"),
                };

                let em_sg_ready1_strokes = em_sg_ready1_strokes_active_closed;
                let em_sg_ready1_fills = em_sg_ready1_fills_active_closed;

                if (this.state.sg_ready1_active_type === 0 /* closed */) {
                    em_sg_ready1_names[0] += " / " + __("em_energy_analysis.content.state_sg_ready_inactive");
                    em_sg_ready1_names[1] += " / " + __("em_energy_analysis.content.state_sg_ready_active");
                }
                else if (this.state.sg_ready1_active_type === 1 /* open */) {
                    em_sg_ready1_names[0] += " / " + __("em_energy_analysis.content.state_sg_ready_active");
                    em_sg_ready1_names[1] += " / " + __("em_energy_analysis.content.state_sg_ready_inactive");
                    em_sg_ready1_strokes = em_sg_ready1_strokes_active_open;
                    em_sg_ready1_fills = em_sg_ready1_fills_active_open;
                }

                uplot_data.keys.push(`em_sg_ready1`);
                uplot_data.names.push(translate_unchecked(`em_energy_analysis.content.state_sg_ready1`));
                uplot_data.values.push(ios[4]);
                uplot_data.paths.push(UplotPath.Line);
                uplot_data.value_names.push(em_sg_ready1_names);
                uplot_data.value_strokes.push(em_sg_ready1_strokes);
                uplot_data.value_fills.push(em_sg_ready1_fills);
                uplot_data.default_visibilty.push(true);

                let em_sg_ready2_names: {[id: number]: string} = {
                    0: __("em_energy_analysis.content.state_sg_ready_open"),
                    1: __("em_energy_analysis.content.state_sg_ready_closed"),
                };

                let em_sg_ready2_strokes = em_sg_ready2_strokes_active_closed;
                let em_sg_ready2_fills = em_sg_ready2_fills_active_closed;

                if (this.state.sg_ready2_active_type === 0 /* closed */) {
                    em_sg_ready2_names[0] += " / " + __("em_energy_analysis.content.state_sg_ready_inactive");
                    em_sg_ready2_names[1] += " / " + __("em_energy_analysis.content.state_sg_ready_active");
                }
                else if (this.state.sg_ready2_active_type === 1 /* open */) {
                    em_sg_ready2_names[0] += " / " + __("em_energy_analysis.content.state_sg_ready_active");
                    em_sg_ready2_names[1] += " / " + __("em_energy_analysis.content.state_sg_ready_inactive");
                    em_sg_ready2_strokes = em_sg_ready2_strokes_active_open;
                    em_sg_ready2_fills = em_sg_ready2_fills_active_open;
                }

                uplot_data.keys.push(`em_sg_ready2`);
                uplot_data.names.push(translate_unchecked(`em_energy_analysis.content.state_sg_ready2`));
                uplot_data.values.push(ios[5]);
                uplot_data.paths.push(UplotPath.Line);
                uplot_data.value_names.push(em_sg_ready2_names);
                uplot_data.value_strokes.push(em_sg_ready2_strokes);
                uplot_data.value_fills.push(em_sg_ready2_fills);
                uplot_data.default_visibilty.push(true);

                for (let k = 0; k < 2; ++k) {
                    uplot_data.keys.push(`em_relay${k + 1}`);
                    uplot_data.names.push(translate_unchecked(`em_energy_analysis.content.state_relay${k + 1}`));
                    uplot_data.values.push(ios[6 + k]);
                    uplot_data.paths.push(UplotPath.Line);
                    uplot_data.value_names.push(resolve_translation(em_relay_names));
                    uplot_data.value_strokes.push(em_relay_strokes);
                    uplot_data.value_fills.push(em_relay_fills);
                    uplot_data.default_visibilty.push(false);
                }
            }
        }

        for (let charger of this.chargers) {
            if (this.wallbox_5min_cache[charger.uid]) {
                let wallbox_data = this.wallbox_5min_cache[charger.uid][key];

                if (wallbox_data && !wallbox_data.empty) {
                    timestamp_slot_count = Math.max(timestamp_slot_count, wallbox_data.flags.length);

                    let state = new Array(wallbox_data.flags.length);

                    for (let i = 0; i < wallbox_data.flags.length; ++i) {
                        if (wallbox_data.flags[i] === null) {
                            state[i] = null;
                        }
                        else if (i > 0 && wallbox_data.flags[i - 1] !== null && (wallbox_data.flags[i] & 0b11111) == (wallbox_data.flags[i - 1] & 0b11111)) {
                            state[i] = undefined; // charger state (bit 0-2) and phases (bit 3-4) didn't change
                        }
                        else {
                            state[i] = get_wallbox_state(wallbox_data.flags[i]);
                        }
                    }

                    uplot_data.keys.push('wb_state_' + charger.uid);
                    uplot_data.names.push(charger.name);
                    uplot_data.values.push(state);
                    uplot_data.paths.push(UplotPath.Line);
                    uplot_data.value_names.push(resolve_translation(wb_state_names));
                    uplot_data.value_strokes.push(wb_state_strokes);
                    uplot_data.value_fills.push(wb_state_fills);
                    uplot_data.default_visibilty.push(true);
                }
            }
        }

        let timestamps: number[] = new Array(timestamp_slot_count);
        let base = date.getTime() / 1000;

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            timestamps[timestamp_slot] = base + timestamp_slot * 300;
        }

        uplot_data.values[0] = timestamps;

        this.uplot_5min_flags_cache[key] = uplot_data;
        this.expire_cache(this.uplot_5min_flags_cache);
    }

    update_uplot_5min_status_cache(date: Date) {
        let key = this.date_to_5min_key(date);

        if (key == null) {
            return;
        }

        let uplot_data = this.uplot_5min_status_cache[key];
        let needs_update = false;
        let now = Date.now();

        if (!uplot_data) {
            needs_update = true;
        }
        else {
            let energy_manager_data = this.energy_manager_5min_cache[key];

            if (energy_manager_data && uplot_data.update_timestamp < energy_manager_data.update_timestamp) {
                needs_update = true;
            }
        }

        if (!needs_update) {
            // cache is valid
            uplot_data.use_timestamp = now;
            return;
        }

        uplot_data = {
            update_timestamp: now,
            use_timestamp: now,
            keys: [null],
            names: [null],
            values: [null],
            stacked: [null],
            paths: [null],
        };

        let timestamp_slot_count: number = 0;
        let energy_manager_data = this.energy_manager_5min_cache[key];

        if (energy_manager_data) {
            for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                if (!energy_manager_data.power_empty[meter_slot]) {
                    timestamp_slot_count = Math.max(timestamp_slot_count, energy_manager_data.power[this.state.meter_slot_status].length)

                    uplot_data.keys.push('em_power_' + meter_slot);
                    uplot_data.names.push(get_meter_name(this.state.meter_configs, meter_slot));
                    uplot_data.values.push(energy_manager_data.power[meter_slot]);
                    uplot_data.stacked.push(false);
                    uplot_data.paths.push(UplotPath.Line);
                }
            }
        }

        let timestamps: number[] = new Array(timestamp_slot_count);
        let base = date.getTime() / 1000;

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            timestamps[timestamp_slot] = base + timestamp_slot * 5 * 60;
        }

        uplot_data.values[0] = timestamps;

        this.uplot_5min_status_cache[key] = uplot_data;
        this.expire_cache(this.uplot_5min_status_cache);
    }

    update_uplot_daily_cache(date: Date) {
        let key = this.date_to_daily_key(date);

        if (key == null) {
            return;
        }

        let previous_key = this.date_to_daily_key(new Date(date.getFullYear(), date.getMonth() - 1));
        let uplot_data = this.uplot_daily_cache[key];
        let needs_update = false;
        let now = Date.now();

        if (!uplot_data) {
            needs_update = true;
        }
        else {
            let energy_manager_data = this.energy_manager_daily_cache[key];

            if (energy_manager_data && uplot_data.update_timestamp < energy_manager_data.update_timestamp) {
                needs_update = true;
            }

            if (!needs_update) {
                let energy_manager_previous_data = this.energy_manager_daily_cache[previous_key];

                if (energy_manager_previous_data && uplot_data.update_timestamp < energy_manager_previous_data.update_timestamp) {
                    needs_update = true;
                }

                if (!needs_update) {
                    for (let charger of this.chargers) {
                        if (this.wallbox_daily_cache[charger.uid]) {
                            let wallbox_data = this.wallbox_daily_cache[charger.uid][key];

                            if (wallbox_data && uplot_data.update_timestamp < wallbox_data.update_timestamp) {
                                needs_update = true;
                                break;
                            }

                            let wallbox_previous_data = this.wallbox_daily_cache[charger.uid][previous_key];

                            if (wallbox_previous_data && uplot_data.update_timestamp < wallbox_previous_data.update_timestamp) {
                                needs_update = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (!needs_update) {
            // cache is valid
            uplot_data.use_timestamp = now;
            return;
        }

        uplot_data = {
            update_timestamp: now,
            use_timestamp: now,
            keys: [null],
            names: [null],
            values: [null],
            stacked: [null],
            filled: [null],
            paths: [null],
            y_axes: [null],
        };

        let timestamp_slot_count: number = 0;
        let energy_manager_data = this.energy_manager_daily_cache[key];
        let energy_manager_previous_data = this.energy_manager_daily_cache[previous_key];
        let energy_import_5min_total = new Array(METERS_SLOTS);
        let energy_import_daily_total = new Array(METERS_SLOTS);
        let energy_export_5min_total = new Array(METERS_SLOTS);
        let energy_export_daily_total = new Array(METERS_SLOTS);

        if (energy_manager_data && !energy_manager_data.empty) {
            for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                // energy_import and energy_export have the same length
                timestamp_slot_count = Math.max(timestamp_slot_count, energy_manager_data.energy_import[meter_slot].length);

                let energy_import = new Array(energy_manager_data.energy_import[meter_slot].length);
                let last_energy_import: number = null;

                if (energy_manager_previous_data) {
                    last_energy_import = array_last(energy_manager_previous_data.energy_import[meter_slot]);
                }

                energy_import_5min_total[meter_slot] = new Array(energy_manager_data.energy_import[meter_slot].length);
                energy_import_daily_total[meter_slot] = null;

                for (let timestamp_slot = 0; timestamp_slot < energy_manager_data.energy_import[meter_slot].length; ++timestamp_slot) {
                    if (energy_manager_data.energy_import[meter_slot][timestamp_slot] !== null && last_energy_import !== null) {
                        energy_import[timestamp_slot] = energy_manager_data.energy_import[meter_slot][timestamp_slot] - last_energy_import;

                        if (energy_import_daily_total[meter_slot] === null) {
                            energy_import_daily_total[meter_slot] = energy_import[timestamp_slot];
                        }
                        else {
                            energy_import_daily_total[meter_slot] += energy_import[timestamp_slot];
                        }
                    }
                    else {
                        energy_import[timestamp_slot] = null;
                    }

                    energy_import_5min_total[meter_slot][timestamp_slot] = energy_import[timestamp_slot];
                    last_energy_import = energy_manager_data.energy_import[meter_slot][timestamp_slot];
                }

                // FIXME: only show meter #0 for now, because there is no good way to show the other
                //        meters. they would have to be shown side by side, but there is no space
                if (meter_slot == 0) {
                    uplot_data.keys.push('em_energy_import_' + meter_slot);
                    uplot_data.names.push(`${get_meter_name(this.state.meter_configs, meter_slot)} (${__("em_energy_analysis.content.import")})`);
                    uplot_data.values.push(energy_import);
                    uplot_data.stacked.push(false);
                    uplot_data.filled.push(true);
                    uplot_data.paths.push(UplotPath.Bar);
                    uplot_data.y_axes.push('y');
                }

                let energy_export = new Array(energy_manager_data.energy_export[meter_slot].length);
                let last_energy_export: number = null;

                if (energy_manager_previous_data) {
                    last_energy_export = array_last(energy_manager_previous_data.energy_export[meter_slot]);
                }

                energy_export_5min_total[meter_slot] = new Array(energy_manager_data.energy_export[meter_slot].length);
                energy_export_daily_total[meter_slot] = null;

                for (let timestamp_slot = 0; timestamp_slot < energy_manager_data.energy_export[meter_slot].length; ++timestamp_slot) {
                    if (energy_manager_data.energy_export[meter_slot][timestamp_slot] !== null && last_energy_export !== null) {
                        energy_export[timestamp_slot] = energy_manager_data.energy_export[meter_slot][timestamp_slot] - last_energy_export;

                        if (energy_export[timestamp_slot] > 0) {
                            energy_export[timestamp_slot] = -energy_export[timestamp_slot];
                        }

                        if (energy_export_daily_total[meter_slot] === null) {
                            energy_export_daily_total[meter_slot] = energy_export[timestamp_slot];
                        }
                        else {
                            energy_export_daily_total[meter_slot] += energy_export[timestamp_slot];
                        }
                    }
                    else {
                        energy_export[timestamp_slot] = null;
                    }

                    energy_export_5min_total[meter_slot][timestamp_slot] = energy_export[timestamp_slot];
                    last_energy_export = energy_manager_data.energy_export[meter_slot][timestamp_slot];
                }

                // FIXME: only show meter #0 for now, because there is no good way to show the other
                //        meters. they would have to be shown side by side, but there is no space
                if (meter_slot == 0) {
                    uplot_data.keys.push('em_energy_export_' + meter_slot);
                    uplot_data.names.push(`${get_meter_name(this.state.meter_configs, meter_slot)} (${__("em_energy_analysis.content.export")})`);
                    uplot_data.values.push(energy_export);
                    uplot_data.stacked.push(false);
                    uplot_data.filled.push(true);
                    uplot_data.paths.push(UplotPath.Bar);
                    uplot_data.y_axes.push('y');
                }
            }

            // FIXME: combine min/avg/max into a band instead of three single line plots
            uplot_data.keys.push('em_price_min');
            uplot_data.names.push(__("em_energy_analysis.script.price_min"));
            uplot_data.values.push(energy_manager_data.price_min);
            uplot_data.stacked.push(false);
            uplot_data.filled.push(false);
            uplot_data.paths.push(UplotPath.Line);
            uplot_data.y_axes.push('y2');

            uplot_data.keys.push('em_price_avg');
            uplot_data.names.push(__("em_energy_analysis.script.price_avg"));
            uplot_data.values.push(energy_manager_data.price_avg);
            uplot_data.stacked.push(false);
            uplot_data.filled.push(false);
            uplot_data.paths.push(UplotPath.Line);
            uplot_data.y_axes.push('y2');

            uplot_data.keys.push('em_price_max');
            uplot_data.names.push(__("em_energy_analysis.script.price_max"));
            uplot_data.values.push(energy_manager_data.price_max);
            uplot_data.stacked.push(false);
            uplot_data.filled.push(false);
            uplot_data.paths.push(UplotPath.Line);
            uplot_data.y_axes.push('y2');

            this.setState((prevState) => ({
                energy_manager_5min_cache_energy_total: {
                    ...prevState.energy_manager_5min_cache_energy_total,
                    [key]: {
                        import: energy_import_5min_total,
                        export: energy_export_5min_total
                    }
                },
                energy_manager_daily_cache_energy_total: {
                    ...prevState.energy_manager_daily_cache_energy_total,
                    [key]: {
                        import: energy_import_daily_total,
                        export: energy_export_daily_total
                    }
                }
            }));
        }

        for (let charger of this.chargers) {
            if (this.wallbox_daily_cache[charger.uid]) {
                let wallbox_data = this.wallbox_daily_cache[charger.uid][key];
                let wallbox_previous_data = this.wallbox_daily_cache[charger.uid][previous_key];

                if (wallbox_data && !wallbox_data.empty) {
                    timestamp_slot_count = Math.max(timestamp_slot_count, wallbox_data.energy.length);

                    let energy = new Array(wallbox_data.energy.length);
                    let last_energy: number = null;
                    let energy_5min_total = new Array(wallbox_data.energy.length);
                    let energy_daily_total: number = null;

                    if (wallbox_previous_data) {
                        last_energy = array_last(wallbox_previous_data.energy);
                    }

                    for (let timestamp_slot = 0; timestamp_slot < wallbox_data.energy.length; ++timestamp_slot) {
                        if (wallbox_data.energy[timestamp_slot] !== null && last_energy !== null) {
                            energy[timestamp_slot] = wallbox_data.energy[timestamp_slot] - last_energy;

                            if (energy_daily_total === null) {
                                energy_daily_total = energy[timestamp_slot];
                            }
                            else {
                                energy_daily_total += energy[timestamp_slot];
                            }
                        }
                        else {
                            energy[timestamp_slot] = null;
                        }

                        energy_5min_total[timestamp_slot] = energy[timestamp_slot];
                        last_energy = wallbox_data.energy[timestamp_slot];
                    }

                    uplot_data.keys.push('wb_energy_' + charger.uid);
                    uplot_data.names.push(charger.name);
                    uplot_data.values.push(energy);
                    uplot_data.stacked.push(true);
                    uplot_data.filled.push(true);
                    uplot_data.paths.push(UplotPath.Bar);
                    uplot_data.y_axes.push('y');

                    this.setState((prevState) => ({
                        wallbox_5min_cache_energy_total: {
                            ...prevState.wallbox_5min_cache_energy_total,
                            [charger.uid]: {
                                ...(prevState.wallbox_5min_cache_energy_total[charger.uid] || {}), [key]: energy_5min_total
                            }
                        },
                        wallbox_daily_cache_energy_total: {
                            ...prevState.wallbox_daily_cache_energy_total,
                            [charger.uid]: {
                                ...(prevState.wallbox_daily_cache_energy_total[charger.uid] || {}), [key]: energy_daily_total
                            }
                        }
                    }));
                }
            }
        }

        let timestamps: number[] = new Array(timestamp_slot_count);
        let base = date.getTime() / 1000;

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            timestamps[timestamp_slot] = base + timestamp_slot * 24 * 60 * 60;
        }

        uplot_data.values[0] = timestamps;

        this.uplot_daily_cache[key] = uplot_data;
        this.expire_cache(this.uplot_daily_cache);
    }

    async update_wallbox_5min_cache_all(date: Date) {
        let all: Promise<boolean>[] = [];

        for (let charger of this.chargers) {
            all.push(this.update_wallbox_5min_cache(charger.uid, date));
        }

        let result = await (Promise<boolean[]>).all(all);

        for (let success of result) {
            if (!success) {
                return false;
            }
        }

        return true;
    }

    timestamp_to_5min_slot(timestamp: number) {
        return Math.floor(timestamp / (5 * 60 * 1000));
    }

    set_5min_loading() {
        if (this.uplot_wrapper_5min_flags_ref.current) {
            this.uplot_wrapper_5min_flags_ref.current.set_loading();
        }

        if (this.uplot_wrapper_5min_power_ref.current) {
            this.uplot_wrapper_5min_power_ref.current.set_loading();
        }

        if (this.uplot_loader_5min_ref.current) {
            this.uplot_loader_5min_ref.current.set_loading();
        }
    }

    async update_wallbox_5min_cache(uid: number, date: Date) {
        let now = Date.now();

        if (date.getTime() > now) {
            return true;
        }

        let key = this.date_to_5min_key(date);

        if (key == null) {
            return true;
        }

        if (this.wallbox_5min_cache[uid]
            && this.wallbox_5min_cache[uid][key]
            && (this.wallbox_5min_cache[uid][key].complete
                || this.timestamp_to_5min_slot(this.wallbox_5min_cache[uid][key].update_timestamp) >= this.timestamp_to_5min_slot(now))) {
            // cache is valid
            this.wallbox_5min_cache[uid][key].use_timestamp = now;
            return true;
        }

        this.set_5min_loading();

        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let day: number = date.getDate();
        let response: string = '';

        try {
            response = await (await util.put('energy_manager/history_wallbox_5min', {uid: uid, year: year, month: month, day: day})).text();
        } catch (e) {
            console.log('Energy Analysis: Could not get wallbox 5min data: ' + e);
            return false;
        }

        // reload now timestamp, because of the await call before, the previous value is
        // old and might result in wrong cache ordering because an uplot cache update could
        // have occurred during the await call
        now = Date.now();

        let payload = JSON.parse(response);
        let timestamp_slot_count = payload.length / 2;
        let data: Wallbox5minData = {
            update_timestamp: now,
            use_timestamp: now,
            empty: true,
            complete: key < this.date_to_5min_key(new Date(now)),
            flags: new Array(timestamp_slot_count),
            power: new Array(timestamp_slot_count),
        };

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            data.flags[timestamp_slot] = payload[timestamp_slot * 2];
            data.power[timestamp_slot] = payload[timestamp_slot * 2 + 1];

            if (data.flags[timestamp_slot] !== null) {
                data.empty = false;
            }
        }

        if (!this.wallbox_5min_cache[uid]) {
            this.wallbox_5min_cache[uid] = {};
        }

        this.wallbox_5min_cache[uid][key] = data;
        this.expire_cache(this.wallbox_5min_cache[uid]);

        return true;
    }

    async update_energy_manager_5min_cache(date: Date) {
        let now = Date.now();

        if (date.getTime() > now) {
            return true;
        }

        let key = this.date_to_5min_key(date);

        if (key == null) {
            return true;
        }

        if (this.energy_manager_5min_cache[key]
            && (this.energy_manager_5min_cache[key].complete
                || this.timestamp_to_5min_slot(this.energy_manager_5min_cache[key].update_timestamp) >= this.timestamp_to_5min_slot(now))) {
            // cache is valid
            this.energy_manager_5min_cache[key].use_timestamp = now;
            return true;
        }

        this.set_5min_loading();

        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let day: number = date.getDate();
        let response: string = '';

        try {
            response = await (await util.put('energy_manager/history_energy_manager_5min', {year: year, month: month, day: day})).text();
        } catch (e) {
            console.log('Energy Analysis: Could not get energy manager 5min data: ' + e);
            return false;
        }

        // reload now timestamp, because of the await call before, the previous value is
        // old and might result in wrong cache ordering because an uplot cache update could
        // have occurred during the await call
        now = Date.now();

        let payload = JSON.parse(response);
        let timestamp_slot_count = payload.length / 9;
        let data: EnergyManager5minData = {
            update_timestamp: now,
            use_timestamp: now,
            empty: true,
            complete: key < this.date_to_5min_key(new Date(now)),
            flags: new Array(timestamp_slot_count),
            power: new Array(METERS_SLOTS),
            power_empty: new Array(METERS_SLOTS),
            price: new Array(timestamp_slot_count),
            price_empty: true,
        };

        data.power_empty.fill(true);

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            data.flags[timestamp_slot] = payload[timestamp_slot * 9];

            if (data.flags[timestamp_slot] !== null) {
                data.empty = false;
            }
        }

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            data.power[meter_slot] = new Array(timestamp_slot_count);

            for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
                data.power[meter_slot][timestamp_slot] = payload[timestamp_slot * 9 + 1 + meter_slot];

                if (data.power[meter_slot][timestamp_slot] !== null) {
                    data.power_empty[meter_slot] = false;
                }
            }
        }

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            data.price[timestamp_slot] = payload[timestamp_slot * 9 + 8];

            if (data.price[timestamp_slot] !== null) {
                data.price_empty = false;
            }
        }

        this.energy_manager_5min_cache[key] = data;
        this.expire_cache(this.energy_manager_5min_cache);

        return true;
    }

//#if MODULE_DAY_AHEAD_PRICES_AVAILABLE
    update_day_ahead_prices_cache() {
        let dap_prices = API.get("day_ahead_prices/prices");

        if (dap_prices.prices.length > 0) {
            let dap_config = API.get("day_ahead_prices/config");
            let resolution_multiplier = dap_prices.resolution == 0 ? 15 : 60;
            this.day_ahead_prices_first_timestamp = dap_prices.first_date * 60;
            this.day_ahead_prices_last_timestamp = this.day_ahead_prices_first_timestamp + dap_prices.prices.length * 60 * resolution_multiplier; // exclusive range
        }
        else {
            this.day_ahead_prices_first_timestamp = null;
            this.day_ahead_prices_last_timestamp = null;
        }

        this.day_ahead_prices_update_timestamp = Date.now();
        this.schedule_uplot_update();
    }
//#endif

    async update_wallbox_daily_cache_all(date: Date) {
        let all: Promise<boolean>[] = [];

        for (let charger of this.chargers) {
            all.push(this.update_wallbox_daily_cache(charger.uid, date));
        }

        let result = await (Promise<boolean[]>).all(all);

        for (let success of result) {
            if (!success) {
                return false;
            }
        }

        return true;
    }

    timestamp_to_daily_slot(timestamp: number) {
        return Math.floor(timestamp / (24 * 60 * 60 * 1000));
    }

    set_daily_loading() {
        if (this.uplot_wrapper_daily_ref.current) {
            this.uplot_wrapper_daily_ref.current.set_loading();
        }

        if (this.uplot_loader_daily_ref.current) {
            this.uplot_loader_daily_ref.current.set_loading();
        }
    }

    async update_wallbox_daily_cache(uid: number, date: Date, update_previous?: boolean) {
        if (update_previous !== false) {
            let previous_date = new Date(date.getFullYear(), date.getMonth() - 1, date.getDate());

            await this.update_wallbox_daily_cache(uid, previous_date, false);
        }

        let now = Date.now();

        if (date.getTime() > now) {
            return true;
        }

        let key = this.date_to_daily_key(date);

        if (key == null) {
            return;
        }

        if (this.wallbox_daily_cache[uid]
            && this.wallbox_daily_cache[uid][key]
            && (this.wallbox_daily_cache[uid][key].complete
                || this.timestamp_to_daily_slot(this.wallbox_daily_cache[uid][key].update_timestamp) >= this.timestamp_to_daily_slot(now))) {
            // cache is valid
            this.wallbox_daily_cache[uid][key].use_timestamp = now;
            return true;
        }

        this.set_daily_loading();

        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let response: string = '';

        try {
            response = await (await util.put('energy_manager/history_wallbox_daily', {uid: uid, year: year, month: month})).text();
        } catch (e) {
            console.log('Energy Analysis: Could not get wallbox daily data: ' + e);
            return false;
        }

        // reload now timestamp, because of the await call before, the previous value is
        // old and might result in wrong cache ordering because an uplot cache update could
        // have orccured during the await call
        now = Date.now();

        let payload = JSON.parse(response);
        let timestamp_slot_count = payload.length;
        let data: WallboxDailyData = {
            update_timestamp: now,
            use_timestamp: now,
            empty: true,
            complete: key < this.date_to_daily_key(new Date(now)),
            energy: new Array(timestamp_slot_count),
        };

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            data.energy[timestamp_slot] = payload[timestamp_slot];

            if (data.energy[timestamp_slot] !== null) {
                data.empty = false;
            }
        }

        if (!this.wallbox_daily_cache[uid]) {
            this.wallbox_daily_cache[uid] = {};
        }

        this.wallbox_daily_cache[uid][key] = data;
        this.expire_cache(this.wallbox_daily_cache[uid]);

        return true;
    }

    async update_energy_manager_daily_cache(date: Date, update_previous?: boolean) {
        if (update_previous !== false) {
            let previous_date = new Date(date.getFullYear(), date.getMonth() - 1, date.getDate());

            await this.update_energy_manager_daily_cache(previous_date, false);
        }

        let now = Date.now();

        if (date.getTime() > now) {
            return true;
        }

        let key = this.date_to_daily_key(date);

        if (key == null) {
            return true;
        }

        if (this.energy_manager_daily_cache[key]
            && (this.energy_manager_daily_cache[key].complete
                || this.timestamp_to_daily_slot(this.energy_manager_daily_cache[key].update_timestamp) >= this.timestamp_to_daily_slot(now))) {
            // cache is valid
            this.energy_manager_daily_cache[key].use_timestamp = now;
            return true;
        }

        this.set_daily_loading();

        let year: number = date.getFullYear();
        let month: number = date.getMonth() + 1;
        let response: string = '';

        try {
            response = await (await util.put('energy_manager/history_energy_manager_daily', {year: year, month: month})).text();
        } catch (e) {
            console.log('Energy Analysis: Could not get energy manager daily data: ' + e);
            return false;
        }

        // reload now timestamp, because of the await call before, the previous value is
        // old and might result in wrong cache ordering because an uplot cache update could
        // have orccured during the await call
        now = Date.now();

        let payload = JSON.parse(response);
        let timestamp_slot_count = payload.length / 17;
        let data: EnergyManagerDailyData = {
            update_timestamp: now,
            use_timestamp: now,
            empty: true,
            complete: key < this.date_to_daily_key(new Date(now)),
            energy_import: new Array(METERS_SLOTS),
            energy_export: new Array(METERS_SLOTS),
            price_min: new Array(timestamp_slot_count),
            price_avg: new Array(timestamp_slot_count),
            price_max: new Array(timestamp_slot_count),
        };

        for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
            data.energy_import[meter_slot] = new Array(timestamp_slot_count);
            data.energy_export[meter_slot] = new Array(timestamp_slot_count);

            for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
                data.energy_import[meter_slot][timestamp_slot] = payload[timestamp_slot * 17 + meter_slot];

                if (data.energy_import[meter_slot][timestamp_slot] !== null) {
                    data.empty = false;
                }

                data.energy_export[meter_slot][timestamp_slot] = payload[timestamp_slot * 17 + 7 + meter_slot];

                if (data.energy_export[meter_slot][timestamp_slot] !== null) {
                    data.empty = false;
                }
            }
        }

        for (let timestamp_slot = 0; timestamp_slot < timestamp_slot_count; ++timestamp_slot) {
            data.price_min[timestamp_slot] = payload[timestamp_slot * 17 + 14];

            if (data.price_min[timestamp_slot] !== null) {
                data.empty = false;
            }

            data.price_avg[timestamp_slot] = payload[timestamp_slot * 17 + 15];

            if (data.price_avg[timestamp_slot] !== null) {
                data.empty = false;
            }

            data.price_max[timestamp_slot] = payload[timestamp_slot * 17 + 16];

            if (data.price_max[timestamp_slot] !== null) {
                data.empty = false;
            }
        }

        this.energy_manager_daily_cache[key] = data;
        this.expire_cache(this.energy_manager_daily_cache);

        return true;
    }

    set_current_5min_date(date: Date) {
        this.setState({current_5min_date: date}, () => {
            this.update_current_5min_cache();
        });
    }

    set_current_daily_date(date: Date) {
        this.setState({current_daily_date: date}, () => {
            this.update_current_daily_cache();
        });
    }

    reload_wallbox_cache() {
        this.wallbox_5min_cache = {};
        this.wallbox_daily_cache = {};

        this.update_wallbox_5min_cache_all(this.state.current_5min_date)
            .then((success: boolean) => {
                if (!success) {
                    return Promise.resolve(false);
                }

                return this.update_wallbox_daily_cache_all(this.state.current_daily_date);
            })
            .then((success: boolean) => {
                if (!success) {
                    window.setTimeout(() => {
                        this.reload_wallbox_cache();
                    }, UPDATE_RETRY_DELAY);

                    return;
                }

                if (this.state.data_type == '5min') {
                    this.update_5min_uplot();
                }
                else {
                    this.update_daily_uplot();
                }
            });
    }

    update_current_5min_cache() {
        let current_daily_date: Date = new Date(this.state.current_5min_date);

        current_daily_date.setDate(1);
        current_daily_date.setHours(0);
        current_daily_date.setMinutes(0);
        current_daily_date.setSeconds(0);
        current_daily_date.setMilliseconds(0);

        this.update_energy_manager_5min_cache(this.state.current_5min_date)
            .then((success: boolean) => {
                if (!success) {
                    return Promise.resolve(false);
                }

                return this.update_wallbox_5min_cache_all(this.state.current_5min_date);
            })
            .then((success: boolean) => {
                if (!success) {
                    return Promise.resolve(false);
                }

                return this.update_energy_manager_daily_cache(current_daily_date);
            })
            .then((success: boolean) => {
                if (!success) {
                    return Promise.resolve(false);
                }

                return this.update_wallbox_daily_cache_all(current_daily_date);
            })
            .then((success: boolean) => {
                if (!success) {
                    window.setTimeout(() => {
                        this.update_current_5min_cache();
                    }, UPDATE_RETRY_DELAY);

                    return;
                }

                this.uplot_5min_cache_initalized = true;

                this.update_5min_uplot();
                this.update_status_uplot();
            });
    }

    update_current_daily_cache() {
        this.update_energy_manager_daily_cache(this.state.current_daily_date)
            .then((success: boolean) => {
                if (!success) {
                    return Promise.resolve(false);
                }

                return this.update_wallbox_daily_cache_all(this.state.current_daily_date);
            })
            .then((success: boolean) => {
                if (!success) {
                    window.setTimeout(() => {
                        this.update_current_daily_cache();
                    }, UPDATE_RETRY_DELAY);

                    return;
                }

                this.uplot_daily_cache_initalized = true;

                this.update_daily_uplot();
            });
    }

    schedule_uplot_update() {
        if (this.uplot_update_timeout) {
            window.clearTimeout(this.uplot_update_timeout);
        }

        this.uplot_update_timeout = window.setTimeout(() => {
            this.uplot_update_timeout = null;

            if (this.state.data_type == '5min') {
                this.update_5min_uplot();
            }
            else {
                this.update_daily_uplot();
            }

            this.update_status_uplot();
        }, 100);
    }

    update_5min_uplot() {
        if (this.uplot_5min_cache_initalized && this.uplot_loader_5min_ref.current && this.uplot_wrapper_5min_power_ref.current && this.uplot_wrapper_5min_flags_ref.current) {
            this.update_uplot_5min_power_cache(this.state.current_5min_date);
            this.update_uplot_5min_flags_cache(this.state.current_5min_date);

            let current_daily_date: Date = new Date(this.state.current_5min_date);

            current_daily_date.setDate(1);
            current_daily_date.setHours(0);
            current_daily_date.setMinutes(0);
            current_daily_date.setSeconds(0);
            current_daily_date.setMilliseconds(0);

            this.update_uplot_daily_cache(current_daily_date);

            let key = this.date_to_5min_key(this.state.current_5min_date);
            let data_power = this.uplot_5min_power_cache[key];
            let data_flags = this.uplot_5min_flags_cache[key];

            let has_flags = data_flags && data_flags.keys.length > 1;
            let has_power = data_power && data_power.keys.length > 1;
            let visible = has_flags || has_power;

            if (has_flags && !has_power) {
                data_power = {
                    update_timestamp: null,
                    use_timestamp: null,
                    keys: [null],
                    names: [null],
                    values: [data_flags.values[0]],
                };
            }

            if (!has_flags && has_power) {
                data_flags = {
                    update_timestamp: null,
                    use_timestamp: null,
                    keys: [null],
                    names: [null],
                    values: [data_power.values[0]],
                };
            }

            this.uplot_loader_5min_ref.current.set_data(visible);
            this.uplot_wrapper_5min_power_ref.current.set_data(data_power, visible);
            this.uplot_wrapper_5min_flags_ref.current.set_data(data_flags, visible);
        }
    }

    update_daily_uplot() {
        if (this.uplot_daily_cache_initalized && this.uplot_loader_daily_ref.current && this.uplot_wrapper_daily_ref.current) {
            this.update_uplot_daily_cache(this.state.current_daily_date);

            let key = this.date_to_daily_key(this.state.current_daily_date);
            let data = this.uplot_daily_cache[key];

            this.uplot_loader_daily_ref.current.set_data(data && data.keys.length > 1);
            this.uplot_wrapper_daily_ref.current.set_data(data);
        }
    }

    update_status_uplot() {
        if (this.uplot_5min_cache_initalized && this.props.status_ref.current && this.props.status_ref.current.uplot_loader_ref.current && this.props.status_ref.current.uplot_wrapper_ref.current) {
            let status_date: Date = new Date();

            status_date.setHours(0);
            status_date.setMinutes(0);
            status_date.setSeconds(0);
            status_date.setMilliseconds(0);

            this.update_uplot_5min_status_cache(status_date);

            let key = this.date_to_5min_key(status_date);
            let data = this.uplot_5min_status_cache[key];

            this.props.status_ref.current.uplot_loader_ref.current.set_data(data && data.keys.length > 1);
            this.props.status_ref.current.uplot_wrapper_ref.current.set_data(data);
        }
    }

    render(props: {}, state: Readonly<EMEnergyAnalysisState>) {
        if (!util.render_allowed())
            return <SubPage name="em_energy_analysis" />;

        let total_5min = () => {
            let key = this.date_to_daily_key(state.current_5min_date);
            let energy_total = state.energy_manager_5min_cache_energy_total[key];
            let timestamp_slot = state.current_5min_date.getDate() - 1;
            let rows = [];

            if (util.hasValue(energy_total)) {
                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                    let has_import = util.hasValue(energy_total.import) && util.hasValue(energy_total.import[meter_slot]) && util.hasValue(energy_total.import[meter_slot][timestamp_slot]);
                    let has_export = util.hasValue(energy_total.export) && util.hasValue(energy_total.export[meter_slot]) && util.hasValue(energy_total.export[meter_slot][timestamp_slot]);

                    if (has_import || has_export) {
                        rows.push(
                            <FormRow label={`${get_meter_name(this.state.meter_configs, meter_slot)} (${__("em_energy_analysis.content.import")} / ${__("em_energy_analysis.content.export")})`}>
                                <div class="row">
                                    <div class="col-md-6">
                                        {has_import ?
                                            <OutputFloat value={energy_total.import[meter_slot][timestamp_slot]} digits={2} scale={0} unit="kWh"/>
                                            : undefined
                                        }
                                    </div>
                                    <div class="col-md-6">
                                        {has_export ?
                                            <OutputFloat value={energy_total.export[meter_slot][timestamp_slot]} digits={2} scale={0} unit="kWh"/>
                                            : undefined
                                        }
                                    </div>
                                </div>
                            </FormRow>
                        );
                    }
                }
            }

            for (let charger of this.chargers) {
                let energy_total = ((state.wallbox_5min_cache_energy_total[charger.uid] || {})[key] || [])[state.current_5min_date.getDate() - 1];

                if (util.hasValue(energy_total)) {
                    rows.push(
                        <FormRow label={charger.name}>
                            <div class="row">
                                <div class="col-md-6">
                                    <OutputFloat value={energy_total} digits={2} scale={0} unit="kWh"/>
                                </div>
                                <div class="col-md-6">
                                </div>
                            </div>
                        </FormRow>
                    );
                }
            }

            return rows;
        };

        let total_daily = () => {
            let key = this.date_to_daily_key(state.current_daily_date);
            let energy_total = state.energy_manager_daily_cache_energy_total[key];
            let rows = [];

            if (util.hasValue(energy_total)) {
                for (let meter_slot = 0; meter_slot < METERS_SLOTS; ++meter_slot) {
                    let has_import = util.hasValue(energy_total.import) && util.hasValue(energy_total.import[meter_slot]);
                    let has_export = util.hasValue(energy_total.export) && util.hasValue(energy_total.export[meter_slot]);

                    if (has_import || has_export) {
                        rows.push(
                            <FormRow label={`${get_meter_name(this.state.meter_configs, meter_slot)} (${__("em_energy_analysis.content.import")} / ${__("em_energy_analysis.content.export")})`}>
                                <div class="row">
                                    <div class="col-md-6">
                                        {has_import ?
                                            <OutputFloat value={energy_total.import[meter_slot]} digits={2} scale={0} unit="kWh"/>
                                            : undefined
                                        }
                                    </div>
                                    <div class="col-md-6">
                                        {has_export ?
                                            <OutputFloat value={energy_total.export[meter_slot]} digits={2} scale={0} unit="kWh"/>
                                            : undefined
                                        }
                                    </div>
                                </div>
                            </FormRow>
                        );
                    }
                }
            }

            for (let charger of this.chargers) {
                let energy_total = (state.wallbox_daily_cache_energy_total[charger.uid] || {})[key];

                if (util.hasValue(energy_total)) {
                    rows.push(
                        <FormRow label={charger.name}>
                            <div class="row">
                                <div class="col-md-6">
                                    <OutputFloat value={energy_total} digits={2} scale={0} unit="kWh"/>
                                </div>
                                <div class="col-md-6">
                                </div>
                            </div>
                        </FormRow>
                    );
                }
            }

            return rows;
        };

        let data_select =
            <InputSelect value={state.data_type} style="width: 6rem" onValue={(v) => {
                    let data_type: '5min'|'daily' = v as any;

                    this.setState({data_type: data_type}, () => {
                        if (data_type == '5min') {
                            this.uplot_loader_5min_ref.current.set_show(true);
                            this.uplot_wrapper_5min_flags_ref.current.set_show(true);
                            this.uplot_wrapper_5min_power_ref.current.set_show(true);
                            this.uplot_loader_daily_ref.current.set_show(false);
                            this.uplot_wrapper_daily_ref.current.set_show(false);

                            this.update_5min_uplot();
                        }
                        else {
                            this.uplot_loader_daily_ref.current.set_show(true);
                            this.uplot_wrapper_daily_ref.current.set_show(true);
                            this.uplot_loader_5min_ref.current.set_show(false);
                            this.uplot_wrapper_5min_flags_ref.current.set_show(false);
                            this.uplot_wrapper_5min_power_ref.current.set_show(false);

                            this.update_daily_uplot();
                        }
                    });
                }}
                items={[
                    ["5min", __("em_energy_analysis.content.data_type_5min")],
                    ["daily", __("em_energy_analysis.content.data_type_daily")],
                ]}/>;

        return (
            <SubPage name="em_energy_analysis" colClasses="col-xl-10">
                <PageHeader title={__("em_energy_analysis.content.em_energy_analysis")} titleClass="col-12 col-sm text-center text-sm-left" childrenClass="col-12 col-sm-auto mb-2">
                    {this.state.data_type == '5min'
                    ? <InputDate date={this.state.current_5min_date} onDate={(date) => this.set_current_5min_date(date)} buttons="day" style="width: 11rem">{data_select}</InputDate>
                    : <InputMonth date={this.state.current_daily_date} onDate={(date) => this.set_current_daily_date(date)} buttons="month" style="width: 11rem">{data_select}</InputMonth>}
                </PageHeader>

                <div style="position: relative;"> {/* this plain div is neccessary to make the size calculation stable in safari. without this div the height continues to grow */}
                    <UplotLoader ref={this.uplot_loader_5min_ref}
                                    show={true}
                                    marker_class="h3"
                                    no_data={__("em_energy_analysis.content.no_data")}
                                    loading={__("em_energy_analysis.content.loading")} >
                        <UplotFlagsWrapper ref={this.uplot_wrapper_5min_flags_ref}
                                            class="em-energy-analysis-flags-chart"
                                            sub_page="em_energy_analysis"
                                            show={true}
                                            sync={this.uplot_sync}
                                            legend_time_label={__("em_energy_analysis.script.time_5min")}
                                            legend_time_with_minutes={true}
                                            legend_div_ref={this.uplot_legend_div_5min_flags_ref}
                                            x_padding_factor={0}
                                            y_sync_ref={this.uplot_wrapper_5min_power_ref}
                                            y2_enable={true} />
                        <UplotWrapper ref={this.uplot_wrapper_5min_power_ref}
                                        class="em-energy-analysis-chart"
                                        sub_page="em_energy_analysis"
                                        color_cache_group="em_energy_analysis.analysis_5min"
                                        show={true}
                                        on_mount={() => this.update_5min_uplot()}
                                        sync={this.uplot_sync}
                                        legend_time_label={__("em_energy_analysis.script.time_5min")}
                                        legend_time_with_minutes={true}
                                        legend_div_ref={this.uplot_legend_div_5min_power_ref}
                                        aspect_ratio={3}
                                        x_height={35}
                                        x_format={{hour: '2-digit', minute: '2-digit'}}
                                        x_padding_factor={0}
                                        x_include_date={false}
                                        y_min={0}
                                        y_max={100}
                                        y_unit={"W"}
                                        y_label={__("em_energy_analysis.script.power") + " [W]"}
                                        y_digits={0}
                                        y_skip_upper={true}
                                        y_sync_ref={this.uplot_wrapper_5min_flags_ref}
                                        y2_enable={true}
                                        y2_min={-2}
                                        y2_max={8}
                                        y2_unit={"ct/kWh"}
                                        y2_label={__("em_energy_analysis.script.price") + " [ct/kWh]"}
                                        y2_digits={3}
                                        y2_skip_upper={true}
                                        padding={[0, 5, null, null]}/>
                        <div class="pb-4">
                            <div class={"uplot u-hz" + (util.is_native_median_app() ? "" : " u-time-in-legend-alone") + " u-hide-marker-in-legend u-hide-empty-legend"} ref={this.uplot_legend_div_5min_flags_ref} style="width: 100%; visibility: hidden;" />
                            <div class={"uplot u-hz" + (util.is_native_median_app() ? "" : " u-hide-first-series-in-legend")} ref={this.uplot_legend_div_5min_power_ref} style="width: 100%; visibility: hidden;" />
                        </div>
                    </UplotLoader>
                    <UplotLoader ref={this.uplot_loader_daily_ref}
                                    show={false}
                                    marker_class="h3"
                                    no_data={__("em_energy_analysis.content.no_data")}
                                    loading={__("em_energy_analysis.content.loading")} >
                        <UplotWrapper ref={this.uplot_wrapper_daily_ref}
                                        class="em-energy-analysis-chart pb-4"
                                        sub_page="em_energy_analysis"
                                        color_cache_group="em_energy_analysis.analysis_daily"
                                        show={false}
                                        on_mount={() => this.update_daily_uplot()}
                                        legend_time_label={__("em_energy_analysis.script.time_daily")}
                                        legend_time_with_minutes={false}
                                        aspect_ratio={3}
                                        x_height={35}
                                        x_format={{month: '2-digit', day: '2-digit'}}
                                        x_padding_factor={0.015}
                                        x_include_date={false}
                                        y_min={0}
                                        y_max={10}
                                        y_unit={"kWh"}
                                        y_label={__("em_energy_analysis.script.energy") + " [kWh]"}
                                        y_digits={2}
                                        y2_enable={true}
                                        y2_min={-1}
                                        y2_max={9}
                                        y2_unit={"ct/kWh"}
                                        y2_label={__("em_energy_analysis.script.price") + " [ct/kWh]"}
                                        y2_digits={0}
                                        padding={[null, 5, null, null]} />
                    </UplotLoader>
                </div>

                <FormSeparator heading={__("em_energy_analysis.script.total_energy")}/>

                {state.data_type == '5min' ?
                    <>
                        {
                            total_5min()
                        }
                    </> :
                    <>
                        {
                            total_daily()
                        }
                    </>
                }
            </SubPage>
        )
    }
}

export function init() {
}

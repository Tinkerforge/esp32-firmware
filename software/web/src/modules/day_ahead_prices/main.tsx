/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { Component, createRef, h, RefObject } from "preact";
import { __ } from "../../ts/translation";
import { Switch } from "../../ts/components/switch";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { FormSeparator } from "../../ts/components/form_separator";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Collapse } from "react-bootstrap";
import { InputSelect } from "../../ts/components/input_select";
import { InputFloat } from "../../ts/components/input_float";
import { UplotLoader } from "../../ts/components/uplot_loader";
import { UplotData, UplotWrapper, UplotPath } from "../../ts/components/uplot_wrapper_2nd";
import { InputText } from "../../ts/components/input_text";
import { StatusSection } from "../../ts/components/status_section";

function get_timestamp_today_00_00_in_seconds() {
    return Math.floor(new Date().setHours(0, 0, 0, 0) / 1000);
}

function day_ahead_price_time_between(index: number, start: number, end: number) {
    const dap_prices = API.get("day_ahead_prices/prices");
    const resolution_multiplier = dap_prices.resolution == 0 ? 15 : 60;
    const index_timestamp_seconds = (dap_prices.first_date + index*resolution_multiplier)*60;
    return (index_timestamp_seconds >= start) && (index_timestamp_seconds <= end);
}

export function is_day_ahead_prices_enabled() {
    return API.get("day_ahead_prices/config").enable;
}

export function get_price_from_index(index: number, incl_vat: boolean = true) {
    const dap_prices = API.get("day_ahead_prices/prices");
    const dap_config = API.get("day_ahead_prices/config");

    if (incl_vat && (dap_config.vat != 0)) {
        return Math.round(dap_prices.prices[index] * (1 + dap_config.vat / 10000.0));
    }

    return dap_prices.prices[index];
}

export function get_current_price(incl_vat: boolean = true, incl_tax_and_markup: boolean = true) {
    const dap_state  = API.get("day_ahead_prices/state");
    const dap_config = API.get("day_ahead_prices/config");

    if (dap_state.current_price == 0x7fffffff) { // INT32_MAX = current price not available yet
        return NaN
    }

    let price = dap_state.current_price;

    if (incl_vat) {
        price = Math.round(dap_state.current_price * (1 + dap_config.vat / 10000.0));
    }

    if (incl_tax_and_markup) {
        price += dap_config.grid_costs_and_taxes + dap_config.supplier_markup;
    }

    return price;
}

export function get_average_price_today(add_markup: boolean = true) {
    const dap_prices = API.get("day_ahead_prices/prices");
    const start      = get_timestamp_today_00_00_in_seconds();
    const end        = start + 60*60*24 - 1;
    let price_sum    = 0.0;
    let price_count  = 0;
    for (let index = 0; index < dap_prices.prices.length; index++) {
        if (day_ahead_price_time_between(index, start, end)) {
            price_sum += get_price_from_index(index);
            price_count++;
        }
    }

    if (price_count == 0) {
        return NaN;
    }

    const dap_config = API.get("day_ahead_prices/config");
    const price_avg = price_sum / price_count;
    const grid_costs_and_taxes_and_supplier_markup = dap_config.grid_costs_and_taxes + dap_config.supplier_markup;
    return price_avg + (add_markup ? grid_costs_and_taxes_and_supplier_markup : 0);
}

export function get_average_price_tomorrow(add_markup: boolean = true) {
    const dap_prices = API.get("day_ahead_prices/prices");
    const start      = get_timestamp_today_00_00_in_seconds() + 60*60*24;
    const end        = start + 60*60*24 - 1;
    let price_sum    = 0.0;
    let price_count  = 0;
    for (let index = 0; index < dap_prices.prices.length; index++) {
        if (day_ahead_price_time_between(index, start, end)) {
            price_sum += get_price_from_index(index);
            price_count++;
        }
    }

    // If we only have a few hours of data for the next day we don't count this as valid data
    if (price_count < 8) {
        return NaN;
    }

    const dap_config = API.get("day_ahead_prices/config");
    const price_avg = price_sum / price_count;
    const grid_costs_and_taxes_and_supplier_markup = dap_config.grid_costs_and_taxes + dap_config.supplier_markup;
    return price_avg + (add_markup ? grid_costs_and_taxes_and_supplier_markup : 0);
}

function get_current_price_string() {
    const dap_config = API.get("day_ahead_prices/config");

    let str = util.get_value_with_unit(get_current_price(false, false), "ct/kWh", 2, 1000);
    if ((dap_config.vat != 0) || (dap_config.grid_costs_and_taxes) != 0 || (dap_config.supplier_markup != 0)) {
        str += " (";
        str += util.get_value_with_unit(get_current_price(true, true), "ct/kWh", 2, 1000);
        str += " ";
        str += __("day_ahead_prices.content.incl_all_costs");
        str += ")";
    }

    return str;
}


function get_price_timeframe() {
    const dap_config = API.get("day_ahead_prices/config");

    let time = new Date(util.get_date_now_1m_update_rate());
    let s = ""
    if(dap_config.resolution == 0) {
        time.setMilliseconds(Math.floor(time.getMilliseconds() / 1000) * 1000);
        time.setSeconds(Math.floor(time.getSeconds() / 60) * 60);
        time.setMinutes(Math.floor(time.getMinutes() / 15) * 15);
        s += time.toLocaleTimeString() + '-';
        time.setMinutes(time.getMinutes() + 15);
        s += time.toLocaleTimeString()
    } else {
        time.setMilliseconds(Math.floor(time.getMilliseconds() / 1000) * 1000);
        time.setSeconds(Math.floor(time.getSeconds() / 60) * 60);
        time.setMinutes(Math.floor(time.getMinutes() / 60) * 60);
        s += time.toLocaleTimeString() + '-';
        time.setMinutes(time.getMinutes() + 60);
        s += time.toLocaleTimeString()
    }

    return s
}

export function DayAheadPricesNavbar() {
    return (
        <NavbarItem
            name="day_ahead_prices"
            title={__("day_ahead_prices.navbar.day_ahead_prices")}
            hidden={false}
            symbol={
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" fill="currentColor" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round">
                    <path d="M11.438 21.363V20.04H12.608v1.458l.17-.015c1.279-.117 2.4-.43 3.486-.974a9.544 9.544 0 0 0 5.002-6.273c.114-.467.247-1.322.248-1.596v-.053H20.063v-1.172h1.458l-.015-.17A9.601 9.601 0 0 0 19.396 6a12.023 12.023 0 0 0-1.415-1.411c-1.52-1.212-3.196-1.877-5.213-2.07l-.159-.015v1.48H11.438v-1.48l-.17.015c-1.702.155-3.17.672-4.569 1.608-2.12 1.42-3.626 3.752-4.05 6.276-.044.258-.117.862-.117.97 0 .038.05.041.726.041h.726v1.172H1.336v-.393c.001-1.778.344-3.33 1.064-4.822.247-.512.423-.819.753-1.312a10.67 10.67 0 0 1 10.21-4.651c2.365.3 4.523 1.357 6.209 3.043a10.664 10.664 0 0 1 3.09 8.498 10.688 10.688 0 0 1-5.876 8.612c-1.539.767-3.126 1.126-4.98 1.126h-.368Zm-5.72.8v-.5l-.175-.016a2.101 2.101 0 0 1-1.699-1.15 1.782 1.782 0 0 1-.205-.71l-.013-.17h1.172l.014.115c.026.219.184.456.404.606.039.027.138.071.22.099.142.048.183.05.925.043.74-.008.782-.01.904-.06.195-.08.373-.214.484-.363.14-.19.185-.336.184-.592 0-.188-.008-.228-.075-.37a1.124 1.124 0 0 0-.674-.58c-.064-.02-.381-.035-.973-.046-.863-.016-.883-.017-1.085-.079a2.195 2.195 0 0 1-.963-.552 2.117 2.117 0 0 1-.433-2.498c.332-.668 1.05-1.151 1.784-1.2l.205-.014v-.978H6.89v.985h.127c.23 0 .574.092.834.222.428.215.811.632.993 1.08.07.172.132.46.132.62v.118l-.58-.007-.58-.006-.018-.124c-.045-.312-.286-.582-.627-.699-.116-.04-.204-.044-.879-.044-.725 0-.755.002-.902.055-.395.14-.644.425-.71.808-.071.419.159.83.584 1.04l.182.09.88.014c.843.013.89.017 1.105.074.822.217 1.43.824 1.631 1.626.06.241.054.757-.013 1.004a2.243 2.243 0 0 1-1.974 1.643l-.185.016v1.001H5.719Zm8.163-7.474-2.444-2.443V6.773h1.172v4.98l2.28 2.28 2.279 2.28-.41.41c-.225.225-.415.41-.422.41-.006 0-1.111-1.1-2.455-2.444z"/>
                </svg>
            }
        />
    );
}

type DayAheadPricesConfig = API.getType["day_ahead_prices/config"];

interface DayAheadPricesState {
    dap_state:  API.getType["day_ahead_prices/state"];
    dap_prices: API.getType["day_ahead_prices/prices"];
}

export class DayAheadPrices extends ConfigComponent<"day_ahead_prices/config", {status_ref?: RefObject<DayAheadPricesStatus>}, DayAheadPricesState> {
    uplot_loader_ref        = createRef();
    uplot_wrapper_ref       = createRef();

    constructor() {
        super('day_ahead_prices/config',
              () => __("day_ahead_prices.script.save_failed"));

        util.addApiEventListener("day_ahead_prices/state", () => {
            this.setState({dap_state: API.get("day_ahead_prices/state")});
        });
        util.addApiEventListener("day_ahead_prices/prices", () => {
            this.setState({dap_prices: API.get("day_ahead_prices/prices")});
            // Update chart every time new price data comes in
            this.update_uplot();
        });
    }

    update_uplot() {
        if (this.uplot_wrapper_ref.current == null) {
            return;
        }

        let data: UplotData;

        // If we have not got any prices yet, use empty data
        if (this.state.dap_prices.prices.length == 0) {
            data = {
                keys: [null],
                names: [null],
                values: [null],
                stacked: [null],
                paths: [null],
            }
        // Else fill with time and the three different prices we want to show
        } else {
            data = {
                keys: [null, 'price', 'price2', 'price3'],
                names: [null, __("day_ahead_prices.content.electricity_price"), __("day_ahead_prices.content.grid_fees_plus_taxes"), __("day_ahead_prices.content.surcharge")],
                values: [[], [], [], []],
                stacked: [null, true, true, true],
                paths: [null, UplotPath.Step, UplotPath.Step, UplotPath.Step],
                // Only enable the electricity price by default.
                // The chart with only electricity price is the most useful in most cases.
                default_visibilty: [null, true, false, false],
                lines_vertical: []
            }
            let resolution_multiplier = this.state.dap_prices.resolution == 0 ? 15 : 60
            for (let i = 0; i < this.state.dap_prices.prices.length; i++) {
                data.values[0].push(this.state.dap_prices.first_date * 60 + i * 60 * resolution_multiplier);
                data.values[1].push(get_price_from_index(i) / 1000.0);
                data.values[2].push(this.state.grid_costs_and_taxes / 1000.0);
                data.values[3].push(this.state.supplier_markup / 1000.0);
            }

            data.values[0].push(this.state.dap_prices.first_date * 60 + this.state.dap_prices.prices.length * 60 * resolution_multiplier - 1);
            data.values[1].push(get_price_from_index(this.state.dap_prices.prices.length - 1) / 1000.0);
            data.values[2].push(this.state.grid_costs_and_taxes / 1000.0);
            data.values[3].push(this.state.supplier_markup / 1000.0);

            // Add vertical line at current time
            const resolution_divisor = this.state.resolution == 0 ? 15 : 60;
            const diff = Math.floor(util.get_date_now_1m_update_rate() / 60000) - this.state.dap_prices.first_date;
            const index = Math.floor(diff / resolution_divisor);
            data.lines_vertical.push({'index': index, 'text': __("day_ahead_prices.content.now"), 'color': [64, 64, 64, 0.2]});
        }

        // Show loader or data depending on the availability of data
        this.uplot_loader_ref.current.set_data(data && data.keys.length > 1);
        this.uplot_wrapper_ref.current.set_data(data);
    }

    render(props: {}, dap: DayAheadPricesState & DayAheadPricesConfig) {
        if (!util.render_allowed()) {
            return <SubPage name="day_ahead_prices" />;
        }

        return (
            <SubPage name="day_ahead_prices">
                <ConfigForm id="day_ahead_prices_config_form"
                            title={__("day_ahead_prices.content.day_ahead_prices")}
                            isModified={false}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("day_ahead_prices.content.enable_day_ahead_prices")} label_muted={__("day_ahead_prices.content.day_ahead_prices_muted")}>
                        <Switch desc={__("day_ahead_prices.content.day_ahead_prices_desc")}
                                checked={dap.enable}
                                onClick={this.toggle('enable')}
                        />
                    </FormRow>
                    <FormRow label={__("day_ahead_prices.content.region")}>
                        <InputSelect
                            items={[
                                ["0", __("day_ahead_prices.content.germany")],
                                ["1", __("day_ahead_prices.content.austria")],
                                ["2", __("day_ahead_prices.content.luxembourg")],
                            ]}
                            value={dap.region}
                            onValue={(v) => this.setState({region: parseInt(v)})}
                        />
                    </FormRow>
                    <FormRow label={__("day_ahead_prices.content.resolution")} label_muted={__("day_ahead_prices.content.resolution_muted")}>
                        <InputSelect
                            items={[
                                ["0", __("day_ahead_prices.content.minutes15")],
                                ["1", __("day_ahead_prices.content.minutes60")],
                            ]}
                            value={dap.resolution}
                            onValue={(v) => this.setState({resolution: parseInt(v)})}
                        />
                    </FormRow>
                    <FormSeparator heading={__("day_ahead_prices.content.extra_costs")} help={__("day_ahead_prices.content.extra_costs_help")}/>
                    <FormRow label={__("day_ahead_prices.content.vat")} label_muted={__("day_ahead_prices.content.optional_muted")}>
                        <InputFloat value={dap.vat} onValue={(v) => {this.setState({vat: v}, function() {this.update_uplot()})}} digits={2} unit={'%'} max={10000} min={0}/>
                        <div class="invalid-feedback">{__("day_ahead_prices.content.price_invalid")}</div>
                    </FormRow>
                    <FormRow label={__("day_ahead_prices.content.grid_fees_and_taxes")} label_muted={__("day_ahead_prices.content.optional_muted")}>
                        <InputFloat value={dap.grid_costs_and_taxes} onValue={(v) => {this.setState({grid_costs_and_taxes: v}, function() {this.update_uplot()})}} digits={3} unit={'ct/kWh'} max={99000} min={0}/>
                        <div class="invalid-feedback">{__("day_ahead_prices.content.price_invalid")}</div>
                    </FormRow>
                    <FormRow label={__("day_ahead_prices.content.electricity_provider_surcharge")} label_muted={__("day_ahead_prices.content.optional_muted")}>
                        <InputFloat value={dap.supplier_markup} onValue={(v) => {this.setState({supplier_markup: v}, function() {this.update_uplot()})}} digits={3} unit={'ct/kWh'} max={99000} min={0}/>
                        <div class="invalid-feedback">{__("day_ahead_prices.content.price_invalid")}</div>
                    </FormRow>
                    <FormRow label={__("day_ahead_prices.content.electricity_provider_base_fee")} label_muted={__("day_ahead_prices.content.optional_muted")}>
                        <InputFloat value={dap.supplier_base_fee} onValue={this.set('supplier_base_fee')} digits={2} unit={__("day_ahead_prices.content.euro_divided_by_month")} max={99000} min={0}/>
                        <div class="invalid-feedback">{__("day_ahead_prices.content.price_invalid")}</div>
                    </FormRow>
                </ConfigForm>
                <FormSeparator heading={__("day_ahead_prices.content.day_ahead_market_prices_heading")}/>
                <FormRow label={__("day_ahead_prices.content.current_price")} label_muted={get_price_timeframe()}>
                    <InputText value={get_current_price_string()}/>
                </FormRow>
                <FormRow label={__("day_ahead_prices.content.average_price")} label_muted={((dap.vat != 0) || (dap.grid_costs_and_taxes) != 0 || (dap.supplier_markup != 0)) ? __("day_ahead_prices.content.incl_all_costs") : ""}>
                    <div class="row mx-n1">
                        <div class="col-md-6 px-1">
                            <div class="input-group">
                                <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("today")}</span></div>
                                <InputText
                                    value={util.get_value_with_unit(get_average_price_today(), "ct/kWh", 2, 1000)}
                                />
                            </div>
                        </div>
                        <div class="col-md-6 px-1">
                            <div class="input-group">
                                <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("tomorrow")}</span></div>
                                <InputText
                                    value={util.get_value_with_unit(get_average_price_tomorrow(), "ct/kWh", 2, 1000)}
                                />
                            </div>
                        </div>
                    </div>
                </FormRow>
                <div>
                    <div style="position: relative;"> {/* this plain div is neccessary to make the size calculation stable in safari. without this div the height continues to grow */}
                        <UplotLoader
                            ref={this.uplot_loader_ref}
                            show={true}
                            marker_class={'h4'}
                            no_data={__("day_ahead_prices.content.no_data")}
                            loading={__("day_ahead_prices.content.loading")}>
                            <UplotWrapper
                                ref={this.uplot_wrapper_ref}
                                class="day-ahead-prices-chart pb-3 pt-2"
                                sub_page="day_ahead_prices"
                                color_cache_group="day_ahead_prices.default"
                                show={true}
                                on_mount={() => this.update_uplot()}
                                legend_time_label={__("day_ahead_prices.content.time")}
                                legend_time_with_minutes={true}
                                aspect_ratio={3}
                                x_height={50}
                                x_format={{hour: '2-digit', minute: '2-digit'}}
                                x_padding_factor={0}
                                x_include_date={true}
                                y_min={0}
                                y_unit={"ct/kWh"}
                                y_label={__("day_ahead_prices.content.price_ct_per_kwh")}
                                y_digits={3}
                                only_show_visible={true}
                                padding={[20, 20, null, null]}
                            />
                        </UplotLoader>
                    </div>
                </div>
            </SubPage>
        );
    }
}

export class DayAheadPricesStatus extends Component
{
    render() {
        const config = API.get('day_ahead_prices/config')
        if (!util.render_allowed() || !config.enable)
            return <StatusSection name="day_ahead_prices" />

        return <StatusSection name="day_ahead_prices">
            <FormRow label={__("day_ahead_prices.content.current_price")} label_muted={get_price_timeframe()}>
                <InputText value={get_current_price_string()}/>
            </FormRow>
            <FormRow label={__("day_ahead_prices.content.average_price")} label_muted={((config.vat != 0) || (config.grid_costs_and_taxes) != 0 || (config.supplier_markup != 0)) ? __("day_ahead_prices.content.incl_all_costs") : ""}>
                <div class="row mx-n1">
                    <div class="col-md-6 px-1">
                        <div class="input-group">
                            <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("today")}</span></div>
                            <InputText
                                value={util.get_value_with_unit(get_average_price_today(), "ct/kWh", 2, 1000)}
                            />
                        </div>
                    </div>
                    <div class="col-md-6 px-1">
                        <div class="input-group">
                            <div class="input-group-prepend"><span class="heating-fixed-size input-group-text">{__("tomorrow")}</span></div>
                            <InputText
                                value={util.get_value_with_unit(get_average_price_tomorrow(), "ct/kWh", 2, 1000)}
                            />
                        </div>
                    </div>
                </div>
            </FormRow>
        </StatusSection>;
    }
}

export function init() {
}
